/**
 * @file icm20948_service.c
 * @brief ICM20948 IMU driver + SPP service implementation.
 *
 * Implements SPI register access, DMP firmware loading and the full
 * DMP initialisation sequence.  Also exposes a minimal SPP service
 * descriptor so the device can be registered in the service registry.
 */

#include "spp/services/icm20948/icm20948.h"

#include "spp/core/returnTypes.h"
#include "spp/hal/spi.h"
#include "spp/hal/gpio.h"
#include "spp/hal/time.h"
#include "spp/core/types.h"
#include "spp/services/log/log.h"
#include "spp/services/databank/databank.h"
#include "spp/services/pubsub/pubsub.h"

#include <string.h>
#include <math.h>

/* ----------------------------------------------------------------
 * Private constants
 * ---------------------------------------------------------------- */

/** @brief SPI read flag — set MSB to signal a read transaction. */
#define K_READ_OP 0x80U

/** @brief SPI write marker (zero). */
#define K_WRITE_OP 0x00U


#define K_ICM20948_BASE_SAMPLE_RATE      1125L
#define K_ICM20948_DMP_RUNNING_RATE      225L
#define K_ICM20948_DMP_DIVIDER           (K_ICM20948_BASE_SAMPLE_RATE / K_ICM20948_DMP_RUNNING_RATE)
#define K_ICM20948_WHO_AM_I_VALUE        0xEAU
#define K_ICM20948_DMP_PACKET_SIZE_BYTES 24U
#define K_ICM20948_FIFO_RESET_THRESHOLD  512U
#define K_ICM20948_DMP_START_ADDR_MSB    0x10U
#define K_ICM20948_DMP_START_ADDR_LSB    0x00U
#define K_ICM20948_LOG_TAG               "ICM"

/* ----------------------------------------------------------------
 * Private types
 * ---------------------------------------------------------------- */

typedef struct
{
    spp_uint16_t addr;
    spp_uint32_t val;
} ICM20948_DmpMatrixEntry_t;

/* ----------------------------------------------------------------
 * DMP firmware image
 * ---------------------------------------------------------------- */

static const spp_uint8_t s_dmp3Image[] = {
#include "dmpImage.h"
};

/* ----------------------------------------------------------------
 * Private helpers
 * ---------------------------------------------------------------- */

static SPP_RetVal_t SPP_SERVICES_ICM20948_writeReg(void *p_spi, spp_uint8_t reg, spp_uint8_t value)
{
    spp_uint8_t txBuffer[2] = {K_WRITE_OP | reg, value};
    return SPP_HAL_spiTransmit(p_spi, txBuffer, 2U);
}

static SPP_RetVal_t SPP_SERVICES_ICM20948_readReg(void *p_spi, spp_uint8_t reg, spp_uint8_t *p_value)
{
    /* HAL always issues a 3-byte SPI frame for reads (addr + 2 data bytes);
     * ICM-20948 has no dummy byte so data lands at buf[1]. buf[2] absorbs the
     * extra byte clocked out — must exist in the array to avoid a stack write. */
    spp_uint8_t txRxBuffer[3] = {K_READ_OP | reg, K_WRITE_OP, K_WRITE_OP};
    SPP_RetVal_t ret = SPP_HAL_spiTransmit(p_spi, txRxBuffer, 3U);

    if (p_value != NULL)
    {
        *p_value = txRxBuffer[1];
    }

    return ret;
}

/* Read `count` bytes from FIFO_R_W.  The HAL always issues 3-byte read frames
 * (address + 2 data bytes), so each frame yields 2 FIFO bytes.  count must be
 * even (K_ICM20948_DMP_PACKET_SIZE_BYTES = 40 satisfies this). */
static SPP_RetVal_t readFifoBurst(void *p_spi, spp_uint8_t *p_dst, spp_uint8_t count)
{
    spp_uint8_t frame[3];
    spp_uint8_t i = 0U;
    while (i < count)
    {
        frame[0] = K_READ_OP | K_ICM20948_REG_FIFO_R_W;
        frame[1] = 0U;
        frame[2] = 0U;
        SPP_RetVal_t ret = SPP_HAL_spiTransmit(p_spi, frame, 3U);
        if (ret != K_SPP_OK)
        {
            return ret;
        }
        p_dst[i] = frame[1];
        i++;
        if (i < count)
        {
            p_dst[i] = frame[2];
            i++;
        }
    }
    return K_SPP_OK;
}

static SPP_RetVal_t SPP_SERVICES_ICM20948_setBank(void *p_spi, ICM20948_RegBank_t regBank)
{
    ICM20948_RegBankSel_t regBankSel = {.value = 0U};

    switch (regBank)
    {
        case K_ICM20948_REG_BANK_0:
            regBankSel.bits.bankSel = 0U;
            break;
        case K_ICM20948_REG_BANK_1:
            regBankSel.bits.bankSel = 1U;
            break;
        case K_ICM20948_REG_BANK_2:
            regBankSel.bits.bankSel = 2U;
            break;
        case K_ICM20948_REG_BANK_3:
            regBankSel.bits.bankSel = 3U;
            break;
        default:
            return K_SPP_ERROR;
    }

    return SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_BANK_SEL, regBankSel.value);
}

static SPP_RetVal_t SPP_SERVICES_ICM20948_resetFifo(void *p_spi)
{
    ICM20948_RegFifoRst_t fifoResetReg = {.value = 0U};
    SPP_RetVal_t ret;

    fifoResetReg.bits.fifoRst0 = 1U;
    fifoResetReg.bits.fifoRst1 = 1U;
    fifoResetReg.bits.fifoRst2 = 1U;
    fifoResetReg.bits.fifoRst3 = 1U;
    fifoResetReg.bits.fifoRst4 = 1U;

    ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_FIFO_RST, fifoResetReg.value);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    fifoResetReg.bits.fifoRst0 = 0U;

    return SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_FIFO_RST, fifoResetReg.value);
}

static SPP_RetVal_t SPP_SERVICES_ICM20948_dmpWriteBytes(void *p_data, spp_uint16_t addr,
                                           const spp_uint8_t *p_bytes, spp_uint8_t len)
{
    void *p_spi = p_data;
    SPP_RetVal_t ret;

    if ((p_spi == NULL) || (p_bytes == NULL))
    {
        return K_SPP_ERROR_NULL_POINTER;
    }

    for (spp_uint8_t i = 0U; i < len; i++)
    {
        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_MEM_BANK_SEL, (spp_uint8_t)((addr + i) >> 8));
        if (ret != K_SPP_OK)
        {
            return ret;
        }

        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_MEM_START_ADDR,
                                (spp_uint8_t)((addr + i) & 0xFFU));
        if (ret != K_SPP_OK)
        {
            return ret;
        }

        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_MEM_R_W, p_bytes[i]);
        if (ret != K_SPP_OK)
        {
            return ret;
        }
    }

    return K_SPP_OK;
}

static SPP_RetVal_t SPP_SERVICES_ICM20948_dmpWrite32(void *p_data, spp_uint16_t addr, spp_uint32_t value)
{
    spp_uint8_t bytes[4] = {(spp_uint8_t)(value >> 24), (spp_uint8_t)(value >> 16),
                            (spp_uint8_t)(value >> 8), (spp_uint8_t)value};

    return SPP_SERVICES_ICM20948_dmpWriteBytes(p_data, addr, bytes, 4U);
}

static SPP_RetVal_t SPP_SERVICES_ICM20948_dmpWrite16(void *p_data, spp_uint16_t addr, spp_uint16_t value)
{
    spp_uint8_t bytes[2] = {(spp_uint8_t)(value >> 8), (spp_uint8_t)value};

    return SPP_SERVICES_ICM20948_dmpWriteBytes(p_data, addr, bytes, 2U);
}

static SPP_RetVal_t SPP_SERVICES_ICM20948_lpWakeCycle(void *p_data)
{
    void *p_spi = p_data;
    ICM20948_RegPwrMgmt1_t pwrMgmt1Reg = {.value = 0U};
    SPP_RetVal_t ret;

    if (p_spi == NULL)
    {
        return K_SPP_ERROR_NULL_POINTER;
    }

    pwrMgmt1Reg.bits.clkSel = K_ICM20948_CLK_AUTO;
    pwrMgmt1Reg.bits.lpEn = 1U;

    ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_PWR_MGMT_1, pwrMgmt1Reg.value);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    pwrMgmt1Reg.value = 0U;
    pwrMgmt1Reg.bits.clkSel = K_ICM20948_CLK_AUTO;

    return SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_PWR_MGMT_1, pwrMgmt1Reg.value);
}

static spp_int32_t SPP_SERVICES_ICM20948_calcGyroSf(spp_int8_t pll)
{
    spp_int32_t t = 102870L + 81L * (spp_int32_t)pll;
    spp_int32_t a = (1L << 30) / t;
    spp_int32_t r = (1L << 30) - a * t;
    spp_int32_t v = a * 797L * K_ICM20948_DMP_DIVIDER;

    v += (spp_int32_t)(((spp_int64_t)a * 1011387LL * K_ICM20948_DMP_DIVIDER) >> 20);
    v += r * 797L * K_ICM20948_DMP_DIVIDER / t;
    v += (spp_int32_t)(((spp_int64_t)r * 1011387LL * K_ICM20948_DMP_DIVIDER) >> 20) / t;

    return v << 1;
}

static SPP_RetVal_t ICM20948_dmpWriteOutputConfig(void *p_data, spp_uint16_t outCtl1,
                                                  spp_uint16_t motionEvent)
{
    SPP_RetVal_t ret;

    ret = SPP_SERVICES_ICM20948_dmpWrite16(p_data, K_ICM20948_DMP_DATA_OUT_CTL1, outCtl1);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_dmpWrite16(p_data, K_ICM20948_DMP_DATA_INTR_CTL, outCtl1);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_dmpWrite16(p_data, K_ICM20948_DMP_DATA_OUT_CTL2, 0x0000U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_dmpWrite16(p_data, K_ICM20948_DMP_MOTION_EVENT_CTL, motionEvent);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    return K_SPP_OK;
}

/* ----------------------------------------------------------------
 * Public driver API
 * ---------------------------------------------------------------- */

SPP_RetVal_t SPP_SERVICES_ICM20948_loadDmp(void *p_data)
{
    void *p_spi = p_data;
    const spp_uint8_t *p_firmware = s_dmp3Image;
    spp_uint16_t firmwareSize = (spp_uint16_t)sizeof(s_dmp3Image);
    spp_uint16_t done;
    spp_uint16_t loadAddr;
    SPP_RetVal_t ret;

    if (p_spi == NULL)
    {
        return K_SPP_ERROR_NULL_POINTER;
    }

    ret = SPP_SERVICES_ICM20948_setBank(p_spi, K_ICM20948_REG_BANK_0);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    /* ---- Write: byte-by-byte, MEM_BANK_SEL only on change ---- */
    spp_uint8_t lastBank = 0xFFU;

    done     = 0U;
    loadAddr = K_ICM20948_DMP_LOAD_START;
    while (done < firmwareSize)
    {
        spp_uint8_t bank   = (spp_uint8_t)(loadAddr >> 8);
        spp_uint8_t offset = (spp_uint8_t)(loadAddr & 0xFFU);

        if (bank != lastBank)
        {
            ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_MEM_BANK_SEL, bank);
            if (ret != K_SPP_OK)
            {
                return ret;
            }
            lastBank = bank;
        }

        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_MEM_START_ADDR, offset);
        if (ret != K_SPP_OK)
        {
            return ret;
        }
        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_MEM_R_W, p_firmware[done]);
        if (ret != K_SPP_OK)
        {
            return ret;
        }

        done++;
        loadAddr = (spp_uint16_t)(loadAddr + 1U);
    }

    /* Prime the SRAM read path — the DMP memory controller needs one read transaction
     * after the write burst before verify reads return correct values. */
    {
        spp_uint8_t primeVal = 0U;
        (void)SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_MEM_BANK_SEL, 0x00U);
        (void)SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_MEM_START_ADDR, 0x91U);
        (void)SPP_SERVICES_ICM20948_readReg(p_spi, K_ICM20948_REG_MEM_R_W, &primeVal);
    }

    return K_SPP_OK;
}

SPP_RetVal_t SPP_SERVICES_ICM20948_configDmpInit(void *p_data)
{
    void *p_spi = p_data;
    SPP_RetVal_t ret;
    spp_uint8_t whoAmIValue;
    spp_uint8_t pllRaw;
    spp_int8_t pllTrim;

    static const ICM20948_DmpMatrixEntry_t s_cpassMatrix[] = {
        {K_ICM20948_DMP_CPASS_MTX_00, 0x09999999U}, {K_ICM20948_DMP_CPASS_MTX_01, 0x00000000U},
        {K_ICM20948_DMP_CPASS_MTX_02, 0x00000000U}, {K_ICM20948_DMP_CPASS_MTX_10, 0x00000000U},
        {K_ICM20948_DMP_CPASS_MTX_11, 0xF6666667U}, {K_ICM20948_DMP_CPASS_MTX_12, 0x00000000U},
        {K_ICM20948_DMP_CPASS_MTX_20, 0x00000000U}, {K_ICM20948_DMP_CPASS_MTX_21, 0x00000000U},
        {K_ICM20948_DMP_CPASS_MTX_22, 0xF6666667U}};

    static const ICM20948_DmpMatrixEntry_t s_b2sMatrix[] = {
        {K_ICM20948_DMP_B2S_MTX_00, 0x40000000U}, {K_ICM20948_DMP_B2S_MTX_01, 0x00000000U},
        {K_ICM20948_DMP_B2S_MTX_02, 0x00000000U}, {K_ICM20948_DMP_B2S_MTX_10, 0x00000000U},
        {K_ICM20948_DMP_B2S_MTX_11, 0x40000000U}, {K_ICM20948_DMP_B2S_MTX_12, 0x00000000U},
        {K_ICM20948_DMP_B2S_MTX_20, 0x00000000U}, {K_ICM20948_DMP_B2S_MTX_21, 0x00000000U},
        {K_ICM20948_DMP_B2S_MTX_22, 0x40000000U}};

    if (p_spi == NULL)
    {
        return K_SPP_ERROR_NULL_POINTER;
    }

    ret = SPP_SERVICES_ICM20948_setBank(p_spi, K_ICM20948_REG_BANK_0);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_readReg(p_spi, K_ICM20948_REG_WHO_AM_I, &whoAmIValue);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    SPP_LOGI(K_ICM20948_LOG_TAG, "WHO_AM_I = 0x%02X (expect 0xEA)", whoAmIValue);

    if (whoAmIValue != K_ICM20948_WHO_AM_I_VALUE)
    {
        return K_SPP_ERROR;
    }

    {
        ICM20948_RegPwrMgmt1_t pwrMgmt1Reg = {.value = 0U};
        ICM20948_RegUserCtrl_t userCtrlReg = {.value = 0U};
        ICM20948_RegPwrMgmt2_t pwrMgmt2Reg = {.value = 0U};
        ICM20948_RegLpConf_t lpConfReg = {.value = 0U};

        /* Software reset — chip returns to POR state. */
        pwrMgmt1Reg.bits.deviceReset = 1U;
        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_PWR_MGMT_1, pwrMgmt1Reg.value);
        if (ret != K_SPP_OK)
        {
            return ret;
        }
        SPP_HAL_delayMs(50U);

        /* Wake up, auto clock, LP_EN=0 — required before DMP SRAM is writable. */
        pwrMgmt1Reg.value = 0U;
        pwrMgmt1Reg.bits.clkSel = K_ICM20948_CLK_AUTO;
        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_PWR_MGMT_1, pwrMgmt1Reg.value);
        if (ret != K_SPP_OK)
        {
            return ret;
        }

        userCtrlReg.bits.i2cIfDis = 1U;
        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_USER_CTRL, userCtrlReg.value);
        if (ret != K_SPP_OK)
        {
            return ret;
        }

        pwrMgmt2Reg.bits.disableAccelX = 1U;
        pwrMgmt2Reg.bits.disableAccelY = 1U;
        pwrMgmt2Reg.bits.disableAccelZ = 1U;
        pwrMgmt2Reg.bits.disableGyroZ = 1U;
        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_PWR_MGMT_2, pwrMgmt2Reg.value);
        if (ret != K_SPP_OK)
        {
            return ret;
        }

        lpConfReg.bits.i2cMstCyc = 1U;
        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_LP_CONF, lpConfReg.value);
        if (ret != K_SPP_OK)
        {
            return ret;
        }

        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_USER_CTRL, userCtrlReg.value);
        if (ret != K_SPP_OK)
        {
            return ret;
        }
    }

    SPP_LOGI(K_ICM20948_LOG_TAG, "Loading DMP firmware...");
    ret = SPP_SERVICES_ICM20948_loadDmp(p_data);
    if (ret != K_SPP_OK)
    {
        SPP_LOGE(K_ICM20948_LOG_TAG, "loadDmp failed ret=%d", (int)ret);
        return ret;
    }
    SPP_LOGI(K_ICM20948_LOG_TAG, "DMP loaded OK");

    ret = SPP_SERVICES_ICM20948_setBank(p_spi, K_ICM20948_REG_BANK_2);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_DMP_ADDR_MSB, K_ICM20948_DMP_START_ADDR_MSB);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_DMP_ADDR_LSB, K_ICM20948_DMP_START_ADDR_LSB);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_setBank(p_spi, K_ICM20948_REG_BANK_0);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = ICM20948_dmpWriteOutputConfig(p_data, 0x0000U, 0x0000U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_dmpWrite16(p_data, K_ICM20948_DMP_DATA_RDY_STATUS, 0x0000U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_dmpWrite16(p_data, K_ICM20948_DMP_FIFO_WATERMARK, 800U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    {
        ICM20948_RegIntEnable_t intEnableReg = {.value = 0U};
        ICM20948_RegIntEnable2_t intEnable2Reg = {.value = 0U};
        ICM20948_RegSingleFifoPrioritySel_t fifoPriorityReg = {.value = 0U};
        ICM20948_RegHwFixDisable_t hwFixDisableReg = {.value = 0U};

        intEnableReg.bits.rawData0Rdy = 1U;
        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_INT_ENABLE, intEnableReg.value);
        if (ret != K_SPP_OK)
        {
            return ret;
        }

        intEnable2Reg.bits.fifoOverflowEn0 = 1U;
        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_INT_ENABLE_2, intEnable2Reg.value);
        if (ret != K_SPP_OK)
        {
            return ret;
        }

        fifoPriorityReg.value = 0xE4U;
        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_SINGLE_FIFO_PRIORITY_SEL,
                                fifoPriorityReg.value);
        if (ret != K_SPP_OK)
        {
            return ret;
        }

        hwFixDisableReg.value = 0x48U;
        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_HW_FIX_DISABLE, hwFixDisableReg.value);
        if (ret != K_SPP_OK)
        {
            return ret;
        }
    }

    ret = SPP_SERVICES_ICM20948_setBank(p_spi, K_ICM20948_REG_BANK_2);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_GYRO_SMPLRT_DIV, 0x13U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_ACCEL_SMPLRT_DIV_1, 0x00U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_ACCEL_SMPLRT_DIV_2, 0x13U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_setBank(p_spi, K_ICM20948_REG_BANK_0);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_dmpWrite16(p_data, K_ICM20948_DMP_BAC_RATE, 0x0000U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_dmpWrite16(p_data, K_ICM20948_DMP_B2S_RATE, 0x0000U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    {
        ICM20948_RegFifoCfg_t fifoCfgReg = {.value = 0U};
        ICM20948_RegFifoEn1_t fifoEn1Reg = {.value = 0U};
        ICM20948_RegFifoEn2_t fifoEn2Reg = {.value = 0U};

        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_FIFO_CFG, fifoCfgReg.value);
        if (ret != K_SPP_OK)
        {
            return ret;
        }

        ret = SPP_SERVICES_ICM20948_resetFifo(p_spi);
        if (ret != K_SPP_OK)
        {
            return ret;
        }

        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_FIFO_EN_1, fifoEn1Reg.value);
        if (ret != K_SPP_OK)
        {
            return ret;
        }

        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_FIFO_EN_2, fifoEn2Reg.value);
        if (ret != K_SPP_OK)
        {
            return ret;
        }
    }

    {
        ICM20948_RegPwrMgmt1_t pwrMgmt1Reg = {.value = 0U};
        ICM20948_RegPwrMgmt2_t pwrMgmt2Reg = {.value = 0U};

        pwrMgmt1Reg.bits.clkSel = K_ICM20948_CLK_AUTO;
        pwrMgmt1Reg.bits.lpEn = 1U;
        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_PWR_MGMT_1, pwrMgmt1Reg.value);
        if (ret != K_SPP_OK)
        {
            return ret;
        }

        pwrMgmt2Reg.value = 0x7FU;
        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_PWR_MGMT_2, pwrMgmt2Reg.value);
        if (ret != K_SPP_OK)
        {
            return ret;
        }

        pwrMgmt1Reg.bits.sleep = 1U;
        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_PWR_MGMT_1, pwrMgmt1Reg.value);
        if (ret != K_SPP_OK)
        {
            return ret;
        }

        SPP_HAL_delayMs(1U);

        pwrMgmt1Reg.value = 0U;
        pwrMgmt1Reg.bits.clkSel = K_ICM20948_CLK_AUTO;
        pwrMgmt1Reg.bits.lpEn = 1U;
        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_PWR_MGMT_1, pwrMgmt1Reg.value);
        if (ret != K_SPP_OK)
        {
            return ret;
        }

        SPP_HAL_delayMs(1U);

        pwrMgmt1Reg.value = 0U;
        pwrMgmt1Reg.bits.clkSel = K_ICM20948_CLK_AUTO;
        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_PWR_MGMT_1, pwrMgmt1Reg.value);
        if (ret != K_SPP_OK)
        {
            return ret;
        }

        SPP_HAL_delayMs(1U);
    }

    /* AK09916 magnetometer: soft-reset */
    ret = SPP_SERVICES_ICM20948_setBank(p_spi, K_ICM20948_REG_BANK_3);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    SPP_LOGI(K_ICM20948_LOG_TAG, "Phase: mag soft-reset");
    ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_SLV0_CTRL, 0x00U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_I2C_SLV1_CTRL, 0x00U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_writeReg(p_spi, 0x0DU, 0x00U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_writeReg(p_spi, 0x11U, 0x00U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_I2C_CTRL, 0x17U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_I2C_MST_ODR_CONFIG, 0x04U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_I2C_SLV1_ADDR, K_ICM20948_MAG_WR_ADDR);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_I2C_SLV1_REG, 0x32U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_I2C_SLV1_DO, 0x01U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_I2C_SLV1_CTRL, 0x81U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_setBank(p_spi, K_ICM20948_REG_BANK_0);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    {
        ICM20948_RegUserCtrl_t userCtrlReg = {.value = 0U};

        userCtrlReg.bits.i2cIfDis = 1U;
        userCtrlReg.bits.i2cMstEn = 1U;
        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_USER_CTRL, userCtrlReg.value);
        if (ret != K_SPP_OK)
        {
            return ret;
        }

        SPP_HAL_delayMs(100U);

        userCtrlReg.value = 0U;
        userCtrlReg.bits.i2cIfDis = 1U;
        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_USER_CTRL, userCtrlReg.value);
        if (ret != K_SPP_OK)
        {
            return ret;
        }
    }

    ret = SPP_SERVICES_ICM20948_setBank(p_spi, K_ICM20948_REG_BANK_3);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_SLV0_CTRL, 0x00U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_I2C_SLV1_CTRL, 0x00U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_setBank(p_spi, K_ICM20948_REG_BANK_0);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    {
        ICM20948_RegUserCtrl_t userCtrlReg = {.value = 0U};

        userCtrlReg.bits.i2cIfDis = 1U;
        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_USER_CTRL, userCtrlReg.value);
        if (ret != K_SPP_OK)
        {
            return ret;
        }
    }

    SPP_LOGI(K_ICM20948_LOG_TAG, "Phase: cpass+b2s matrix");
    for (spp_uint8_t i = 0U; i < (sizeof(s_cpassMatrix) / sizeof(s_cpassMatrix[0])); i++)
    {
        ret = SPP_SERVICES_ICM20948_dmpWrite32(p_data, s_cpassMatrix[i].addr, s_cpassMatrix[i].val);
        if (ret != K_SPP_OK)
        {
            return ret;
        }

        ret = SPP_SERVICES_ICM20948_lpWakeCycle(p_data);
        if (ret != K_SPP_OK)
        {
            return ret;
        }
    }

    SPP_HAL_delayMs(1U);

    for (spp_uint8_t i = 0U; i < (sizeof(s_b2sMatrix) / sizeof(s_b2sMatrix[0])); i++)
    {
        ret = SPP_SERVICES_ICM20948_dmpWrite32(p_data, s_b2sMatrix[i].addr, s_b2sMatrix[i].val);
        if (ret != K_SPP_OK)
        {
            return ret;
        }

        ret = SPP_SERVICES_ICM20948_lpWakeCycle(p_data);
        if (ret != K_SPP_OK)
        {
            return ret;
        }
    }

    SPP_HAL_delayMs(1U);

    ret = SPP_SERVICES_ICM20948_setBank(p_spi, K_ICM20948_REG_BANK_2);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    {
        ICM20948_RegAccelConfig_t accelConfigReg = {.value = 0U};
        ICM20948_RegAccelConfig2_t accelConfig2Reg = {.value = 0U};

        accelConfigReg.bits.accelFsSel = K_ICM20948_ACCEL_FS_4G;
        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_ACCEL_CONFIG, accelConfigReg.value);
        if (ret != K_SPP_OK)
        {
            return ret;
        }

        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_ACCEL_CONFIG_2, accelConfig2Reg.value);
        if (ret != K_SPP_OK)
        {
            return ret;
        }
    }

    ret = SPP_SERVICES_ICM20948_setBank(p_spi, K_ICM20948_REG_BANK_0);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_lpWakeCycle(p_data);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_dmpWrite32(p_data, K_ICM20948_DMP_ACC_SCALE, 0x04000000U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_dmpWrite32(p_data, K_ICM20948_DMP_ACC_SCALE2, 0x00040000U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_lpWakeCycle(p_data);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_setBank(p_spi, K_ICM20948_REG_BANK_2);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    {
        ICM20948_RegGyroConfig_t gyroConfigReg = {.value = 0U};

        gyroConfigReg.bits.gyroFchoice = 1U;
        gyroConfigReg.bits.gyroFsSel = K_ICM20948_GYRO_FS_2000DPS;
        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_GYRO_CONFIG, gyroConfigReg.value);
        if (ret != K_SPP_OK)
        {
            return ret;
        }

        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, 0x02U, 0x00U);
        if (ret != K_SPP_OK)
        {
            return ret;
        }
    }

    ret = SPP_SERVICES_ICM20948_setBank(p_spi, K_ICM20948_REG_BANK_0);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_lpWakeCycle(p_data);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_dmpWrite32(p_data, K_ICM20948_DMP_GYRO_FULLSCALE, 0x10000000U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_lpWakeCycle(p_data);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_setBank(p_spi, K_ICM20948_REG_BANK_1);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_readReg(p_spi, K_ICM20948_REG_TIMEBASE_CORRECTION_PLL, &pllRaw);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    pllTrim = (spp_int8_t)pllRaw;

    ret = SPP_SERVICES_ICM20948_setBank(p_spi, K_ICM20948_REG_BANK_0);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_lpWakeCycle(p_data);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_dmpWrite32(p_data, K_ICM20948_DMP_GYRO_SF,
                              (spp_uint32_t)SPP_SERVICES_ICM20948_calcGyroSf(pllTrim));
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_lpWakeCycle(p_data);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    {
        ICM20948_RegLpConf_t lpConfReg = {.value = 0U};
        ICM20948_RegPwrMgmt1_t pwrMgmt1Reg = {.value = 0U};

        lpConfReg.bits.i2cMstCyc = 1U;
        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_LP_CONF, lpConfReg.value);
        if (ret != K_SPP_OK)
        {
            return ret;
        }

        pwrMgmt1Reg.bits.clkSel = K_ICM20948_CLK_AUTO;
        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_PWR_MGMT_1, pwrMgmt1Reg.value);
        if (ret != K_SPP_OK)
        {
            return ret;
        }
    }

    SPP_LOGI(K_ICM20948_LOG_TAG, "Phase: 3-seq DMP enable loop");
    for (spp_uint8_t seq = 0U; seq < 3U; seq++)
    {
        ICM20948_RegUserCtrl_t userCtrlReg = {.value = 0U};
        ICM20948_RegPwrMgmt2_t pwrMgmt2Reg = {.value = 0U};
        ICM20948_RegPwrMgmt1_t pwrMgmt1Reg = {.value = 0U};

        if (seq == 0U)
        {
            userCtrlReg.bits.i2cIfDis = 1U;
        }
        else
        {
            /* 0xD0 = DMP_EN + FIFO_EN + I2C_IF_DIS; I2C master not yet enabled here. */
            userCtrlReg.bits.i2cIfDis = 1U;
            userCtrlReg.bits.fifoEn = 1U;
            userCtrlReg.bits.dmpEn = 1U;
        }

        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_USER_CTRL, userCtrlReg.value);
        if (ret != K_SPP_OK)
        {
            return ret;
        }

        pwrMgmt2Reg.value = 0x47U;
        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_PWR_MGMT_2, pwrMgmt2Reg.value);
        if (ret != K_SPP_OK)
        {
            return ret;
        }

        /* Second write: same 0xD0 value. */
        userCtrlReg.value = 0U;
        userCtrlReg.bits.i2cIfDis = 1U;
        userCtrlReg.bits.fifoEn = 1U;
        userCtrlReg.bits.dmpEn = 1U;
        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_USER_CTRL, userCtrlReg.value);
        if (ret != K_SPP_OK)
        {
            return ret;
        }

        ret = ICM20948_dmpWriteOutputConfig(p_data, 0x0000U, 0x0000U);
        if (ret != K_SPP_OK)
        {
            return ret;
        }

        if (seq < 2U)
        {
            pwrMgmt2Reg.value = 0x7FU;
            ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_PWR_MGMT_2, pwrMgmt2Reg.value);
            if (ret != K_SPP_OK)
            {
                return ret;
            }

            pwrMgmt1Reg.bits.clkSel = K_ICM20948_CLK_AUTO;
            pwrMgmt1Reg.bits.sleep = 1U;
            ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_PWR_MGMT_1, pwrMgmt1Reg.value);
            if (ret != K_SPP_OK)
            {
                return ret;
            }

            SPP_HAL_delayMs(1U);

            pwrMgmt1Reg.value = 0U;
            pwrMgmt1Reg.bits.clkSel = K_ICM20948_CLK_AUTO;
            ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_PWR_MGMT_1, pwrMgmt1Reg.value);
            if (ret != K_SPP_OK)
            {
                return ret;
            }

            ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_PWR_MGMT_2, pwrMgmt2Reg.value);
            if (ret != K_SPP_OK)
            {
                return ret;
            }

            ret = SPP_SERVICES_ICM20948_dmpWrite16(p_data, K_ICM20948_DMP_DATA_RDY_STATUS, 0x0000U);
            if (ret != K_SPP_OK)
            {
                return ret;
            }

            ret = SPP_SERVICES_ICM20948_lpWakeCycle(p_data);
            if (ret != K_SPP_OK)
            {
                return ret;
            }
        }
    }

    SPP_LOGI(K_ICM20948_LOG_TAG, "Phase: final output config + mag start");
    ret = ICM20948_dmpWriteOutputConfig(p_data, 0x8400U, 0x03C0U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_dmpWrite32(p_data, K_ICM20948_DMP_ACCEL_ONLY_GAIN, 0x00E8BA2EU);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_dmpWrite32(p_data, K_ICM20948_DMP_ACCEL_ALPHA_VAR, 0x3D27D27DU);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_dmpWrite32(p_data, K_ICM20948_DMP_ACCEL_A_VAR, 0x02D82D83U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_dmpWrite16(p_data, K_ICM20948_DMP_ACCEL_CAL_INIT, 0x0000U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_dmpWrite16(p_data, K_ICM20948_DMP_CPASS_TIME_BUFFER, 0x0045U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_setBank(p_spi, K_ICM20948_REG_BANK_2);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_ACCEL_SMPLRT_DIV_1, 0x00U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_ACCEL_SMPLRT_DIV_2, 0x04U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_GYRO_SMPLRT_DIV, 0x04U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_setBank(p_spi, K_ICM20948_REG_BANK_0);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_dmpWrite16(p_data, K_ICM20948_DMP_ODR_QUAT9, 0x0000U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_dmpWrite16(p_data, K_ICM20948_DMP_ODR_QUAT6, 0x0000U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_dmpWrite16(p_data, K_ICM20948_DMP_ODR_ACCEL, 0x0000U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_dmpWrite16(p_data, K_ICM20948_DMP_ODR_GYRO, 0x0000U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_dmpWrite16(p_data, K_ICM20948_DMP_ODR_CPASS, 0x0000U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_dmpWrite32(p_data, K_ICM20948_DMP_GYRO_SF,
                              (spp_uint32_t)SPP_SERVICES_ICM20948_calcGyroSf(pllTrim));
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    {
        ICM20948_RegPwrMgmt2_t pwrMgmt2Reg = {.value = 0U};

        pwrMgmt2Reg.value = 0x40U;
        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_PWR_MGMT_2, pwrMgmt2Reg.value);
        if (ret != K_SPP_OK)
        {
            return ret;
        }
    }

    ret = SPP_SERVICES_ICM20948_dmpWrite16(p_data, K_ICM20948_DMP_DATA_RDY_STATUS, 0x000BU);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_setBank(p_spi, K_ICM20948_REG_BANK_3);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_SLV0_ADDR, K_ICM20948_MAG_RD_ADDR);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_SLV0_REG, 0x03U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_SLV0_CTRL, 0xDAU);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_I2C_SLV1_ADDR, K_ICM20948_MAG_WR_ADDR);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_I2C_SLV1_REG, K_ICM20948_MAG_CTRL_2);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_I2C_SLV1_DO, 0x01U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_I2C_SLV1_CTRL, 0x81U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_I2C_MST_ODR_CONFIG, 0x04U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_setBank(p_spi, K_ICM20948_REG_BANK_0);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    {
        ICM20948_RegUserCtrl_t userCtrlReg = {.value = 0U};

        userCtrlReg.bits.i2cIfDis = 1U;
        userCtrlReg.bits.i2cMstEn = 1U;
        userCtrlReg.bits.fifoEn = 1U;
        userCtrlReg.bits.dmpEn = 1U;
        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_USER_CTRL, userCtrlReg.value);
        if (ret != K_SPP_OK)
        {
            return ret;
        }

        userCtrlReg.bits.dmpRst = 1U;
        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_USER_CTRL, userCtrlReg.value);
        if (ret != K_SPP_OK)
        {
            return ret;
        }
    }

    SPP_HAL_delayMs(1U);

    ret = SPP_SERVICES_ICM20948_resetFifo(p_spi);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    {
        ICM20948_RegPwrMgmt1_t pwrMgmt1Reg = {.value = 0U};

        pwrMgmt1Reg.bits.clkSel = K_ICM20948_CLK_AUTO;
        pwrMgmt1Reg.bits.lpEn = 1U;
        ret = SPP_SERVICES_ICM20948_writeReg(p_spi, K_ICM20948_REG_PWR_MGMT_1, pwrMgmt1Reg.value);
        if (ret != K_SPP_OK)
        {
            return ret;
        }
    }

    ret = ICM20948_dmpWriteOutputConfig(p_data, 0x8400U, 0x03C0U);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_dmpWrite16(p_data, K_ICM20948_DMP_DATA_RDY_STATUS, 0x000BU);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_ICM20948_resetFifo(p_spi);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    return K_SPP_OK;
}

void SPP_SERVICES_ICM20948_checkFifoData(ICM20948_ServiceCtx_t *p_ctx)
{
    void *p_spi = p_ctx->p_spi;
    spp_uint8_t txRxData[3] = {0U};
    SPP_RetVal_t ret;

    if (p_spi == NULL)
    {
        return;
    }

    txRxData[0] = K_READ_OP | K_ICM20948_REG_INT_STATUS;
    txRxData[1] = K_WRITE_OP;
    txRxData[2] = K_WRITE_OP;

    ret = SPP_HAL_spiTransmit(p_spi, txRxData, 3U);
    if (ret != K_SPP_OK)
    {
        return;
    }

    {
        spp_uint8_t intStatus = txRxData[1];

        txRxData[0] = K_READ_OP | K_ICM20948_REG_DMP_INT_STATUS;
        txRxData[1] = K_WRITE_OP;
        txRxData[2] = K_WRITE_OP;

        ret = SPP_HAL_spiTransmit(p_spi, txRxData, 3U);
        if (ret != K_SPP_OK)
        {
            return;
        }

        if ((intStatus & 0x02U) != 0U)
        {
            txRxData[0] = K_READ_OP | K_ICM20948_REG_FIFO_COUNTH;
            txRxData[1] = K_WRITE_OP;
            txRxData[2] = K_WRITE_OP;

            ret = SPP_HAL_spiTransmit(p_spi, txRxData, 3U);
            if (ret != K_SPP_OK)
            {
                return;
            }

            {
                spp_uint16_t fifoCount = ((spp_uint16_t)txRxData[1] << 8) | txRxData[2];

                {
                    static spp_bool_t s_logged = false;
                    if (!s_logged)
                    {
                        SPP_LOGI(K_ICM20948_LOG_TAG, "FIFO first count=%u pktsz=%u",
                                 (unsigned)fifoCount, K_ICM20948_DMP_PACKET_SIZE_BYTES);
                        s_logged = true;
                    }
                }

                if (fifoCount > K_ICM20948_FIFO_RESET_THRESHOLD)
                {
                    (void)SPP_SERVICES_ICM20948_resetFifo(p_spi);
                    return;
                }

                {
                    spp_uint16_t numPackets = fifoCount / K_ICM20948_DMP_PACKET_SIZE_BYTES;

                    for (spp_uint16_t i = 0U; i < numPackets; i++)
                    {
                        /* fifoBuffer[0..23]: DMP packet (DATA_OUT_CTL1=0x8400).
                         * [0-1]=header1, [2-7]=accel, [8-21]=quat9, [22-23]=footer. */
                        spp_uint8_t fifoBuffer[K_ICM20948_DMP_PACKET_SIZE_BYTES] = {0U};

                        ret = readFifoBurst(p_spi, fifoBuffer, K_ICM20948_DMP_PACKET_SIZE_BYTES);
                        if (ret != K_SPP_OK)
                        {
                            return;
                        }

                        if (i == 0U)
                        {
                            static spp_bool_t s_hdrLogged = false;
                            if (!s_hdrLogged)
                            {
                                SPP_LOGI(K_ICM20948_LOG_TAG,
                                         "DMP hdr=%02X%02X accel=%02X%02X quat=%02X%02X",
                                         fifoBuffer[0], fifoBuffer[1],
                                         fifoBuffer[2], fifoBuffer[3],
                                         fifoBuffer[8], fifoBuffer[9]);
                                s_hdrLogged = true;
                            }
                        }

                        {
                            /* Packet layout (DATA_OUT_CTL1=0x8400, footer always present):
                             * [0:1]=header1, [2:7]=accel, [8:21]=quat9(Q1,Q2,Q3,acc),
                             * [22:23]=footer(gyro count, ignored). */
                            int16_t accelX =
                                (int16_t)(((uint16_t)fifoBuffer[2] << 8) | fifoBuffer[3]);
                            int16_t accelY =
                                (int16_t)(((uint16_t)fifoBuffer[4] << 8) | fifoBuffer[5]);
                            int16_t accelZ =
                                (int16_t)(((uint16_t)fifoBuffer[6] << 8) | fifoBuffer[7]);

                            int32_t q1Raw =
                                ((int32_t)fifoBuffer[8]  << 24) | ((int32_t)fifoBuffer[9]  << 16) |
                                ((int32_t)fifoBuffer[10] << 8)  |  (int32_t)fifoBuffer[11];
                            int32_t q2Raw =
                                ((int32_t)fifoBuffer[12] << 24) | ((int32_t)fifoBuffer[13] << 16) |
                                ((int32_t)fifoBuffer[14] << 8)  |  (int32_t)fifoBuffer[15];
                            int32_t q3Raw =
                                ((int32_t)fifoBuffer[16] << 24) | ((int32_t)fifoBuffer[17] << 16) |
                                ((int32_t)fifoBuffer[18] << 8)  |  (int32_t)fifoBuffer[19];
                            int16_t accuracy =
                                (int16_t)(((uint16_t)fifoBuffer[20] << 8) | fifoBuffer[21]);

                            float ax = accelX / 8192.0f;
                            float ay = accelY / 8192.0f;
                            float az = accelZ / 8192.0f;
                            float qx = q1Raw / 1073741824.0f;
                            float qy = q2Raw / 1073741824.0f;
                            float qz = q3Raw / 1073741824.0f;
                            float qwSq = 1.0f - (qx * qx) - (qy * qy) - (qz * qz);
                            float qw = (qwSq > 0.0f) ? sqrtf(qwSq) : 0.0f;

                            SPP_LOGI(K_ICM20948_LOG_TAG,
                                     "A:[%.2f %.2f %.2f]g Q:[w=%.4f x=%.4f y=%.4f z=%.4f] acc:%d",
                                     ax, ay, az, qw, qx, qy, qz, accuracy);

                            p_ctx->lastData.ax        = ax;
                            p_ctx->lastData.ay        = ay;
                            p_ctx->lastData.az        = az;
                            p_ctx->lastData.gx        = 0.0f;
                            p_ctx->lastData.gy        = 0.0f;
                            p_ctx->lastData.gz        = 0.0f;
                            p_ctx->lastData.mx        = 0.0f;
                            p_ctx->lastData.my        = 0.0f;
                            p_ctx->lastData.mz        = 0.0f;
                            p_ctx->lastData.dataReady = true;
                        }
                    }
                }
            }
        }
    }
}

/* ----------------------------------------------------------------
 * Interrupt initialisation
 * ---------------------------------------------------------------- */

void SPP_SERVICES_ICM20948_init(ICM20948_Data_t *p_icm)
{
    p_icm->drdyFlag       = false;
    p_icm->isr_ctx.p_flag = &p_icm->drdyFlag;
    SPP_HAL_gpioConfigInterrupt(p_icm->intPin, p_icm->intIntrType, p_icm->intPull);
    SPP_HAL_gpioRegisterIsr(p_icm->intPin, (void *)&p_icm->isr_ctx);
}

/* ----------------------------------------------------------------
 * Service task (call from superloop when drdyFlag is set)
 * ---------------------------------------------------------------- */

void SPP_SERVICES_ICM20948_serviceTask(void *p_arg)
{
    ICM20948_ServiceCtx_t *ctx = (ICM20948_ServiceCtx_t *)p_arg;

    if (!ctx->icmData.drdyFlag) return;
    ctx->icmData.drdyFlag      = false;
    ctx->lastData.dataReady    = false;

    SPP_SERVICES_ICM20948_checkFifoData(ctx);

    if (!ctx->lastData.dataReady)
    {
        return;
    }

    SPP_Packet_t *p_pkt = SPP_SERVICES_DATABANK_getPacket();
    if (p_pkt == NULL)
    {
        SPP_LOGI(K_ICM20948_LOG_TAG, "No free packet");
        return;
    }

    float payload[9] = {
        ctx->lastData.ax, ctx->lastData.ay, ctx->lastData.az,
        ctx->lastData.gx, ctx->lastData.gy, ctx->lastData.gz,
        ctx->lastData.mx, ctx->lastData.my, ctx->lastData.mz,
    };

    SPP_RetVal_t ret = SPP_SERVICES_DATABANK_packetData(p_pkt, K_ICM20948_SERVICE_APID,
                                                ctx->seq++, payload,
                                                (spp_uint16_t)sizeof(payload));
    if (ret != K_SPP_OK)
    {
        SPP_LOGE(K_ICM20948_LOG_TAG, "packetData failed ret=%d", (int)ret);
        (void)SPP_SERVICES_DATABANK_returnPacket(p_pkt);
        return;
    }

    (void)SPP_SERVICES_PUBSUB_publish(p_pkt);
}

/* Stub implementations for functions declared in the header but not yet
 * fully implemented. These will be completed in a future iteration. */

SPP_RetVal_t SPP_SERVICES_ICM20948_config(void *p_data)
{
    (void)p_data;
    return K_SPP_OK;
}

SPP_RetVal_t SPP_SERVICES_ICM20948_configDmp(void *p_data)
{
    (void)p_data;
    return K_SPP_OK;
}

SPP_RetVal_t SPP_SERVICES_ICM20948_dmpStart(void *p_data)
{
    (void)p_data;
    return K_SPP_OK;
}

SPP_RetVal_t SPP_SERVICES_ICM20948_readSensors(void *p_data)
{
    (void)p_data;
    return K_SPP_OK;
}

void SPP_SERVICES_ICM20948_getSensorsData(void *p_data)
{
    (void)p_data;
}

/* ----------------------------------------------------------------
 * Service callbacks
 * ---------------------------------------------------------------- */

static SPP_RetVal_t SPP_SERVICES_ICM20948_serviceInit(void *p_ctx, const void *p_cfg)
{
    ICM20948_ServiceCtx_t *ctx = p_ctx;
    const ICM20948_ServiceCfg_t *cfg = p_cfg;

    ctx->p_spi = SPP_HAL_spiGetHandle(cfg->spiDevIdx);
    ctx->seq   = 0U;

    ctx->icmData.intPin      = cfg->intPin;
    ctx->icmData.intIntrType = cfg->intIntrType;
    ctx->icmData.intPull     = cfg->intPull;

    SPP_SERVICES_ICM20948_init(&ctx->icmData);

    SPP_LOGI(K_ICM20948_LOG_TAG, "Service init (spiDevIdx=%u intPin=%u)",
             cfg->spiDevIdx, cfg->intPin);
    return K_SPP_OK;
}

static SPP_RetVal_t SPP_SERVICES_ICM20948_serviceStart(void *p_ctx)
{
    ICM20948_ServiceCtx_t *ctx = p_ctx;
    SPP_RetVal_t ret;

    SPP_LOGI(K_ICM20948_LOG_TAG, "Service start — running configDmpInit");

    ret = SPP_SERVICES_ICM20948_configDmpInit(ctx->p_spi);
    if (ret != K_SPP_OK)
    {
        SPP_LOGE(K_ICM20948_LOG_TAG, "configDmpInit failed ret=%d", (int)ret);
        return ret;
    }

    return K_SPP_OK;
}

static SPP_RetVal_t SPP_SERVICES_ICM20948_serviceStop(void *p_ctx)
{
    (void)p_ctx;
    return K_SPP_OK;
}

static SPP_RetVal_t SPP_SERVICES_ICM20948_serviceDeinit(void *p_ctx)
{
    (void)p_ctx;
    return K_SPP_OK;
}

/* ----------------------------------------------------------------
 * Service descriptor
 * ---------------------------------------------------------------- */

const SPP_Module_t g_icm20948Module = {
    .p_name       = "icm20948",
    .apid         = K_ICM20948_SERVICE_APID,
    .ctxSize      = sizeof(ICM20948_ServiceCtx_t),
    .init         = SPP_SERVICES_ICM20948_serviceInit,
    .start        = SPP_SERVICES_ICM20948_serviceStart,
    .stop         = SPP_SERVICES_ICM20948_serviceStop,
    .deinit       = SPP_SERVICES_ICM20948_serviceDeinit,
    .serviceTask  = SPP_SERVICES_ICM20948_serviceTask,
    .consumesApid = K_SPP_APID_NONE,
    .onPacket     = NULL,
    .onPacketPrio = 0U,
};
