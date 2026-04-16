/**
 * @file bmp390_service.c
 * @brief BMP390 pressure sensor driver + SPP service implementation.
 *
 * Combines the low-level SPI driver with an acquisition task that reads
 * altitude, pressure and temperature at 5 Hz and logs packets to SD card.
 */

#include "spp/services/bmp390/bmp390.h"
#include "spp/services/datalogger/datalogger.h"

#include "spp/hal/spi.h"
#include "spp/osal/task.h"
#include "spp/services/log/log.h"
#include "spp/services/databank/databank.h"
#include "spp/services/db_flow/db_flow.h"
#include "spp/core/packet.h"

#include <string.h>
#include <math.h>

/* ----------------------------------------------------------------
 * Private constants
 * ---------------------------------------------------------------- */

/** @brief SPI read flag — set MSB of register address to signal a read. */
#define K_BMP390_SPI_READ  0x80U

/** @brief SPI write marker (zero — write when MSB clear). */
#define K_BMP390_SPI_WRITE 0x00U

#define K_BMP_SERVICE_APID_DBG        (0x0101U)
#define K_BMP_SERVICE_TASK_PRIO       (5U)
#define K_BMP_SERVICE_TASK_DELAY_MS   (200U)
#define K_BMP_SERVICE_TASK_STACK_SIZE (4096U)
#define K_BMP_SERVICE_PAYLOAD_LEN     (12U)

static const char *const k_tag = "BMP390";
static const char *const k_svcTag = "BMP_SVC";

/* ----------------------------------------------------------------
 * Private globals (used by pressure compensation — mirrored from
 * original bmp390.c to avoid stack bloat inside the task)
 * ---------------------------------------------------------------- */

static float s_pd1, s_pd2, s_pd3, s_pd4;
static float s_po1, s_po2;
static float s_compPress;

/* ----------------------------------------------------------------
 * Driver — initialisation
 * ---------------------------------------------------------------- */

void BMP390_init(void *p_data)
{
    BMP390_Data_t *p_bmp = (BMP390_Data_t *)p_data;

    p_bmp->p_event_group = SPP_OSAL_eventCreate();

    p_bmp->isr_ctx.p_eventGroup = p_bmp->p_event_group;
    p_bmp->isr_ctx.bits = K_BMP390_EVT_DRDY;

    SPP_HAL_gpioConfigInterrupt(p_bmp->intPin, p_bmp->intIntrType, p_bmp->intPull);
    SPP_HAL_gpioRegisterIsr(p_bmp->intPin, (void *)&p_bmp->isr_ctx);
}

/* ----------------------------------------------------------------
 * Driver — configuration helpers
 * ---------------------------------------------------------------- */

SPP_RetVal_t BMP390_softReset(void *p_spi)
{
    spp_uint8_t buf[2] = {(spp_uint8_t)K_BMP390_SOFT_RESET_REG,
                          (spp_uint8_t)BMP390_SOFT_RESET_CMD};
    SPP_RetVal_t ret = SPP_HAL_spiTransmit(p_spi, buf, sizeof(buf));
    SPP_OSAL_taskDelayMs(100U);
    return ret;
}

SPP_RetVal_t BMP390_enableSpiMode(void *p_spi)
{
    spp_uint8_t buf[2] = {(spp_uint8_t)K_BMP390_IF_CONF_REG,
                          (spp_uint8_t)BMP390_IF_CONF_SPI};
    SPP_RetVal_t ret = SPP_HAL_spiTransmit(p_spi, buf, (spp_uint8_t)sizeof(buf));
    SPP_OSAL_taskDelayMs(100U);
    return ret;
}

SPP_RetVal_t BMP390_configCheck(void *p_spi)
{
    spp_uint8_t buf[9] = {
        (spp_uint8_t)(K_BMP390_SPI_READ | K_BMP390_IF_CONF_REG),    K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE,
        (spp_uint8_t)(K_BMP390_SPI_READ | K_BMP390_SOFT_RESET_REG), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE,
        (spp_uint8_t)(K_BMP390_SPI_READ | K_BMP390_CHIP_ID_REG),    K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE};

    SPP_RetVal_t ret = SPP_HAL_spiTransmit(p_spi, buf, (spp_uint8_t)sizeof(buf));
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    SPP_LOGI(k_tag, "ID: 0x%02X", buf[8]);

    if (buf[8] != 0x60U)
    {
        SPP_LOGE(k_tag, "BMP390 not detected! Expected 0x60, got 0x%02X", buf[8]);
        return K_SPP_ERROR;
    }

    return ret;
}

SPP_RetVal_t BMP390_auxConfig(void *p_spi)
{
    SPP_RetVal_t ret;

    ret = BMP390_softReset(p_spi);
    if (ret != K_SPP_OK) { return ret; }

    ret = BMP390_enableSpiMode(p_spi);
    if (ret != K_SPP_OK) { return ret; }

    ret = BMP390_configCheck(p_spi);
    if (ret != K_SPP_OK) { return ret; }

    return ret;
}

SPP_RetVal_t BMP390_prepareMeasure(void *p_spi)
{
    spp_uint8_t buf[8] = {
        (spp_uint8_t)K_BMP390_REG_OSR,     (spp_uint8_t)BMP390_VALUE_OSR,
        (spp_uint8_t)K_BMP390_REG_ODR,     (spp_uint8_t)BMP390_VALUE_ODR,
        (spp_uint8_t)K_BMP390_REG_IIR,     (spp_uint8_t)BMP390_VALUE_IIR,
        (spp_uint8_t)K_BMP390_REG_PWRCTRL, (spp_uint8_t)BMP390_VALUE_PWRCTRL};

    return SPP_HAL_spiTransmit(p_spi, buf, sizeof(buf));
}

SPP_RetVal_t BMP390_waitDrdy(BMP390_Data_t *p_bmp, spp_uint32_t timeout_ms)
{
    spp_uint32_t bits;

    return SPP_OSAL_eventWait(p_bmp->p_event_group, K_BMP390_EVT_DRDY,
                              true,  /* clearOnExit */
                              false, /* waitAll    */
                              timeout_ms, &bits);
}

/* ----------------------------------------------------------------
 * Driver — temperature
 * ---------------------------------------------------------------- */

SPP_RetVal_t BMP390_readRawTempCoeffs(void *p_spi, BMP390_temp_calib_t *tcalib)
{
    spp_uint8_t buf[15] = {
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_TEMP_CALIB_REG_START + 0)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE,
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_TEMP_CALIB_REG_START + 1)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE,
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_TEMP_CALIB_REG_START + 2)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE,
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_TEMP_CALIB_REG_START + 3)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE,
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_TEMP_CALIB_REG_START + 4)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE};

    SPP_RetVal_t ret = SPP_HAL_spiTransmit(p_spi, buf, sizeof(buf));
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    spp_uint8_t raw[5];
    raw[0] = buf[2];
    raw[1] = buf[5];
    raw[2] = buf[8];
    raw[3] = buf[11];
    raw[4] = buf[14];

    tcalib->par_t1 = (uint16_t)((raw[1] << 8) | raw[0]);
    tcalib->par_t2 = (int16_t)((raw[3] << 8) | raw[2]);
    tcalib->par_t3 = (int8_t)raw[4];

    return ret;
}

SPP_RetVal_t BMP390_calibrateTempParams(void *p_spi, BMP390_temp_params_t *out)
{
    BMP390_temp_calib_t raw;

    SPP_RetVal_t ret = BMP390_readRawTempCoeffs(p_spi, &raw);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    out->PAR_T1 = raw.par_t1 * 256.0f;             /* 2^8  */
    out->PAR_T2 = raw.par_t2 / 1073741824.0f;      /* 2^30 */
    out->PAR_T3 = raw.par_t3 / 281474976710656.0f; /* 2^48 */

    return ret;
}

SPP_RetVal_t BMP390_readRawTemp(void *p_spi, uint32_t *raw_temp)
{
    spp_uint8_t buf[9] = {
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_TEMP_RAW_REG + 0)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE,
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_TEMP_RAW_REG + 1)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE,
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_TEMP_RAW_REG + 2)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE};

    SPP_RetVal_t ret = SPP_HAL_spiTransmit(p_spi, buf, sizeof(buf));
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    spp_uint8_t xlsb = buf[2];
    spp_uint8_t lsb  = buf[5];
    spp_uint8_t msb  = buf[8];

    *raw_temp = ((spp_uint32_t)msb << 16) | ((spp_uint32_t)lsb << 8) | (spp_uint32_t)xlsb;

    return ret;
}

float BMP390_compensateTemperature(spp_uint32_t raw_temp, BMP390_temp_params_t *params)
{
    float partial1 = (float)raw_temp - params->PAR_T1;
    float partial2 = partial1 * params->PAR_T2;
    float t_lin    = partial2 + (partial1 * partial1) * params->PAR_T3;

    return t_lin;
}

SPP_RetVal_t BMP390_auxGetTemp(void *p_spi, const BMP390_temp_params_t *temp_params,
                             spp_uint32_t *raw_temp, float *comp_temp)
{
    SPP_RetVal_t ret = BMP390_readRawTemp(p_spi, raw_temp);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    *comp_temp = BMP390_compensateTemperature(*raw_temp, (BMP390_temp_params_t *)temp_params);

    return ret;
}

/* ----------------------------------------------------------------
 * Driver — pressure
 * ---------------------------------------------------------------- */

SPP_RetVal_t BMP390_readRawPressCoeffs(void *p_spi, BMP390_press_calib_t *pcalib)
{
    spp_uint8_t buf[48] = {
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_CALIB_REG_START +  0)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE,
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_CALIB_REG_START +  1)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE,
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_CALIB_REG_START +  2)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE,
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_CALIB_REG_START +  3)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE,
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_CALIB_REG_START +  4)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE,
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_CALIB_REG_START +  5)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE,
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_CALIB_REG_START +  6)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE,
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_CALIB_REG_START +  7)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE,
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_CALIB_REG_START +  8)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE,
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_CALIB_REG_START +  9)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE,
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_CALIB_REG_START + 10)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE,
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_CALIB_REG_START + 11)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE,
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_CALIB_REG_START + 12)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE,
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_CALIB_REG_START + 13)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE,
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_CALIB_REG_START + 14)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE,
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_CALIB_REG_START + 15)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE};

    SPP_RetVal_t ret = SPP_HAL_spiTransmit(p_spi, buf, sizeof(buf));
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    spp_uint8_t raw[16];
    for (int i = 0; i < 16; i++)
    {
        raw[i] = buf[3 * i + 2];
    }

    pcalib->par_p1  = (spp_uint16_t)((raw[1]  << 8) | raw[0]);
    pcalib->par_p2  = (spp_uint16_t)((raw[3]  << 8) | raw[2]);
    pcalib->par_p3  = (spp_int8_t)raw[4];
    pcalib->par_p4  = (spp_int8_t)raw[5];
    pcalib->par_p5  = (spp_uint16_t)((raw[7]  << 8) | raw[6]);
    pcalib->par_p6  = (spp_uint16_t)((raw[9]  << 8) | raw[8]);
    pcalib->par_p7  = (spp_int8_t)raw[10];
    pcalib->par_p8  = (spp_int8_t)raw[11];
    pcalib->par_p9  = (spp_int16_t)((raw[13] << 8) | raw[12]);
    pcalib->par_p10 = (spp_int8_t)raw[14];
    pcalib->par_p11 = (spp_int8_t)raw[15];

    return ret;
}

SPP_RetVal_t BMP390_calibratePressParams(void *p_spi, BMP390_press_params_t *out)
{
    BMP390_press_calib_t raw;

    SPP_RetVal_t ret = BMP390_readRawPressCoeffs(p_spi, &raw);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    out->PAR_P1  = (raw.par_p1  - 16384.0f) / 1048576.0f;     /* (p1 - 2^14) / 2^20 */
    out->PAR_P2  = (raw.par_p2  - 16384.0f) / 536870912.0f;   /* (p2 - 2^14) / 2^29 */
    out->PAR_P3  = raw.par_p3  / 4294967296.0f;                /* / 2^32 */
    out->PAR_P4  = raw.par_p4  / 137438953472.0f;              /* / 2^37 */
    out->PAR_P5  = raw.par_p5  * 8.0f;                        /* / 2^-3 */
    out->PAR_P6  = raw.par_p6  / 64.0f;                       /* / 2^6  */
    out->PAR_P7  = raw.par_p7  / 256.0f;                      /* / 2^8  */
    out->PAR_P8  = raw.par_p8  / 32768.0f;                    /* / 2^15 */
    out->PAR_P9  = raw.par_p9  / 281474976710656.0f;          /* / 2^48 */
    out->PAR_P10 = raw.par_p10 / 281474976710656.0f;          /* / 2^48 */
    out->PAR_P11 = raw.par_p11 / 36893488147419103232.0f;     /* / 2^65 */

    return ret;
}

SPP_RetVal_t BMP390_readRawPress(void *p_spi, spp_uint32_t *raw_press)
{
    spp_uint8_t buf[9] = {
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_RAW_REG + 0)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE,
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_RAW_REG + 1)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE,
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_RAW_REG + 2)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE};

    SPP_RetVal_t ret = SPP_HAL_spiTransmit(p_spi, buf, sizeof(buf));
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    spp_uint8_t xlsb = buf[2];
    spp_uint8_t lsb  = buf[5];
    spp_uint8_t msb  = buf[8];

    *raw_press = ((spp_uint32_t)msb << 16) | ((spp_uint32_t)lsb << 8) | (spp_uint32_t)xlsb;

    return ret;
}

float BMP390_compensatePressure(spp_uint32_t raw_press, float t_lin,
                                  BMP390_press_params_t *p)
{
    s_pd1 = p->PAR_P6 * t_lin;
    s_pd2 = p->PAR_P7 * (t_lin * t_lin);
    s_pd3 = p->PAR_P8 * (t_lin * t_lin * t_lin);
    s_po1 = p->PAR_P5 + s_pd1 + s_pd2 + s_pd3;

    s_pd1 = p->PAR_P2 * t_lin;
    s_pd2 = p->PAR_P3 * (t_lin * t_lin);
    s_pd3 = p->PAR_P4 * (t_lin * t_lin * t_lin);
    s_po2 = raw_press * (p->PAR_P1 + s_pd1 + s_pd2 + s_pd3);

    s_pd1  = raw_press * raw_press;
    s_pd2  = p->PAR_P9 + p->PAR_P10 * t_lin;
    s_pd3  = s_pd1 * s_pd2;
    s_pd4  = s_pd3 + (raw_press * raw_press * raw_press) * p->PAR_P11;

    s_compPress = s_po1 + s_po2 + s_pd4;

    return s_compPress;
}

SPP_RetVal_t BMP390_auxGetPress(void *p_spi, const BMP390_press_params_t *press_params,
                              float t_lin, spp_uint32_t *raw_press, float *comp_press)
{
    SPP_RetVal_t ret = BMP390_readRawPress(p_spi, raw_press);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    *comp_press = BMP390_compensatePressure(*raw_press, t_lin,
                                             (BMP390_press_params_t *)press_params);

    return ret;
}

/* ----------------------------------------------------------------
 * Driver — altitude
 * ---------------------------------------------------------------- */

SPP_RetVal_t BMP390_getAltitude(void *p_spi, BMP390_Data_t *p_bmp, float *altitude_m,
                            float *pressure_pa, float *temperature_c)
{
    (void)p_bmp; /* DRDY wait is handled by the caller */

    if ((p_spi == NULL) || (altitude_m == NULL) || (pressure_pa == NULL) ||
        (temperature_c == NULL))
    {
        return K_SPP_ERROR_NULL_POINTER;
    }

    static spp_bool_t          s_inited = false;
    static BMP390_temp_params_t s_tempParams;
    static BMP390_press_params_t s_pressParams;
    static spp_uint32_t         s_rawTemp;
    static spp_uint32_t         s_rawPress;

    float t_lin    = 0.0f;
    float compPress = 0.0f;

    if (s_inited == false)
    {
        SPP_RetVal_t ret = BMP390_calibrateTempParams(p_spi, &s_tempParams);
        if (ret != K_SPP_OK) { return ret; }

        ret = BMP390_calibratePressParams(p_spi, &s_pressParams);
        if (ret != K_SPP_OK) { return ret; }

        s_inited = true;
    }

    SPP_RetVal_t ret = BMP390_auxGetTemp(p_spi, &s_tempParams, &s_rawTemp, &t_lin);
    if (ret != K_SPP_OK) { return ret; }

    ret = BMP390_auxGetPress(p_spi, &s_pressParams, t_lin, &s_rawPress, &compPress);
    if (ret != K_SPP_OK) { return ret; }

    *temperature_c = t_lin;
    *pressure_pa   = compPress;
    *altitude_m    = 44330.0f * (1.0f - powf(compPress / 101325.0f, 1.0f / 5.255f));

    return K_SPP_OK;
}

/* ----------------------------------------------------------------
 * Driver — interrupt
 * ---------------------------------------------------------------- */

SPP_RetVal_t BMP390_intEnableDrdy(void *p_spi)
{
    spp_uint8_t buf[2] = {
        K_BMP390_REG_INT_CTRL,
        (spp_uint8_t)(K_BMP390_INT_CTRL_LEVEL | K_BMP390_INT_CTRL_DRDY_EN)};

    return SPP_HAL_spiTransmit(p_spi, buf, sizeof(buf));
}

/* ----------------------------------------------------------------
 * Service — acquisition task
 * ---------------------------------------------------------------- */

static void BMP390_ServiceTask(void *p_arg)
{
    BMP390_ServiceCtx_t       *ctx = (BMP390_ServiceCtx_t *)p_arg;
    const BMP390_ServiceCfg_t *cfg = ctx->p_cfg;
    Datalogger_t logger;
    SPP_RetVal_t     ret;
    spp_uint8_t  loggerActive = 0U;

    SPP_LOGI(k_svcTag, "Task start");

    ret = DATALOGGER_init(&logger, (void *)&cfg->sdCfg, cfg->p_logFilePath);
    if (ret != K_SPP_OK)
    {
        SPP_LOGE(k_svcTag, "DATALOGGER_Init failed ret=%d", (int)ret);
    }
    else
    {
        loggerActive = 1U;
        SPP_LOGI(k_svcTag, "Datalogger ready");
    }

    for (;;)
    {
        SPP_Packet_t *p_packet   = NULL;
        SPP_Packet_t *p_packetRx = NULL;
        float         altitude    = 0.0f;
        float         pressure    = 0.0f;
        float         temperature = 0.0f;

        ret = BMP390_waitDrdy(&ctx->bmpData, 5000U);
        if (ret != K_SPP_OK)
        {
            SPP_LOGE(k_svcTag, "DRDY wait failed ret=%d", (int)ret);
            continue;
        }

        p_packet = SPP_Databank_getPacket();
        if (p_packet == NULL)
        {
            SPP_LOGI(k_svcTag, "No free packet");
            continue;
        }

        ret = BMP390_getAltitude(ctx->p_spi, &ctx->bmpData,
                                 &altitude, &pressure, &temperature);
        if (ret != K_SPP_OK)
        {
            SPP_LOGE(k_svcTag, "BMP390_getAltitude failed ret=%d -> return packet",
                     (int)ret);
            (void)SPP_Databank_returnPacket(p_packet);
            continue;
        }

        p_packet->primaryHeader.version    = K_SPP_PKT_VERSION;
        p_packet->primaryHeader.apid       = K_BMP_SERVICE_APID_DBG;
        p_packet->primaryHeader.seq        = ctx->seq++;
        p_packet->primaryHeader.payloadLen = K_BMP_SERVICE_PAYLOAD_LEN;

        p_packet->secondaryHeader.timestampMs = 0U;
        p_packet->secondaryHeader.dropCounter = 0U;
        p_packet->crc = 0U;

        memset(p_packet->payload, 0, K_SPP_PKT_PAYLOAD_MAX);
        memcpy(&p_packet->payload[0], &altitude,    sizeof(float));
        memcpy(&p_packet->payload[4], &pressure,    sizeof(float));
        memcpy(&p_packet->payload[8], &temperature, sizeof(float));

        ret = SPP_DbFlow_pushReady(p_packet);
        if (ret != K_SPP_OK)
        {
            SPP_LOGE(k_svcTag, "SPP_DbFlow_pushReady failed ret=%d -> return packet",
                     (int)ret);
            (void)SPP_Databank_returnPacket(p_packet);
            continue;
        }

        ret = SPP_DbFlow_popReady(&p_packetRx);
        if ((ret != K_SPP_OK) || (p_packetRx == NULL))
        {
            SPP_LOGE(k_svcTag, "SPP_DbFlow_popReady failed ret=%d pkt=%p",
                     (int)ret, (void *)p_packetRx);
            continue;
        }

        if (loggerActive == 1U)
        {
            ret = DATALOGGER_logPacket(&logger, p_packetRx);
            if (ret != K_SPP_OK)
            {
                SPP_LOGE(k_svcTag, "DATALOGGER_LogPacket failed ret=%d", (int)ret);
            }

            if (logger.logged_packets >= cfg->logMaxPackets)
            {
                ret = DATALOGGER_flush(&logger);
                if (ret != K_SPP_OK)
                {
                    SPP_LOGE(k_svcTag, "DATALOGGER_Flush failed ret=%d", (int)ret);
                }

                ret = DATALOGGER_deinit(&logger);
                if (ret != K_SPP_OK)
                {
                    SPP_LOGE(k_svcTag, "DATALOGGER_Deinit failed ret=%d", (int)ret);
                }
                else
                {
                    SPP_LOGI(k_svcTag, "Datalogger finished after %u packets",
                             (unsigned)cfg->logMaxPackets);
                }

                loggerActive = 0U;
            }
        }

        ret = SPP_Databank_returnPacket(p_packetRx);
        if (ret != K_SPP_OK)
        {
            SPP_LOGE(k_svcTag, "SPP_Databank_returnPacket failed ret=%d", (int)ret);
            continue;
        }

        SPP_OSAL_taskDelayMs(K_BMP_SERVICE_TASK_DELAY_MS);
    }
}

/* ----------------------------------------------------------------
 * Service — callbacks
 * ---------------------------------------------------------------- */

static SPP_RetVal_t BMP390_Service_init(void *p_ctx, const void *p_cfg)
{
    BMP390_ServiceCtx_t       *ctx = (BMP390_ServiceCtx_t *)p_ctx;
    const BMP390_ServiceCfg_t *cfg = (const BMP390_ServiceCfg_t *)p_cfg;
    SPP_RetVal_t ret;

    ctx->p_spi  = SPP_HAL_spiGetHandle(cfg->spiDevIdx);
    ctx->seq    = 0U;
    ctx->p_cfg  = cfg;

    ctx->bmpData.intPin      = cfg->intPin;
    ctx->bmpData.intIntrType = cfg->intIntrType;
    ctx->bmpData.intPull     = cfg->intPull;

    BMP390_init(&ctx->bmpData);

    ret = BMP390_auxConfig(ctx->p_spi);
    if (ret != K_SPP_OK) { return ret; }

    ret = BMP390_prepareMeasure(ctx->p_spi);
    if (ret != K_SPP_OK) { return ret; }

    ret = BMP390_intEnableDrdy(ctx->p_spi);
    return ret;
}

static SPP_RetVal_t BMP390_Service_start(void *p_ctx)
{
    BMP390_ServiceCtx_t *ctx = (BMP390_ServiceCtx_t *)p_ctx;

    void *p_handle = SPP_OSAL_taskCreate(
        BMP390_ServiceTask, "bmp390_svc",
        K_BMP_SERVICE_TASK_STACK_SIZE, ctx,
        K_BMP_SERVICE_TASK_PRIO);

    if (p_handle == NULL)
    {
        SPP_LOGE(k_svcTag, "TaskCreate failed");
        return K_SPP_ERROR;
    }

    SPP_LOGI(k_svcTag, "Task created");
    return K_SPP_OK;
}

static SPP_RetVal_t BMP390_Service_stop(void *p_ctx)
{
    (void)p_ctx;
    return K_SPP_OK;
}

static SPP_RetVal_t BMP390_Service_deinit(void *p_ctx)
{
    (void)p_ctx;
    return K_SPP_OK;
}

/* ----------------------------------------------------------------
 * Service descriptor
 * ---------------------------------------------------------------- */

const SPP_ServiceDesc_t g_bmp390ServiceDesc = {
    .p_name  = "bmp390",
    .apid    = K_BMP_SERVICE_APID_DBG,
    .ctxSize = sizeof(BMP390_ServiceCtx_t),
    .init    = BMP390_Service_init,
    .start   = BMP390_Service_start,
    .stop    = BMP390_Service_stop,
    .deinit  = BMP390_Service_deinit,
};
