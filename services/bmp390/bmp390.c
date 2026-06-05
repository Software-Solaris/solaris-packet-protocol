/**
 * @file bmp390_service.c
 * @brief BMP390 pressure sensor driver + SPP service implementation.
 *
 * Combines the low-level SPI driver with an acquisition task that reads
 * altitude, pressure and temperature at 5 Hz and logs packets to SD card.
 */

#include "spp/services/bmp390/bmp390.h"

#include "spp/hal/spi/spi.h"
#include "spp/hal/time/time.h"
#include "spp/services/log/log.h"
#include "spp/services/databank/databank.h"
#include "spp/services/pubsub/pubsub.h"
#include "spp/core/packet.h"

#include <string.h>
#include <math.h>
#ifdef SPP_DEBUG_PRINT
#include <stdio.h>
#endif

/* ----------------------------------------------------------------
 * CONSTANTS
 * ---------------------------------------------------------------- */
#define K_BMP390_TASK_TIMEOUT_MS 5000U
#define K_BMP390_SERVICE_APID    (0x0004U)

/* ----------------------------------------------------------------
 * STATIC FUNCTIONS DEFINITION
 * ---------------------------------------------------------------- */
static SPP_RetVal_t SPP_SERVICES_BMP390_init(void);
static SPP_RetVal_t SPP_BMP390_acquireData(void *p_data);
static SPP_RetVal_t SPP_SERVICES_BMP390_softReset(void *p_spiHandler);
static SPP_RetVal_t SPP_SERVICES_BMP390_enableSpiMode(void *p_spiHandler);
static SPP_RetVal_t SPP_SERVICES_BMP390_configCheck(void *p_spiHandler);
static SPP_RetVal_t SPP_SERVICES_BMP390_auxConfig(void *p_spiHandler);
static SPP_RetVal_t SPP_SERVICES_BMP390_prepareMeasure(void *p_spiHandler);
static SPP_RetVal_t SPP_SERVICES_BMP390_waitDrdy(BMP390_Data_t *p_bmp, spp_uint32_t timeout_ms);
static SPP_RetVal_t SPP_SERVICES_BMP390_readRawTempCoeffs(void *p_spiHandler, BMP390_temp_calib_t *tcalib);
static SPP_RetVal_t SPP_SERVICES_BMP390_calibrateTempParams(void *p_spiHandler, BMP390_temp_params_t *out);
static SPP_RetVal_t SPP_SERVICES_BMP390_readRawTemp(void *p_spiHandler, uint32_t *raw_temp);
static float SPP_SERVICES_BMP390_compensateTemperature(spp_uint32_t raw_temp, BMP390_temp_params_t *params);
static SPP_RetVal_t SPP_SERVICES_BMP390_auxGetTemp(void *p_spiHandler, const BMP390_temp_params_t *temp_params,
                                                   spp_uint32_t *raw_temp, float *comp_temp);
static SPP_RetVal_t SPP_SERVICES_BMP390_readRawPressCoeffs(void *p_spiHandler, BMP390_press_calib_t *pcalib);
static SPP_RetVal_t SPP_SERVICES_BMP390_calibratePressParams(void *p_spiHandler, BMP390_press_params_t *out);
static SPP_RetVal_t SPP_SERVICES_BMP390_readRawPress(void *p_spiHandler, spp_uint32_t *raw_press);
static float SPP_SERVICES_BMP390_compensatePressure(spp_uint32_t raw_press, float t_lin, BMP390_press_params_t *p);
static SPP_RetVal_t SPP_SERVICES_BMP390_auxGetPress(void *p_spiHandler, const BMP390_press_params_t *press_params,
                                                    float t_lin, spp_uint32_t *raw_press, float *comp_press);
static SPP_RetVal_t SPP_SERVICES_BMP390_getAltitude(void *p_spiHandler, BMP390_Data_t *p_bmp, float *altitude_m,
                                                    float *pressure_pa, float *temperature_c);
static SPP_RetVal_t SPP_SERVICES_BMP390_intEnableDrdy(void *p_spiHandler);

/* ----------------------------------------------------------------
* VARIABLES
* ---------------------------------------------------------------- */
static const SPP_SERVICE_ProducerContract_t g_bmp390ProducerContract = {.producerID = K_BMP390_SERVICE_APID,
                                                                        .p_nameProducer = "bmp390",
                                                                        .tiemoutMs = K_BMP390_TASK_TIMEOUT_MS,
                                                                        .init = SPP_SERVICES_BMP390_init,
                                                                        .acquireData = SPP_BMP390_acquireData};

static BMP390_t *s_p_bmpData;

/* ----------------------------------------------------------------
* DEFINES
* ---------------------------------------------------------------- */
static const char *const k_tag = "BMP390";
static const char *const k_svcTag = "BMP_SVC";
static float s_pd1, s_pd2, s_pd3, s_pd4;
static float s_po1, s_po2;
static float s_compPress;

/* ----------------------------------------------------------------
* PUBLIC FUNCTIONS
* ---------------------------------------------------------------- */
const SPP_SERVICE_ProducerContract_t *SPP_SERVICES_BMP390_getProducerContract(void)
{
    return &g_bmp390ProducerContract;
}

/* ----------------------------------------------------------------
* STATIC FUNCTIONS
* ---------------------------------------------------------------- */
/**
* @brief    BMP390 sensor instance initialisation. This function is called by the service task.
            It is responsible for initialising the SPI bus, the GPIO ISR, and the sensor driver.
* @param    p_data    Pointer to the BMP390 sensor instance.
* @return   K_SPP_OK on success.
*/
static SPP_RetVal_t SPP_SERVICES_BMP390_init(void)
{
    SPP_RetVal_t ret = K_SPP_OK;

    s_p_bmpData->gpioConfig.intPin = K_BMP390_INT_PIN_NUM;
    s_p_bmpData->gpioConfig.intIntrType = K_BMP390_INT_INTR_TYPE;
    s_p_bmpData->gpioConfig.intPull = K_BMP390_INT_PULL;
    s_p_bmpData->spiConfig.spiDevIdx = K_BMP390_SPI_BUS_IDX;

    // Get the SPI bus handler
    s_p_bmpData->spiConfig.p_spiHandler = SPP_HAL_SPI_getHandle(s_p_bmpData->spiConfig.spiDevIdx);


    // Init the SPI bus for the BMP390 sensor
    (void)SPP_HAL_SPI_deviceInit(SPP_HAL_SPI_getHandle(s_p_bmpData->spiConfig.spiDevIdx));


    if (ret == K_SPP_OK)
    {
        ret = SPP_HAL_GPIO_configInterrupt(s_p_bmpData->gpioConfig.intPin, s_p_bmpData->gpioConfig.intIntrType,
                                           s_p_bmpData->gpioConfig.intPull);
    }

    if (ret == K_SPP_OK)
    {
        ret = SPP_HAL_GPIO_registerIsr(s_p_bmpData->gpioConfig.intPin, (volatile void *)&s_p_bmpData->bmpData.drdyFlag);
    }

    if (ret == K_SPP_OK)
    {
        ret = SPP_SERVICES_BMP390_auxConfig(s_p_bmpData->spiConfig.p_spiHandler);
    }

    if (ret == K_SPP_OK)
    {
        (void)SPP_SERVICES_BMP390_prepareMeasure(s_p_bmpData->spiConfig.p_spiHandler);

        ret = SPP_SERVICES_BMP390_intEnableDrdy(s_p_bmpData->spiConfig.p_spiHandler);
        if (ret != K_SPP_OK)
        {
            ret = K_SPP_ERROR;
        }
    }

    return ret;
}


/**
 * @brief  Acquisition callback invoked by the SPP service task on each cycle.
 *
 * Checks the DRDY flag, reads compensated altitude, pressure and temperature
 * from the sensor, packs the values into an SPP packet and publishes it.
 * Silently returns if DRDY is not set or no free packet is available.
 *
 * @param  p_data  Pointer to the BMP390_t sensor instance.
 */
static SPP_RetVal_t SPP_BMP390_acquireData(void *p_data)
{
    BMP390_t *ctx = (BMP390_t *)p_data;
    float altitude = 0.0f;
    float pressure = 0.0f;
    float temperature = 0.0f;

    if (!ctx->bmpData.drdyFlag)
    {
        return K_SPP_OK;
    }
    ctx->bmpData.drdyFlag = false;

    SPP_Packet_t *p_packet = SPP_SERVICES_DATABANK_getPacket();
    if (p_packet == NULL)
    {
        SPP_LOGW(k_svcTag, "No free packet");
        return K_SPP_ERROR;
    }

    SPP_RetVal_t ret =
        SPP_SERVICES_BMP390_getAltitude(ctx->spiConfig.p_spiHandler, &ctx->bmpData, &altitude, &pressure, &temperature);
    if (ret != K_SPP_OK)
    {
        SPP_LOGE(k_svcTag, "getAltitude failed ret=%d", (int)ret);
        (void)SPP_SERVICES_DATABANK_returnPacket(p_packet);
        return ret;
    }

#ifdef SPP_DEBUG_PRINT
    printf("[BMP] alt=%.1fm P=%.1fhPa T=%.2fC\n", altitude, pressure / 100.0f, temperature);
#endif

    float payload[3] = {altitude, pressure, temperature};
    ret = SPP_SERVICES_DATABANK_packetData(p_packet, K_BMP390_SERVICE_APID, ctx->seq++, payload,
                                           (spp_uint16_t)sizeof(payload));
    if (ret != K_SPP_OK)
    {
        SPP_LOGE(k_svcTag, "packetData failed ret=%d", (int)ret);
        (void)SPP_SERVICES_DATABANK_returnPacket(p_packet);
        return ret;
    }

    (void)SPP_SERVICES_PUBSUB_publish(p_packet);
    return K_SPP_OK;
}


/* ----------------------------------------------------------------
 * Driver — configuration helpers
 * ---------------------------------------------------------------- */

/**
 * @brief  Sends a soft-reset command to the sensor and waits 100 ms for it to reboot.
 * @param  p_spiHandler  SPI device handle.
 * @return K_SPP_OK on success, K_SPP_ERROR on SPI failure.
 */
static SPP_RetVal_t SPP_SERVICES_BMP390_softReset(void *p_spiHandler)
{
    spp_uint8_t buf[2] = {(spp_uint8_t)K_BMP390_SOFT_RESET_REG, (spp_uint8_t)BMP390_SOFT_RESET_CMD};
    SPP_RetVal_t ret = SPP_HAL_SPI_transmit(p_spiHandler, buf, sizeof(buf));
    SPP_HAL_TIME_delayMs(100U);
    return ret;
}

/**
 * @brief  Writes the interface-config register to lock the sensor into SPI mode.
 * @param  p_spiHandler  SPI device handle.
 * @return K_SPP_OK on success, K_SPP_ERROR on SPI failure.
 */
static SPP_RetVal_t SPP_SERVICES_BMP390_enableSpiMode(void *p_spiHandler)
{
    spp_uint8_t buf[2] = {(spp_uint8_t)K_BMP390_IF_CONF_REG, (spp_uint8_t)BMP390_IF_CONF_SPI};
    SPP_RetVal_t ret = SPP_HAL_SPI_transmit(p_spiHandler, buf, (spp_uint8_t)sizeof(buf));
    SPP_HAL_TIME_delayMs(100U);
    return ret;
}

/**
 * @brief  Reads the chip-ID register and returns an error if the value is not 0x60.
 * @param  p_spiHandler  SPI device handle.
 * @return K_SPP_OK if chip ID matches, K_SPP_ERROR otherwise.
 */
static SPP_RetVal_t SPP_SERVICES_BMP390_configCheck(void *p_spiHandler)
{
    spp_uint8_t buf[9] = {
        (spp_uint8_t)(K_BMP390_SPI_READ | K_BMP390_IF_CONF_REG),    K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE,
        (spp_uint8_t)(K_BMP390_SPI_READ | K_BMP390_SOFT_RESET_REG), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE,
        (spp_uint8_t)(K_BMP390_SPI_READ | K_BMP390_CHIP_ID_REG),    K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE};

    SPP_RetVal_t ret = SPP_HAL_SPI_transmit(p_spiHandler, buf, (spp_uint8_t)sizeof(buf));
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

/**
 * @brief  Resets the sensor, enables SPI mode, and verifies the chip ID.
 * @param  p_spiHandler  SPI device handle.
 * @return K_SPP_OK on success, K_SPP_ERROR if any step fails.
 */
static SPP_RetVal_t SPP_SERVICES_BMP390_auxConfig(void *p_spiHandler)
{
    SPP_RetVal_t ret;

    ret = SPP_SERVICES_BMP390_softReset(p_spiHandler);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_BMP390_enableSpiMode(p_spiHandler);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_BMP390_configCheck(p_spiHandler);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    return ret;
}

/**
 * @brief  Writes OSR, ODR, IIR and power-control registers to start continuous measurements.
 * @param  p_spiHandler  SPI device handle.
 * @return K_SPP_OK on success, K_SPP_ERROR on SPI failure.
 */
static SPP_RetVal_t SPP_SERVICES_BMP390_prepareMeasure(void *p_spiHandler)
{
    spp_uint8_t buf[8] = {(spp_uint8_t)K_BMP390_REG_OSR,     (spp_uint8_t)BMP390_VALUE_OSR,
                          (spp_uint8_t)K_BMP390_REG_ODR,     (spp_uint8_t)BMP390_VALUE_ODR,
                          (spp_uint8_t)K_BMP390_REG_IIR,     (spp_uint8_t)BMP390_VALUE_IIR,
                          (spp_uint8_t)K_BMP390_REG_PWRCTRL, (spp_uint8_t)BMP390_VALUE_PWRCTRL};

    return SPP_HAL_SPI_transmit(p_spiHandler, buf, sizeof(buf));
}

/**
 * @brief  Polls the DRDY flag until set or until the timeout elapses.
 * @param  p_bmp        Pointer to the driver context containing the ISR flag.
 * @param  timeout_ms   Maximum time to wait in milliseconds.
 * @return K_SPP_OK when data is ready, K_SPP_ERROR on timeout.
 */
static SPP_RetVal_t SPP_SERVICES_BMP390_waitDrdy(BMP390_Data_t *p_bmp, spp_uint32_t timeout_ms)
{
    spp_uint32_t start = SPP_HAL_TIME_getTimeMs();

    while (!p_bmp->drdyFlag)
    {
        if ((SPP_HAL_TIME_getTimeMs() - start) >= timeout_ms)
        {
            return K_SPP_ERROR;
        }
    }
    p_bmp->drdyFlag = false;
    return K_SPP_OK;
}

/* ----------------------------------------------------------------
 * Driver — temperature
 * ---------------------------------------------------------------- */

/**
 * @brief  Reads raw temperature calibration coefficients from the sensor's NVM registers.
 * @param  p_spiHandler  SPI device handle.
 * @param  tcalib        Output struct populated with raw par_t1/t2/t3 values.
 * @return K_SPP_OK on success, K_SPP_ERROR on SPI failure.
 */
static SPP_RetVal_t SPP_SERVICES_BMP390_readRawTempCoeffs(void *p_spiHandler, BMP390_temp_calib_t *tcalib)
{
    spp_uint8_t buf[15] = {
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_TEMP_CALIB_REG_START + 0)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE,
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_TEMP_CALIB_REG_START + 1)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE,
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_TEMP_CALIB_REG_START + 2)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE,
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_TEMP_CALIB_REG_START + 3)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE,
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_TEMP_CALIB_REG_START + 4)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE};

    SPP_RetVal_t ret = SPP_HAL_SPI_transmit(p_spiHandler, buf, sizeof(buf));
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

/**
 * @brief  Reads raw temperature coefficients and scales them to floating-point parameters.
 * @param  p_spiHandler  SPI device handle.
 * @param  out           Output struct populated with scaled PAR_T1/T2/T3.
 * @return K_SPP_OK on success, K_SPP_ERROR on SPI failure.
 */
static SPP_RetVal_t SPP_SERVICES_BMP390_calibrateTempParams(void *p_spiHandler, BMP390_temp_params_t *out)
{
    BMP390_temp_calib_t raw;

    SPP_RetVal_t ret = SPP_SERVICES_BMP390_readRawTempCoeffs(p_spiHandler, &raw);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    out->PAR_T1 = raw.par_t1 * 256.0f;             /* 2^8  */
    out->PAR_T2 = raw.par_t2 / 1073741824.0f;      /* 2^30 */
    out->PAR_T3 = raw.par_t3 / 281474976710656.0f; /* 2^48 */

    return ret;
}

/**
 * @brief  Reads the 24-bit raw temperature ADC value from the sensor.
 * @param  p_spiHandler  SPI device handle.
 * @param  raw_temp      Output: raw 24-bit ADC reading.
 * @return K_SPP_OK on success, K_SPP_ERROR on SPI failure.
 */
static SPP_RetVal_t SPP_SERVICES_BMP390_readRawTemp(void *p_spiHandler, uint32_t *raw_temp)
{
    spp_uint8_t buf[9] = {
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_TEMP_RAW_REG + 0)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE,
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_TEMP_RAW_REG + 1)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE,
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_TEMP_RAW_REG + 2)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE};

    SPP_RetVal_t ret = SPP_HAL_SPI_transmit(p_spiHandler, buf, sizeof(buf));
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    spp_uint8_t xlsb = buf[2];
    spp_uint8_t lsb = buf[5];
    spp_uint8_t msb = buf[8];

    *raw_temp = ((spp_uint32_t)msb << 16) | ((spp_uint32_t)lsb << 8) | (spp_uint32_t)xlsb;

    return ret;
}

/**
 * @brief  Applies the BMP390 double-quadratic temperature compensation formula.
 * @param  raw_temp  Raw 24-bit ADC reading from the sensor.
 * @param  params    Scaled calibration parameters (PAR_T1/T2/T3).
 * @return Compensated temperature in degrees Celsius (t_lin).
 */
static float SPP_SERVICES_BMP390_compensateTemperature(spp_uint32_t raw_temp, BMP390_temp_params_t *params)
{
    float partial1 = (float)raw_temp - params->PAR_T1;
    float partial2 = partial1 * params->PAR_T2;
    float t_lin = partial2 + (partial1 * partial1) * params->PAR_T3;

    return t_lin;
}

/**
 * @brief  Reads the raw temperature ADC and returns the compensated value.
 * @param  p_spiHandler  SPI device handle.
 * @param  temp_params   Scaled calibration parameters.
 * @param  raw_temp      Output: raw ADC reading.
 * @param  comp_temp     Output: compensated temperature in °C.
 * @return K_SPP_OK on success, K_SPP_ERROR on SPI failure.
 */
static SPP_RetVal_t SPP_SERVICES_BMP390_auxGetTemp(void *p_spiHandler, const BMP390_temp_params_t *temp_params,
                                                   spp_uint32_t *raw_temp, float *comp_temp)
{
    SPP_RetVal_t ret = SPP_SERVICES_BMP390_readRawTemp(p_spiHandler, raw_temp);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    *comp_temp = SPP_SERVICES_BMP390_compensateTemperature(*raw_temp, (BMP390_temp_params_t *)temp_params);

    return ret;
}

/* ----------------------------------------------------------------
 * Driver — pressure
 * ---------------------------------------------------------------- */

/**
 * @brief  Reads raw pressure calibration coefficients from the sensor's NVM registers.
 * @param  p_spiHandler  SPI device handle.
 * @param  pcalib        Output struct populated with raw par_p1..p11 values.
 * @return K_SPP_OK on success, K_SPP_ERROR on SPI failure.
 */
static SPP_RetVal_t SPP_SERVICES_BMP390_readRawPressCoeffs(void *p_spiHandler, BMP390_press_calib_t *pcalib)
{
    spp_uint8_t buf[48] = {(spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_CALIB_REG_START + 0)),
                           K_BMP390_SPI_WRITE,
                           K_BMP390_SPI_WRITE,
                           (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_CALIB_REG_START + 1)),
                           K_BMP390_SPI_WRITE,
                           K_BMP390_SPI_WRITE,
                           (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_CALIB_REG_START + 2)),
                           K_BMP390_SPI_WRITE,
                           K_BMP390_SPI_WRITE,
                           (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_CALIB_REG_START + 3)),
                           K_BMP390_SPI_WRITE,
                           K_BMP390_SPI_WRITE,
                           (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_CALIB_REG_START + 4)),
                           K_BMP390_SPI_WRITE,
                           K_BMP390_SPI_WRITE,
                           (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_CALIB_REG_START + 5)),
                           K_BMP390_SPI_WRITE,
                           K_BMP390_SPI_WRITE,
                           (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_CALIB_REG_START + 6)),
                           K_BMP390_SPI_WRITE,
                           K_BMP390_SPI_WRITE,
                           (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_CALIB_REG_START + 7)),
                           K_BMP390_SPI_WRITE,
                           K_BMP390_SPI_WRITE,
                           (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_CALIB_REG_START + 8)),
                           K_BMP390_SPI_WRITE,
                           K_BMP390_SPI_WRITE,
                           (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_CALIB_REG_START + 9)),
                           K_BMP390_SPI_WRITE,
                           K_BMP390_SPI_WRITE,
                           (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_CALIB_REG_START + 10)),
                           K_BMP390_SPI_WRITE,
                           K_BMP390_SPI_WRITE,
                           (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_CALIB_REG_START + 11)),
                           K_BMP390_SPI_WRITE,
                           K_BMP390_SPI_WRITE,
                           (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_CALIB_REG_START + 12)),
                           K_BMP390_SPI_WRITE,
                           K_BMP390_SPI_WRITE,
                           (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_CALIB_REG_START + 13)),
                           K_BMP390_SPI_WRITE,
                           K_BMP390_SPI_WRITE,
                           (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_CALIB_REG_START + 14)),
                           K_BMP390_SPI_WRITE,
                           K_BMP390_SPI_WRITE,
                           (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_CALIB_REG_START + 15)),
                           K_BMP390_SPI_WRITE,
                           K_BMP390_SPI_WRITE};

    SPP_RetVal_t ret = SPP_HAL_SPI_transmit(p_spiHandler, buf, sizeof(buf));
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    spp_uint8_t raw[16];
    for (int i = 0; i < 16; i++)
    {
        raw[i] = buf[3 * i + 2];
    }

    pcalib->par_p1 = (spp_uint16_t)((raw[1] << 8) | raw[0]);
    pcalib->par_p2 = (spp_uint16_t)((raw[3] << 8) | raw[2]);
    pcalib->par_p3 = (spp_int8_t)raw[4];
    pcalib->par_p4 = (spp_int8_t)raw[5];
    pcalib->par_p5 = (spp_uint16_t)((raw[7] << 8) | raw[6]);
    pcalib->par_p6 = (spp_uint16_t)((raw[9] << 8) | raw[8]);
    pcalib->par_p7 = (spp_int8_t)raw[10];
    pcalib->par_p8 = (spp_int8_t)raw[11];
    pcalib->par_p9 = (spp_int16_t)((raw[13] << 8) | raw[12]);
    pcalib->par_p10 = (spp_int8_t)raw[14];
    pcalib->par_p11 = (spp_int8_t)raw[15];

    return ret;
}

/**
 * @brief  Reads raw pressure coefficients and scales them to floating-point parameters.
 * @param  p_spiHandler  SPI device handle.
 * @param  out           Output struct populated with scaled PAR_P1..P11.
 * @return K_SPP_OK on success, K_SPP_ERROR on SPI failure.
 */
static SPP_RetVal_t SPP_SERVICES_BMP390_calibratePressParams(void *p_spiHandler, BMP390_press_params_t *out)
{
    BMP390_press_calib_t raw;

    SPP_RetVal_t ret = SPP_SERVICES_BMP390_readRawPressCoeffs(p_spiHandler, &raw);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    out->PAR_P1 = (raw.par_p1 - 16384.0f) / 1048576.0f;   /* (p1 - 2^14) / 2^20 */
    out->PAR_P2 = (raw.par_p2 - 16384.0f) / 536870912.0f; /* (p2 - 2^14) / 2^29 */
    out->PAR_P3 = raw.par_p3 / 4294967296.0f;             /* / 2^32 */
    out->PAR_P4 = raw.par_p4 / 137438953472.0f;           /* / 2^37 */
    out->PAR_P5 = raw.par_p5 * 8.0f;                      /* / 2^-3 */
    out->PAR_P6 = raw.par_p6 / 64.0f;                     /* / 2^6  */
    out->PAR_P7 = raw.par_p7 / 256.0f;                    /* / 2^8  */
    out->PAR_P8 = raw.par_p8 / 32768.0f;                  /* / 2^15 */
    out->PAR_P9 = raw.par_p9 / 281474976710656.0f;        /* / 2^48 */
    out->PAR_P10 = raw.par_p10 / 281474976710656.0f;      /* / 2^48 */
    out->PAR_P11 = raw.par_p11 / 36893488147419103232.0f; /* / 2^65 */

    return ret;
}

/**
 * @brief  Reads the 24-bit raw pressure ADC value from the sensor.
 * @param  p_spiHandler  SPI device handle.
 * @param  raw_press     Output: raw 24-bit ADC reading.
 * @return K_SPP_OK on success, K_SPP_ERROR on SPI failure.
 */
static SPP_RetVal_t SPP_SERVICES_BMP390_readRawPress(void *p_spiHandler, spp_uint32_t *raw_press)
{
    spp_uint8_t buf[9] = {
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_RAW_REG + 0)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE,
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_RAW_REG + 1)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE,
        (spp_uint8_t)(K_BMP390_SPI_READ | (K_BMP390_PRESS_RAW_REG + 2)), K_BMP390_SPI_WRITE, K_BMP390_SPI_WRITE};

    SPP_RetVal_t ret = SPP_HAL_SPI_transmit(p_spiHandler, buf, sizeof(buf));
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    spp_uint8_t xlsb = buf[2];
    spp_uint8_t lsb = buf[5];
    spp_uint8_t msb = buf[8];

    *raw_press = ((spp_uint32_t)msb << 16) | ((spp_uint32_t)lsb << 8) | (spp_uint32_t)xlsb;

    return ret;
}

/**
 * @brief  Applies the BMP390 pressure compensation formula using the linearised temperature.
 * @param  raw_press  Raw 24-bit ADC pressure reading.
 * @param  t_lin      Linearised (compensated) temperature in °C.
 * @param  p          Scaled pressure calibration parameters.
 * @return Compensated pressure in Pa.
 */
static float SPP_SERVICES_BMP390_compensatePressure(spp_uint32_t raw_press, float t_lin, BMP390_press_params_t *p)
{
    s_pd1 = p->PAR_P6 * t_lin;
    s_pd2 = p->PAR_P7 * (t_lin * t_lin);
    s_pd3 = p->PAR_P8 * (t_lin * t_lin * t_lin);
    s_po1 = p->PAR_P5 + s_pd1 + s_pd2 + s_pd3;

    s_pd1 = p->PAR_P2 * t_lin;
    s_pd2 = p->PAR_P3 * (t_lin * t_lin);
    s_pd3 = p->PAR_P4 * (t_lin * t_lin * t_lin);
    s_po2 = raw_press * (p->PAR_P1 + s_pd1 + s_pd2 + s_pd3);

    s_pd1 = raw_press * raw_press;
    s_pd2 = p->PAR_P9 + p->PAR_P10 * t_lin;
    s_pd3 = s_pd1 * s_pd2;
    s_pd4 = s_pd3 + (raw_press * raw_press * raw_press) * p->PAR_P11;

    s_compPress = s_po1 + s_po2 + s_pd4;

    return s_compPress;
}

/**
 * @brief  Reads raw pressure ADC and returns the compensated value in Pa.
 * @param  p_spiHandler  SPI device handle.
 * @param  press_params  Scaled pressure calibration parameters.
 * @param  t_lin         Linearised temperature used for pressure compensation.
 * @param  raw_press     Output: raw 24-bit ADC reading.
 * @param  comp_press    Output: compensated pressure in Pa.
 * @return K_SPP_OK on success, K_SPP_ERROR on SPI failure.
 */
static SPP_RetVal_t SPP_SERVICES_BMP390_auxGetPress(void *p_spiHandler, const BMP390_press_params_t *press_params,
                                                    float t_lin, spp_uint32_t *raw_press, float *comp_press)
{
    SPP_RetVal_t ret = SPP_SERVICES_BMP390_readRawPress(p_spiHandler, raw_press);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    *comp_press = SPP_SERVICES_BMP390_compensatePressure(*raw_press, t_lin, (BMP390_press_params_t *)press_params);

    return ret;
}

/* ----------------------------------------------------------------
 * Driver — altitude
 * ---------------------------------------------------------------- */

/**
 * @brief  Reads compensated altitude, pressure and temperature from the sensor.
 * @param  p_spiHandler   SPI device handle.
 * @param  p_bmp          Driver context (unused; DRDY is handled by the caller).
 * @param  altitude_m     Output: altitude in metres (barometric formula, ref 1013.25 hPa).
 * @param  pressure_pa    Output: compensated pressure in Pa.
 * @param  temperature_c  Output: compensated temperature in °C.
 * @return K_SPP_OK on success, K_SPP_ERROR_NULL_POINTER if any pointer is NULL,
 *         K_SPP_ERROR on SPI failure.
 */
static SPP_RetVal_t SPP_SERVICES_BMP390_getAltitude(void *p_spiHandler, BMP390_Data_t *p_bmp, float *altitude_m,
                                                    float *pressure_pa, float *temperature_c)
{
    (void)p_bmp; /* DRDY wait is handled by the caller */

    if ((p_spiHandler == NULL) || (altitude_m == NULL) || (pressure_pa == NULL) || (temperature_c == NULL))
    {
        return K_SPP_ERROR_NULL_POINTER;
    }

    static spp_bool_t s_inited = false;
    static BMP390_temp_params_t s_tempParams;
    static BMP390_press_params_t s_pressParams;
    static spp_uint32_t s_rawTemp;
    static spp_uint32_t s_rawPress;

    float t_lin = 0.0f;
    float compPress = 0.0f;

    if (s_inited == false)
    {
        SPP_RetVal_t ret = SPP_SERVICES_BMP390_calibrateTempParams(p_spiHandler, &s_tempParams);
        if (ret != K_SPP_OK)
        {
            return ret;
        }

        ret = SPP_SERVICES_BMP390_calibratePressParams(p_spiHandler, &s_pressParams);
        if (ret != K_SPP_OK)
        {
            return ret;
        }

        s_inited = true;
    }

    SPP_RetVal_t ret = SPP_SERVICES_BMP390_auxGetTemp(p_spiHandler, &s_tempParams, &s_rawTemp, &t_lin);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_BMP390_auxGetPress(p_spiHandler, &s_pressParams, t_lin, &s_rawPress, &compPress);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    *temperature_c = t_lin;
    *pressure_pa = compPress;
    *altitude_m = 44330.0f * (1.0f - powf(compPress / 101325.0f, 1.0f / 5.255f));

    return K_SPP_OK;
}

/* ----------------------------------------------------------------
 * Driver — interrupt
 * ---------------------------------------------------------------- */

/**
 * @brief  Enables the active-high data-ready interrupt output on the sensor.
 * @param  p_spiHandler  SPI device handle.
 * @return K_SPP_OK on success, K_SPP_ERROR on SPI failure.
 */
static SPP_RetVal_t SPP_SERVICES_BMP390_intEnableDrdy(void *p_spiHandler)
{
    spp_uint8_t buf[2] = {K_BMP390_REG_INT_CTRL, (spp_uint8_t)(K_BMP390_INT_CTRL_LEVEL | K_BMP390_INT_CTRL_DRDY_EN)};

    return SPP_HAL_SPI_transmit(p_spiHandler, buf, sizeof(buf));
}

/* ----------------------------------------------------------------
 * Service — callbacks
 * ---------------------------------------------------------------- */


static SPP_RetVal_t bmp390Start(void *p_data)
{
    (void)p_data;
    SPP_LOGI(k_svcTag, "Ready");
    return K_SPP_OK;
}

static SPP_RetVal_t bmp390Stop(void *p_data)
{
    (void)p_data;
    return K_SPP_OK;
}
static SPP_RetVal_t bmp390Deinit(void *p_data)
{
    (void)p_data;
    return K_SPP_OK;
}