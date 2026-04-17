/**
 * @file icm20948_service.h
 * @brief ICM20948 IMU driver and SPP service.
 *
 * Provides SPI register access, DMP firmware loading and sensor data
 * acquisition for the TDK InvenSense ICM20948 IMU.
 *
 * To use the SPP service registry, allocate a @ref ICM20948_ServiceCtx_t and
 * a @ref ICM20948_ServiceCfg_t, then call SPP_Service_register() with
 * @ref g_icm20948ServiceDesc.
 *
 * Naming conventions used in this file:
 * - Constants/macros:  K_ICM20948_*
 * - Types:             ICM20948_*_t
 * - Public functions:  ICM20948_*()
 * - Pointer params:    p_*
 */

#ifndef SPP_ICM20948_SERVICE_H
#define SPP_ICM20948_SERVICE_H

#include "spp/core/returntypes.h"
#include "spp/core/types.h"
#include "spp/hal/gpio.h"
#include "spp/services/service.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ----------------------------------------------------------------
 * Task priorities
 * ---------------------------------------------------------------- */

#define K_ICM20948_TASK_PRIORITY          4U
#define K_ICM20948_CONFIG_TASK_PRIORITY   5U
#define K_ICM20948_READ_SENSORS_PRIORITY  4U

/* ----------------------------------------------------------------
 * Hardware pins
 * ---------------------------------------------------------------- */

/** @brief SPI host identifier (SPI2_HOST = 1 on ESP32). */
#define K_ICM20948_SPI_HOST_USED   (1U)

#define K_ICM20948_PIN_NUM_CS   21U
#define K_ICM20948_PIN_NUM_CIPO 47U
#define K_ICM20948_PIN_NUM_COPI 38U
#define K_ICM20948_PIN_NUM_CLK  48U

/* ----------------------------------------------------------------
 * SPI operation type
 * ---------------------------------------------------------------- */

#define K_ICM20948_READ_OP        0x80U
#define K_ICM20948_WRITE_OP       0x00U
#define K_ICM20948_EMPTY_MESSAGE  0x00U

/* ----------------------------------------------------------------
 * Register banks
 * ---------------------------------------------------------------- */

typedef enum
{
    K_ICM20948_REG_BANK_0 = 0x00U,
    K_ICM20948_REG_BANK_1 = 0x10U,
    K_ICM20948_REG_BANK_2 = 0x20U,
    K_ICM20948_REG_BANK_3 = 0x30U
} ICM20948_RegBank_t;

#define K_ICM20948_REG_BANK_SEL 0x7FU

/* ----------------------------------------------------------------
 * Bank 0 register addresses
 * ---------------------------------------------------------------- */

#define K_ICM20948_REG_WHO_AM_I                 0x00U
#define K_ICM20948_REG_USER_CTRL                0x03U
#define K_ICM20948_REG_LP_CONF                  0x05U
#define K_ICM20948_REG_PWR_MGMT_1               0x06U
#define K_ICM20948_REG_PWR_MGMT_2               0x07U
#define K_ICM20948_REG_INT_ENABLE               0x10U
#define K_ICM20948_REG_INT_ENABLE_1             0x11U
#define K_ICM20948_REG_INT_ENABLE_2             0x12U
#define K_ICM20948_REG_DMP_INT_STATUS           0x18U
#define K_ICM20948_REG_INT_STATUS               0x19U
#define K_ICM20948_REG_SINGLE_FIFO_PRIORITY_SEL 0x26U
#define K_ICM20948_REG_PERIPH_SENS_DATA_00      0x3BU
#define K_ICM20948_REG_FIFO_EN_1                0x66U
#define K_ICM20948_REG_FIFO_EN_2                0x67U
#define K_ICM20948_REG_FIFO_RST                 0x68U
#define K_ICM20948_REG_FIFO_MODE                0x69U
#define K_ICM20948_REG_FIFO_COUNTH              0x70U
#define K_ICM20948_REG_FIFO_COUNTL              0x71U
#define K_ICM20948_REG_FIFO_R_W                 0x72U
#define K_ICM20948_REG_HW_FIX_DISABLE           0x75U
#define K_ICM20948_REG_FIFO_CFG                 0x76U

/* ----------------------------------------------------------------
 * Bank 1 register addresses
 * ---------------------------------------------------------------- */

#define K_ICM20948_REG_TIMEBASE_CORRECTION_PLL 0x28U

/* ----------------------------------------------------------------
 * Bank 2 register addresses
 * ---------------------------------------------------------------- */

#define K_ICM20948_REG_GYRO_SMPLRT_DIV    0x00U
#define K_ICM20948_REG_GYRO_CONFIG        0x01U
#define K_ICM20948_REG_ACCEL_SMPLRT_DIV_1 0x10U
#define K_ICM20948_REG_ACCEL_SMPLRT_DIV_2 0x11U
#define K_ICM20948_REG_ACCEL_CONFIG       0x14U
#define K_ICM20948_REG_ACCEL_CONFIG_2     0x15U
#define K_ICM20948_REG_DMP_ADDR_MSB       0x50U
#define K_ICM20948_REG_DMP_ADDR_LSB       0x51U

/* ----------------------------------------------------------------
 * Bank 3 register addresses
 * ---------------------------------------------------------------- */

#define K_ICM20948_I2C_MST_ODR_CONFIG 0x00U
#define K_ICM20948_REG_I2C_CTRL       0x01U
#define K_ICM20948_REG_SLV0_ADDR      0x03U
#define K_ICM20948_REG_SLV0_REG       0x04U
#define K_ICM20948_REG_SLV0_CTRL      0x05U
#define K_ICM20948_REG_SLV0_DO        0x06U
#define K_ICM20948_I2C_SLV1_ADDR      0x07U
#define K_ICM20948_I2C_SLV1_REG       0x08U
#define K_ICM20948_I2C_SLV1_CTRL      0x09U
#define K_ICM20948_I2C_SLV1_DO        0x0AU

/* ----------------------------------------------------------------
 * DMP memory access
 * ---------------------------------------------------------------- */

#define K_ICM20948_REG_MEM_START_ADDR 0x7CU
#define K_ICM20948_REG_MEM_R_W        0x7DU
#define K_ICM20948_REG_MEM_BANK_SEL   0x7EU
#define K_ICM20948_DMP_LOAD_START     0x0090U

/* ----------------------------------------------------------------
 * DMP memory addresses
 * ---------------------------------------------------------------- */

#define K_ICM20948_DMP_DATA_OUT_CTL1     (4U * 16U)
#define K_ICM20948_DMP_DATA_OUT_CTL2     (4U * 16U + 2U)
#define K_ICM20948_DMP_DATA_INTR_CTL     (4U * 16U + 12U)
#define K_ICM20948_DMP_MOTION_EVENT_CTL  (4U * 16U + 14U)
#define K_ICM20948_DMP_DATA_RDY_STATUS   (8U * 16U + 10U)
#define K_ICM20948_DMP_ODR_QUAT6         (10U * 16U + 12U)
#define K_ICM20948_DMP_ODR_QUAT9         (10U * 16U + 8U)
#define K_ICM20948_DMP_ODR_GYRO          (11U * 16U + 10U)
#define K_ICM20948_DMP_ODR_CPASS         (11U * 16U + 6U)
#define K_ICM20948_DMP_ODR_ACCEL         (11U * 16U + 14U)
#define K_ICM20948_DMP_CPASS_TIME_BUFFER (29U * 16U + 8U)
#define K_ICM20948_DMP_GYRO_SF           (19U * 16U)
#define K_ICM20948_DMP_ACC_SCALE         (30U * 16U)
#define K_ICM20948_DMP_FIFO_WATERMARK    (31U * 16U + 14U)
#define K_ICM20948_DMP_BAC_RATE          (48U * 16U + 10U)
#define K_ICM20948_DMP_B2S_RATE          (48U * 16U + 8U)
#define K_ICM20948_DMP_ACCEL_ONLY_GAIN   (16U * 16U + 12U)
#define K_ICM20948_DMP_ACC_SCALE2        (79U * 16U + 4U)
#define K_ICM20948_DMP_CPASS_MTX_00      (23U * 16U)
#define K_ICM20948_DMP_CPASS_MTX_01      (23U * 16U + 4U)
#define K_ICM20948_DMP_CPASS_MTX_02      (23U * 16U + 8U)
#define K_ICM20948_DMP_CPASS_MTX_10      (23U * 16U + 12U)
#define K_ICM20948_DMP_CPASS_MTX_11      (24U * 16U)
#define K_ICM20948_DMP_CPASS_MTX_12      (24U * 16U + 4U)
#define K_ICM20948_DMP_CPASS_MTX_20      (24U * 16U + 8U)
#define K_ICM20948_DMP_CPASS_MTX_21      (24U * 16U + 12U)
#define K_ICM20948_DMP_CPASS_MTX_22      (25U * 16U)
#define K_ICM20948_DMP_GYRO_FULLSCALE    (72U * 16U + 12U)
#define K_ICM20948_DMP_ACCEL_ALPHA_VAR   (91U * 16U)
#define K_ICM20948_DMP_ACCEL_A_VAR       (92U * 16U)
#define K_ICM20948_DMP_ACCEL_CAL_INIT    (94U * 16U + 4U)
#define K_ICM20948_DMP_B2S_MTX_00        (208U * 16U)
#define K_ICM20948_DMP_B2S_MTX_01        (208U * 16U + 4U)
#define K_ICM20948_DMP_B2S_MTX_02        (208U * 16U + 8U)
#define K_ICM20948_DMP_B2S_MTX_10        (208U * 16U + 12U)
#define K_ICM20948_DMP_B2S_MTX_11        (209U * 16U)
#define K_ICM20948_DMP_B2S_MTX_12        (209U * 16U + 4U)
#define K_ICM20948_DMP_B2S_MTX_20        (209U * 16U + 8U)
#define K_ICM20948_DMP_B2S_MTX_21        (209U * 16U + 12U)
#define K_ICM20948_DMP_B2S_MTX_22        (210U * 16U)

/* ----------------------------------------------------------------
 * Magnetometer (AK09916)
 * ---------------------------------------------------------------- */

#define K_ICM20948_MAG_WR_ADDR 0x0CU
#define K_ICM20948_MAG_RD_ADDR 0x8CU
#define K_ICM20948_MAG_CTRL_2  0x31U

#define K_ICM20948_REG_ACCEL_X_H   0x2DU
#define K_ICM20948_REG_ACCEL_X_L   0x2EU
#define K_ICM20948_REG_ACCEL_Y_H   0x2FU
#define K_ICM20948_REG_ACCEL_Y_L   0x30U
#define K_ICM20948_REG_ACCEL_Z_H   0x31U
#define K_ICM20948_REG_ACCEL_Z_L   0x32U
#define K_ICM20948_REG_GYRO_X_H    0x33U
#define K_ICM20948_REG_GYRO_X_L    0x34U
#define K_ICM20948_REG_GYRO_Y_H    0x35U
#define K_ICM20948_REG_GYRO_Y_L    0x36U
#define K_ICM20948_REG_GYRO_Z_H    0x37U
#define K_ICM20948_REG_GYRO_Z_L    0x38U
#define K_ICM20948_REG_MAGNETO_X_L 0x3CU
#define K_ICM20948_REG_MAGNETO_X_H 0x3DU
#define K_ICM20948_REG_MAGNETO_Y_L 0x3EU
#define K_ICM20948_REG_MAGNETO_Y_H 0x3FU
#define K_ICM20948_REG_MAGNETO_Z_L 0x40U
#define K_ICM20948_REG_MAGNETO_Z_H 0x41U

/* ----------------------------------------------------------------
 * Register unions
 * ---------------------------------------------------------------- */

typedef union
{
    spp_uint8_t value;
    struct { spp_uint8_t reserved0 : 4; spp_uint8_t bankSel : 2; spp_uint8_t reserved1 : 2; } bits;
} ICM20948_RegBankSel_t;

typedef union
{
    spp_uint8_t value;
    struct {
        spp_uint8_t reserved0 : 1; spp_uint8_t i2cMstRst : 1; spp_uint8_t sramRst : 1;
        spp_uint8_t dmpRst : 1; spp_uint8_t i2cIfDis : 1; spp_uint8_t i2cMstEn : 1;
        spp_uint8_t fifoEn : 1; spp_uint8_t dmpEn : 1;
    } bits;
} ICM20948_RegUserCtrl_t;

typedef union
{
    spp_uint8_t value;
    struct {
        spp_uint8_t reserved0 : 4; spp_uint8_t gyroCyc : 1; spp_uint8_t accelCyc : 1;
        spp_uint8_t i2cMstCyc : 1; spp_uint8_t reserved1 : 1;
    } bits;
} ICM20948_RegLpConf_t;

typedef enum {
    K_ICM20948_CLK_INTERNAL_20MHZ = 0U,
    K_ICM20948_CLK_AUTO = 1U,
    K_ICM20948_CLK_STOP = 7U
} ICM20948_ClockSel_t;

typedef union
{
    spp_uint8_t value;
    struct {
        spp_uint8_t clkSel : 3; spp_uint8_t tempDis : 1; spp_uint8_t reserved0 : 1;
        spp_uint8_t lpEn : 1; spp_uint8_t sleep : 1; spp_uint8_t deviceReset : 1;
    } bits;
} ICM20948_RegPwrMgmt1_t;

typedef union
{
    spp_uint8_t value;
    struct {
        spp_uint8_t disableAccelX : 1; spp_uint8_t disableAccelY : 1; spp_uint8_t disableAccelZ : 1;
        spp_uint8_t disableGyroX : 1; spp_uint8_t disableGyroY : 1; spp_uint8_t disableGyroZ : 1;
        spp_uint8_t reserved0 : 2;
    } bits;
} ICM20948_RegPwrMgmt2_t;

typedef union
{
    spp_uint8_t value;
    struct { spp_uint8_t reserved0 : 1; spp_uint8_t rawData0Rdy : 1; spp_uint8_t reserved1 : 6; } bits;
} ICM20948_RegIntEnable_t;

typedef union
{
    spp_uint8_t value;
    struct {
        spp_uint8_t fifoOverflowEn0 : 1; spp_uint8_t fifoOverflowEn1 : 1;
        spp_uint8_t fifoOverflowEn2 : 1; spp_uint8_t fifoOverflowEn3 : 1;
        spp_uint8_t reserved0 : 3; spp_uint8_t dmpInt1En : 1;
    } bits;
} ICM20948_RegIntEnable2_t;

typedef union
{
    spp_uint8_t value;
    struct { spp_uint8_t reserved0 : 1; spp_uint8_t rawData0Rdy : 1; spp_uint8_t reserved1 : 6; } bits;
} ICM20948_RegIntStatus_t;

typedef union
{
    spp_uint8_t value;
    struct {
        spp_uint8_t dmpInt0 : 1; spp_uint8_t dmpInt1 : 1; spp_uint8_t dmpInt2 : 1;
        spp_uint8_t dmpInt3 : 1; spp_uint8_t dmpInt4 : 1; spp_uint8_t reserved0 : 3;
    } bits;
} ICM20948_RegDmpIntStatus_t;

typedef union
{
    spp_uint8_t value;
    struct {
        spp_uint8_t accelFifoPri : 2; spp_uint8_t gyroFifoPri : 2;
        spp_uint8_t dmpFifoPri : 2; spp_uint8_t reserved0 : 2;
    } bits;
} ICM20948_RegSingleFifoPrioritySel_t;

typedef union
{
    spp_uint8_t value;
    struct {
        spp_uint8_t slv0FifoEn : 1; spp_uint8_t slv1FifoEn : 1;
        spp_uint8_t slv2FifoEn : 1; spp_uint8_t slv3FifoEn : 1; spp_uint8_t reserved0 : 4;
    } bits;
} ICM20948_RegFifoEn1_t;

typedef union
{
    spp_uint8_t value;
    struct {
        spp_uint8_t accelFifoEn : 1; spp_uint8_t gyroXFifoEn : 1; spp_uint8_t gyroYFifoEn : 1;
        spp_uint8_t gyroZFifoEn : 1; spp_uint8_t tempFifoEn : 1; spp_uint8_t reserved0 : 3;
    } bits;
} ICM20948_RegFifoEn2_t;

typedef union
{
    spp_uint8_t value;
    struct {
        spp_uint8_t fifoRst0 : 1; spp_uint8_t fifoRst1 : 1; spp_uint8_t fifoRst2 : 1;
        spp_uint8_t fifoRst3 : 1; spp_uint8_t fifoRst4 : 1; spp_uint8_t reserved0 : 3;
    } bits;
} ICM20948_RegFifoRst_t;

typedef union
{
    spp_uint8_t value;
    struct { spp_uint8_t fifoMode : 1; spp_uint8_t reserved0 : 7; } bits;
} ICM20948_RegFifoMode_t;

typedef union
{
    spp_uint8_t value;
    struct { spp_uint8_t fifoCfg : 1; spp_uint8_t reserved0 : 7; } bits;
} ICM20948_RegFifoCfg_t;

typedef union
{
    spp_uint8_t value;
    struct {
        spp_uint8_t bit0 : 1; spp_uint8_t bit1 : 1; spp_uint8_t bit2 : 1; spp_uint8_t bit3 : 1;
        spp_uint8_t bit4 : 1; spp_uint8_t bit5 : 1; spp_uint8_t bit6 : 1; spp_uint8_t bit7 : 1;
    } bits;
} ICM20948_RegHwFixDisable_t;

typedef enum {
    K_ICM20948_GYRO_FS_250DPS = 0U,
    K_ICM20948_GYRO_FS_500DPS = 1U,
    K_ICM20948_GYRO_FS_1000DPS = 2U,
    K_ICM20948_GYRO_FS_2000DPS = 3U
} ICM20948_GyroFsSel_t;

typedef union
{
    spp_uint8_t value;
    struct {
        spp_uint8_t gyroFchoice : 1; spp_uint8_t gyroFsSel : 2;
        spp_uint8_t gyroDlpfCfg : 3; spp_uint8_t reserved0 : 2;
    } bits;
} ICM20948_RegGyroConfig_t;

typedef enum {
    K_ICM20948_ACCEL_FS_2G = 0U,
    K_ICM20948_ACCEL_FS_4G = 1U,
    K_ICM20948_ACCEL_FS_8G = 2U,
    K_ICM20948_ACCEL_FS_16G = 3U
} ICM20948_AccelFsSel_t;

typedef union
{
    spp_uint8_t value;
    struct {
        spp_uint8_t accelFchoice : 1; spp_uint8_t accelFsSel : 2;
        spp_uint8_t accelDlpfCfg : 3; spp_uint8_t reserved0 : 2;
    } bits;
} ICM20948_RegAccelConfig_t;

typedef union
{
    spp_uint8_t value;
    struct { spp_uint8_t dec3Cfg : 2; spp_uint8_t reserved0 : 6; } bits;
} ICM20948_RegAccelConfig2_t;

typedef union
{
    spp_uint8_t value;
    struct {
        spp_uint8_t i2cMstClk : 4; spp_uint8_t i2cMstPnt : 1; spp_uint8_t reserved0 : 3;
    } bits;
} ICM20948_RegI2cCtrl_t;

typedef union
{
    spp_uint8_t value;
    struct { spp_uint8_t i2cId : 7; spp_uint8_t readNotWrite : 1; } bits;
} ICM20948_RegI2cSlvAddr_t;

typedef union
{
    spp_uint8_t value;
    struct {
        spp_uint8_t length : 4; spp_uint8_t group : 1; spp_uint8_t regDis : 1;
        spp_uint8_t byteSwap : 1; spp_uint8_t enable : 1;
    } bits;
} ICM20948_RegI2cSlvCtrl_t;

typedef union
{
    spp_uint8_t value;
    struct { spp_uint8_t i2cMstOdr : 4; spp_uint8_t reserved0 : 4; } bits;
} ICM20948_RegI2cMstOdrConfig_t;

/* ----------------------------------------------------------------
 * Service types
 * ---------------------------------------------------------------- */

/** @brief APID used by the ICM20948 service. */
#define K_ICM20948_SERVICE_APID (0x0201U)

/**
 * @brief Parsed sensor sample from the DMP FIFO.
 */
typedef struct
{
    float      ax;        /**< Accelerometer X (g)   */
    float      ay;        /**< Accelerometer Y (g)   */
    float      az;        /**< Accelerometer Z (g)   */
    float      gx;        /**< Gyroscope X (dps)     */
    float      gy;        /**< Gyroscope Y (dps)     */
    float      gz;        /**< Gyroscope Z (dps)     */
    float      mx;        /**< Magnetometer X (uT)   */
    float      my;        /**< Magnetometer Y (uT)   */
    float      mz;        /**< Magnetometer Z (uT)   */
    spp_bool_t dataReady; /**< Set when a FIFO packet has been parsed. */
} ICM20948_SensorData_t;

/**
 * @brief ICM20948 interrupt and GPIO context.
 */
typedef struct
{
    volatile spp_bool_t  drdyFlag;      /**< Set by ISR on data-ready interrupt. */
    SPP_GpioIsrCtx_t     isr_ctx;       /**< ISR context (points at drdyFlag).   */
    spp_uint32_t         intPin;        /**< GPIO interrupt pin number.           */
    spp_uint32_t         intIntrType;   /**< Interrupt trigger type (1=rising).   */
    spp_uint32_t         intPull;       /**< Pull resistor: 0=none, 1=up, 2=down. */
} ICM20948_Data_t;

/**
 * @brief Configuration for the ICM20948 service instance.
 */
typedef struct
{
    spp_uint8_t  spiDevIdx;   /**< SPI device handle index (ICM = 0). */
    spp_uint32_t intPin;      /**< GPIO interrupt pin number.          */
    spp_uint32_t intIntrType; /**< Interrupt edge type (1=rising).     */
    spp_uint32_t intPull;     /**< Pull resistor (0=none, 1=up).       */
} ICM20948_ServiceCfg_t;

/**
 * @brief Runtime context for the ICM20948 service instance.
 */
typedef struct
{
    void                  *p_spi;    /**< SPI device handle.              */
    ICM20948_Data_t        icmData;  /**< Interrupt flag and ISR context. */
    ICM20948_SensorData_t  lastData; /**< Last parsed FIFO sample.        */
    spp_uint16_t           seq;      /**< Packet sequence counter.        */
} ICM20948_ServiceCtx_t;

/**
 * @brief ICM20948 service descriptor — pass to SPP_Service_register().
 */
extern const SPP_ServiceDesc_t g_icm20948ServiceDesc;

/* ----------------------------------------------------------------
 * Public driver API
 * ---------------------------------------------------------------- */

SPP_RetVal_t ICM20948_config(void *p_data);
SPP_RetVal_t ICM20948_configDmp(void *p_data);
SPP_RetVal_t ICM20948_configDmpInit(void *p_data);
SPP_RetVal_t ICM20948_loadDmp(void *p_data);
SPP_RetVal_t ICM20948_dmpStart(void *p_data);
SPP_RetVal_t ICM20948_readSensors(void *p_data);
void     ICM20948_getSensorsData(void *p_data);
void     ICM20948_checkFifoData(ICM20948_ServiceCtx_t *p_ctx);

/**
 * @brief Initialise the ICM20948 interrupt context and register the GPIO ISR.
 *
 * @param[out] p_icm  Pointer to the ICM20948 device context to initialise.
 */
void ICM20948_init(ICM20948_Data_t *p_icm);

/**
 * @brief Read the DMP FIFO, package sensor data and publish via pub/sub.
 *
 * Call from the superloop when @c icmData.drdyFlag is set.
 *
 * @param[in] p_ctx  Pointer to the ICM20948 service context.
 */
void ICM20948_ServiceTask(void *p_ctx);

#ifdef __cplusplus
}
#endif

#endif /* SPP_ICM20948_SERVICE_H */
