/**
 * @file icm20948.h
 * @brief ICM20948 IMU driver and SPP service.
 *
 * Provides SPI register access, DMP firmware loading and sensor data
 * acquisition for the TDK InvenSense ICM20948 9-axis IMU.  Supports
 * data-ready interrupt via GPIO event groups.
 *
 * The driver follows a three-step workflow:
 * 1. Call SPP_SERVICES_ICM20948_getProducerContract() and register the result
 *    with SPP_SERVICES_PUBSUB_registerProducer().
 * 2. The service framework calls init (GPIO/SPI setup + DMP firmware load).
 * 3. On each DRDY interrupt acquireData reads the DMP FIFO and publishes
 *    a 9-axis data packet.
 *
 * Hardware pin configuration is encoded in K_ICM20948_INT_* and
 * K_ICM20948_SPI_HOST_USED constants; no external instance is needed.
 *
 * Naming conventions used in this file:
 * - Constants/macros:  K_ICM20948_*
 * - Types:             ICM20948_*_t
 * - Public functions:  SPP_SERVICES_ICM20948_*()
 * - Pointer params:    p_*
 */

#ifndef SPP_ICM20948_H
#define SPP_ICM20948_H

#include "spp/core/types.h"
#include "spp/hal/gpio/gpio.h"
#include "spp/services/service.h"

/* ----------------------------------------------------------------
 * DEFINES
 * ---------------------------------------------------------------- */

/* Service */
#define K_ICM20948_TASK_PRIORITY         4U        /**< FreeRTOS task priority for the ICM20948 service task. */
#define K_ICM20948_CONFIG_TASK_PRIORITY  5U        /**< Priority for the DMP configuration task. */
#define K_ICM20948_READ_SENSORS_PRIORITY 4U        /**< Priority for the sensor read task. */
#define K_ICM20948_TASK_TIMEOUT_MS       5000U     /**< Timeout in ms for the acquire task. */
#define K_ICM20948_SERVICE_APID          (0x0002U) /**< APID produced by the ICM20948 module (bitmask scheme). */

/* SPI */
#define K_ICM20948_SPI_HOST_USED (1U)  /**< SPI host identifier (SPI2_HOST = 1 on ESP32). */
#define K_ICM20948_PIN_NUM_CS    21U   /**< SPI chip-select GPIO pin. */
#define K_ICM20948_PIN_NUM_CIPO  47U   /**< SPI CIPO (MISO) GPIO pin. */
#define K_ICM20948_PIN_NUM_COPI  38U   /**< SPI COPI (MOSI) GPIO pin. */
#define K_ICM20948_PIN_NUM_CLK   48U   /**< SPI clock GPIO pin. */
#define K_ICM20948_READ_OP       0x80U /**< SPI read flag: set MSB to signal a read transaction. */
#define K_ICM20948_WRITE_OP      0x00U /**< SPI write marker (zero). */
#define K_ICM20948_EMPTY_MESSAGE 0x00U /**< Dummy byte clocked out during SPI reads. */

/* Interrupt */
#define K_ICM20948_INT_PIN_NUM   17U /**< GPIO interrupt pin. */
#define K_ICM20948_INT_INTR_TYPE 1U  /**< Interrupt trigger type (1=rising). */
#define K_ICM20948_INT_PULL      0U  /**< Pull resistor: 0=none, 1=up, 2=down. */

/* Register bank select */
#define K_ICM20948_REG_BANK_SEL 0x7FU /**< Register bank select register address. */

/* Bank 0 register addresses */
#define K_ICM20948_REG_WHO_AM_I                 0x00U /**< WHO_AM_I register; expected value 0xEA. */
#define K_ICM20948_REG_USER_CTRL                0x03U /**< User control register. */
#define K_ICM20948_REG_LP_CONF                  0x05U /**< Low-power configuration register. */
#define K_ICM20948_REG_PWR_MGMT_1               0x06U /**< Power management 1 register. */
#define K_ICM20948_REG_PWR_MGMT_2               0x07U /**< Power management 2 register. */
#define K_ICM20948_REG_INT_ENABLE               0x10U /**< Interrupt enable register. */
#define K_ICM20948_REG_INT_ENABLE_1             0x11U /**< Interrupt enable 1 register. */
#define K_ICM20948_REG_INT_ENABLE_2             0x12U /**< Interrupt enable 2 register. */
#define K_ICM20948_REG_DMP_INT_STATUS           0x18U /**< DMP interrupt status register. */
#define K_ICM20948_REG_INT_STATUS               0x19U /**< Interrupt status register. */
#define K_ICM20948_REG_SINGLE_FIFO_PRIORITY_SEL 0x26U /**< FIFO single-stream priority select register. */
#define K_ICM20948_REG_PERIPH_SENS_DATA_00      0x3BU /**< External sensor data register 0. */
#define K_ICM20948_REG_FIFO_EN_1                0x66U /**< FIFO enable 1 register. */
#define K_ICM20948_REG_FIFO_EN_2                0x67U /**< FIFO enable 2 register. */
#define K_ICM20948_REG_FIFO_RST                 0x68U /**< FIFO reset register. */
#define K_ICM20948_REG_FIFO_MODE                0x69U /**< FIFO mode register. */
#define K_ICM20948_REG_FIFO_COUNTH              0x70U /**< FIFO byte count high register. */
#define K_ICM20948_REG_FIFO_COUNTL              0x71U /**< FIFO byte count low register. */
#define K_ICM20948_REG_FIFO_R_W                 0x72U /**< FIFO read/write data register. */
#define K_ICM20948_REG_HW_FIX_DISABLE           0x75U /**< Hardware fix disable register. */
#define K_ICM20948_REG_FIFO_CFG                 0x76U /**< FIFO configuration register. */

/* Bank 1 register addresses */
#define K_ICM20948_REG_TIMEBASE_CORRECTION_PLL 0x28U /**< PLL timebase correction register (bank 1). */

/* Bank 2 register addresses */
#define K_ICM20948_REG_GYRO_SMPLRT_DIV    0x00U /**< Gyroscope sample-rate divider register (bank 2). */
#define K_ICM20948_REG_GYRO_CONFIG        0x01U /**< Gyroscope configuration register (bank 2). */
#define K_ICM20948_REG_ACCEL_SMPLRT_DIV_1 0x10U /**< Accelerometer sample-rate divider high register (bank 2). */
#define K_ICM20948_REG_ACCEL_SMPLRT_DIV_2 0x11U /**< Accelerometer sample-rate divider low register (bank 2). */
#define K_ICM20948_REG_ACCEL_CONFIG       0x14U /**< Accelerometer configuration register (bank 2). */
#define K_ICM20948_REG_ACCEL_CONFIG_2     0x15U /**< Accelerometer configuration 2 register (bank 2). */
#define K_ICM20948_REG_DMP_ADDR_MSB       0x50U /**< DMP program start address MSB (bank 2). */
#define K_ICM20948_REG_DMP_ADDR_LSB       0x51U /**< DMP program start address LSB (bank 2). */

/* Bank 3 register addresses */
#define K_ICM20948_I2C_MST_ODR_CONFIG 0x00U /**< I2C master ODR configuration register (bank 3). */
#define K_ICM20948_REG_I2C_CTRL       0x01U /**< I2C master control register (bank 3). */
#define K_ICM20948_REG_SLV0_ADDR      0x03U /**< I2C slave 0 address register (bank 3). */
#define K_ICM20948_REG_SLV0_REG       0x04U /**< I2C slave 0 register address (bank 3). */
#define K_ICM20948_REG_SLV0_CTRL      0x05U /**< I2C slave 0 control register (bank 3). */
#define K_ICM20948_REG_SLV0_DO        0x06U /**< I2C slave 0 data out register (bank 3). */
#define K_ICM20948_I2C_SLV1_ADDR      0x07U /**< I2C slave 1 address register (bank 3). */
#define K_ICM20948_I2C_SLV1_REG       0x08U /**< I2C slave 1 register address (bank 3). */
#define K_ICM20948_I2C_SLV1_CTRL      0x09U /**< I2C slave 1 control register (bank 3). */
#define K_ICM20948_I2C_SLV1_DO        0x0AU /**< I2C slave 1 data out register (bank 3). */

/* DMP memory access */
#define K_ICM20948_REG_MEM_START_ADDR 0x7CU   /**< DMP memory start address register. */
#define K_ICM20948_REG_MEM_R_W        0x7DU   /**< DMP memory read/write register. */
#define K_ICM20948_REG_MEM_BANK_SEL   0x7EU   /**< DMP memory bank select register. */
#define K_ICM20948_DMP_LOAD_START     0x0090U /**< Start address in DMP memory for firmware load. */

/* DMP memory addresses */
#define K_ICM20948_DMP_DATA_OUT_CTL1     (4U * 16U)         /**< DMP output control 1 address. */
#define K_ICM20948_DMP_DATA_OUT_CTL2     (4U * 16U + 2U)    /**< DMP output control 2 address. */
#define K_ICM20948_DMP_DATA_INTR_CTL     (4U * 16U + 12U)   /**< DMP interrupt control address. */
#define K_ICM20948_DMP_MOTION_EVENT_CTL  (4U * 16U + 14U)   /**< DMP motion event control address. */
#define K_ICM20948_DMP_DATA_RDY_STATUS   (8U * 16U + 10U)   /**< DMP data ready status address. */
#define K_ICM20948_DMP_ODR_QUAT6         (10U * 16U + 12U)  /**< DMP quaternion6 output data rate address. */
#define K_ICM20948_DMP_ODR_QUAT9         (10U * 16U + 8U)   /**< DMP quaternion9 output data rate address. */
#define K_ICM20948_DMP_ODR_GYRO          (11U * 16U + 10U)  /**< DMP gyroscope output data rate address. */
#define K_ICM20948_DMP_ODR_CPASS         (11U * 16U + 6U)   /**< DMP compass output data rate address. */
#define K_ICM20948_DMP_ODR_ACCEL         (11U * 16U + 14U)  /**< DMP accelerometer output data rate address. */
#define K_ICM20948_DMP_CPASS_TIME_BUFFER (29U * 16U + 8U)   /**< DMP compass time buffer address. */
#define K_ICM20948_DMP_GYRO_SF           (19U * 16U)        /**< DMP gyroscope scale factor address. */
#define K_ICM20948_DMP_ACC_SCALE         (30U * 16U)        /**< DMP accelerometer scale address. */
#define K_ICM20948_DMP_FIFO_WATERMARK    (31U * 16U + 14U)  /**< DMP FIFO watermark address. */
#define K_ICM20948_DMP_BAC_RATE          (48U * 16U + 10U)  /**< DMP basic activity classifier rate address. */
#define K_ICM20948_DMP_B2S_RATE          (48U * 16U + 8U)   /**< DMP bring-to-see rate address. */
#define K_ICM20948_DMP_ACCEL_ONLY_GAIN   (16U * 16U + 12U)  /**< DMP accel-only mode gain address. */
#define K_ICM20948_DMP_ACC_SCALE2        (79U * 16U + 4U)   /**< DMP accelerometer scale 2 address. */
#define K_ICM20948_DMP_CPASS_MTX_00      (23U * 16U)        /**< Compass mount matrix [0][0]. */
#define K_ICM20948_DMP_CPASS_MTX_01      (23U * 16U + 4U)   /**< Compass mount matrix [0][1]. */
#define K_ICM20948_DMP_CPASS_MTX_02      (23U * 16U + 8U)   /**< Compass mount matrix [0][2]. */
#define K_ICM20948_DMP_CPASS_MTX_10      (23U * 16U + 12U)  /**< Compass mount matrix [1][0]. */
#define K_ICM20948_DMP_CPASS_MTX_11      (24U * 16U)        /**< Compass mount matrix [1][1]. */
#define K_ICM20948_DMP_CPASS_MTX_12      (24U * 16U + 4U)   /**< Compass mount matrix [1][2]. */
#define K_ICM20948_DMP_CPASS_MTX_20      (24U * 16U + 8U)   /**< Compass mount matrix [2][0]. */
#define K_ICM20948_DMP_CPASS_MTX_21      (24U * 16U + 12U)  /**< Compass mount matrix [2][1]. */
#define K_ICM20948_DMP_CPASS_MTX_22      (25U * 16U)        /**< Compass mount matrix [2][2]. */
#define K_ICM20948_DMP_GYRO_FULLSCALE    (72U * 16U + 12U)  /**< DMP gyroscope full-scale address. */
#define K_ICM20948_DMP_ACCEL_ALPHA_VAR   (91U * 16U)        /**< DMP accel alpha variance address. */
#define K_ICM20948_DMP_ACCEL_A_VAR       (92U * 16U)        /**< DMP accel A variance address. */
#define K_ICM20948_DMP_ACCEL_CAL_INIT    (94U * 16U + 4U)   /**< DMP accel calibration init address. */
#define K_ICM20948_DMP_B2S_MTX_00        (208U * 16U)       /**< Body-to-sensor matrix [0][0]. */
#define K_ICM20948_DMP_B2S_MTX_01        (208U * 16U + 4U)  /**< Body-to-sensor matrix [0][1]. */
#define K_ICM20948_DMP_B2S_MTX_02        (208U * 16U + 8U)  /**< Body-to-sensor matrix [0][2]. */
#define K_ICM20948_DMP_B2S_MTX_10        (208U * 16U + 12U) /**< Body-to-sensor matrix [1][0]. */
#define K_ICM20948_DMP_B2S_MTX_11        (209U * 16U)       /**< Body-to-sensor matrix [1][1]. */
#define K_ICM20948_DMP_B2S_MTX_12        (209U * 16U + 4U)  /**< Body-to-sensor matrix [1][2]. */
#define K_ICM20948_DMP_B2S_MTX_20        (209U * 16U + 8U)  /**< Body-to-sensor matrix [2][0]. */
#define K_ICM20948_DMP_B2S_MTX_21        (209U * 16U + 12U) /**< Body-to-sensor matrix [2][1]. */
#define K_ICM20948_DMP_B2S_MTX_22        (210U * 16U)       /**< Body-to-sensor matrix [2][2]. */

/* Magnetometer (AK09916) */
#define K_ICM20948_MAG_WR_ADDR 0x0CU /**< AK09916 magnetometer write address (I2C). */
#define K_ICM20948_MAG_RD_ADDR 0x8CU /**< AK09916 magnetometer read address (I2C). */
#define K_ICM20948_MAG_CTRL_2  0x31U /**< AK09916 control 2 register address. */

/* Raw sensor data registers (bank 0) */
#define K_ICM20948_REG_ACCEL_X_H   0x2DU /**< Accelerometer X high byte register. */
#define K_ICM20948_REG_ACCEL_X_L   0x2EU /**< Accelerometer X low byte register. */
#define K_ICM20948_REG_ACCEL_Y_H   0x2FU /**< Accelerometer Y high byte register. */
#define K_ICM20948_REG_ACCEL_Y_L   0x30U /**< Accelerometer Y low byte register. */
#define K_ICM20948_REG_ACCEL_Z_H   0x31U /**< Accelerometer Z high byte register. */
#define K_ICM20948_REG_ACCEL_Z_L   0x32U /**< Accelerometer Z low byte register. */
#define K_ICM20948_REG_GYRO_X_H    0x33U /**< Gyroscope X high byte register. */
#define K_ICM20948_REG_GYRO_X_L    0x34U /**< Gyroscope X low byte register. */
#define K_ICM20948_REG_GYRO_Y_H    0x35U /**< Gyroscope Y high byte register. */
#define K_ICM20948_REG_GYRO_Y_L    0x36U /**< Gyroscope Y low byte register. */
#define K_ICM20948_REG_GYRO_Z_H    0x37U /**< Gyroscope Z high byte register. */
#define K_ICM20948_REG_GYRO_Z_L    0x38U /**< Gyroscope Z low byte register. */
#define K_ICM20948_REG_MAGNETO_X_L 0x3CU /**< Magnetometer X low byte register. */
#define K_ICM20948_REG_MAGNETO_X_H 0x3DU /**< Magnetometer X high byte register. */
#define K_ICM20948_REG_MAGNETO_Y_L 0x3EU /**< Magnetometer Y low byte register. */
#define K_ICM20948_REG_MAGNETO_Y_H 0x3FU /**< Magnetometer Y high byte register. */
#define K_ICM20948_REG_MAGNETO_Z_L 0x40U /**< Magnetometer Z low byte register. */
#define K_ICM20948_REG_MAGNETO_Z_H 0x41U /**< Magnetometer Z high byte register. */


/* ----------------------------------------------------------------
 * STRUCTS AND ENUMS
 * ---------------------------------------------------------------- */

/** @brief ICM20948 register bank selector. */
typedef enum
{
    K_ICM20948_REG_BANK_0 = 0x00U,
    K_ICM20948_REG_BANK_1 = 0x10U,
    K_ICM20948_REG_BANK_2 = 0x20U,
    K_ICM20948_REG_BANK_3 = 0x30U
} ICM20948_RegBank_t;

/** @brief REG_BANK_SEL register bit layout. */
typedef union
{
    spp_uint8_t value;
    struct
    {
        spp_uint8_t reserved0 : 4;
        spp_uint8_t bankSel   : 2;
        spp_uint8_t reserved1 : 2;
    } bits;
} ICM20948_RegBankSel_t;

/** @brief USER_CTRL register bit layout. */
typedef union
{
    spp_uint8_t value;
    struct
    {
        spp_uint8_t reserved0 : 1;
        spp_uint8_t i2cMstRst : 1;
        spp_uint8_t sramRst   : 1;
        spp_uint8_t dmpRst    : 1;
        spp_uint8_t i2cIfDis  : 1;
        spp_uint8_t i2cMstEn  : 1;
        spp_uint8_t fifoEn    : 1;
        spp_uint8_t dmpEn     : 1;
    } bits;
} ICM20948_RegUserCtrl_t;

/** @brief LP_CONFIG register bit layout. */
typedef union
{
    spp_uint8_t value;
    struct
    {
        spp_uint8_t reserved0 : 4;
        spp_uint8_t gyroCyc   : 1;
        spp_uint8_t accelCyc  : 1;
        spp_uint8_t i2cMstCyc : 1;
        spp_uint8_t reserved1 : 1;
    } bits;
} ICM20948_RegLpConf_t;

/** @brief Clock source selection for PWR_MGMT_1. */
typedef enum
{
    K_ICM20948_CLK_INTERNAL_20MHZ = 0U,
    K_ICM20948_CLK_AUTO = 1U,
    K_ICM20948_CLK_STOP = 7U
} ICM20948_ClockSel_t;

/** @brief PWR_MGMT_1 register bit layout. */
typedef union
{
    spp_uint8_t value;
    struct
    {
        spp_uint8_t clkSel      : 3;
        spp_uint8_t tempDis     : 1;
        spp_uint8_t reserved0   : 1;
        spp_uint8_t lpEn        : 1;
        spp_uint8_t sleep       : 1;
        spp_uint8_t deviceReset : 1;
    } bits;
} ICM20948_RegPwrMgmt1_t;

/** @brief PWR_MGMT_2 register bit layout. */
typedef union
{
    spp_uint8_t value;
    struct
    {
        spp_uint8_t disableAccelX : 1;
        spp_uint8_t disableAccelY : 1;
        spp_uint8_t disableAccelZ : 1;
        spp_uint8_t disableGyroX  : 1;
        spp_uint8_t disableGyroY  : 1;
        spp_uint8_t disableGyroZ  : 1;
        spp_uint8_t reserved0     : 2;
    } bits;
} ICM20948_RegPwrMgmt2_t;

/** @brief INT_ENABLE register bit layout. */
typedef union
{
    spp_uint8_t value;
    struct
    {
        spp_uint8_t reserved0   : 1;
        spp_uint8_t rawData0Rdy : 1;
        spp_uint8_t reserved1   : 6;
    } bits;
} ICM20948_RegIntEnable_t;

/** @brief INT_ENABLE_2 register bit layout. */
typedef union
{
    spp_uint8_t value;
    struct
    {
        spp_uint8_t fifoOverflowEn0 : 1;
        spp_uint8_t fifoOverflowEn1 : 1;
        spp_uint8_t fifoOverflowEn2 : 1;
        spp_uint8_t fifoOverflowEn3 : 1;
        spp_uint8_t reserved0       : 3;
        spp_uint8_t dmpInt1En       : 1;
    } bits;
} ICM20948_RegIntEnable2_t;

/** @brief INT_STATUS register bit layout. */
typedef union
{
    spp_uint8_t value;
    struct
    {
        spp_uint8_t reserved0   : 1;
        spp_uint8_t rawData0Rdy : 1;
        spp_uint8_t reserved1   : 6;
    } bits;
} ICM20948_RegIntStatus_t;

/** @brief DMP_INT_STATUS register bit layout. */
typedef union
{
    spp_uint8_t value;
    struct
    {
        spp_uint8_t dmpInt0   : 1;
        spp_uint8_t dmpInt1   : 1;
        spp_uint8_t dmpInt2   : 1;
        spp_uint8_t dmpInt3   : 1;
        spp_uint8_t dmpInt4   : 1;
        spp_uint8_t reserved0 : 3;
    } bits;
} ICM20948_RegDmpIntStatus_t;

/** @brief SINGLE_FIFO_PRIORITY_SEL register bit layout. */
typedef union
{
    spp_uint8_t value;
    struct
    {
        spp_uint8_t accelFifoPri : 2;
        spp_uint8_t gyroFifoPri  : 2;
        spp_uint8_t dmpFifoPri   : 2;
        spp_uint8_t reserved0    : 2;
    } bits;
} ICM20948_RegSingleFifoPrioritySel_t;

/** @brief FIFO_EN_1 register bit layout. */
typedef union
{
    spp_uint8_t value;
    struct
    {
        spp_uint8_t slv0FifoEn : 1;
        spp_uint8_t slv1FifoEn : 1;
        spp_uint8_t slv2FifoEn : 1;
        spp_uint8_t slv3FifoEn : 1;
        spp_uint8_t reserved0  : 4;
    } bits;
} ICM20948_RegFifoEn1_t;

/** @brief FIFO_EN_2 register bit layout. */
typedef union
{
    spp_uint8_t value;
    struct
    {
        spp_uint8_t accelFifoEn : 1;
        spp_uint8_t gyroXFifoEn : 1;
        spp_uint8_t gyroYFifoEn : 1;
        spp_uint8_t gyroZFifoEn : 1;
        spp_uint8_t tempFifoEn  : 1;
        spp_uint8_t reserved0   : 3;
    } bits;
} ICM20948_RegFifoEn2_t;

/** @brief FIFO_RST register bit layout. */
typedef union
{
    spp_uint8_t value;
    struct
    {
        spp_uint8_t fifoRst0  : 1;
        spp_uint8_t fifoRst1  : 1;
        spp_uint8_t fifoRst2  : 1;
        spp_uint8_t fifoRst3  : 1;
        spp_uint8_t fifoRst4  : 1;
        spp_uint8_t reserved0 : 3;
    } bits;
} ICM20948_RegFifoRst_t;

/** @brief FIFO_MODE register bit layout. */
typedef union
{
    spp_uint8_t value;
    struct
    {
        spp_uint8_t fifoMode  : 1;
        spp_uint8_t reserved0 : 7;
    } bits;
} ICM20948_RegFifoMode_t;

/** @brief FIFO_CFG register bit layout. */
typedef union
{
    spp_uint8_t value;
    struct
    {
        spp_uint8_t fifoCfg   : 1;
        spp_uint8_t reserved0 : 7;
    } bits;
} ICM20948_RegFifoCfg_t;

/** @brief HW_FIX_DISABLE register bit layout. */
typedef union
{
    spp_uint8_t value;
    struct
    {
        spp_uint8_t bit0 : 1;
        spp_uint8_t bit1 : 1;
        spp_uint8_t bit2 : 1;
        spp_uint8_t bit3 : 1;
        spp_uint8_t bit4 : 1;
        spp_uint8_t bit5 : 1;
        spp_uint8_t bit6 : 1;
        spp_uint8_t bit7 : 1;
    } bits;
} ICM20948_RegHwFixDisable_t;

/** @brief Gyroscope full-scale range selection. */
typedef enum
{
    K_ICM20948_GYRO_FS_250DPS = 0U,
    K_ICM20948_GYRO_FS_500DPS = 1U,
    K_ICM20948_GYRO_FS_1000DPS = 2U,
    K_ICM20948_GYRO_FS_2000DPS = 3U
} ICM20948_GyroFsSel_t;

/** @brief GYRO_CONFIG_1 register bit layout. */
typedef union
{
    spp_uint8_t value;
    struct
    {
        spp_uint8_t gyroFchoice : 1;
        spp_uint8_t gyroFsSel   : 2;
        spp_uint8_t gyroDlpfCfg : 3;
        spp_uint8_t reserved0   : 2;
    } bits;
} ICM20948_RegGyroConfig_t;

/** @brief Accelerometer full-scale range selection. */
typedef enum
{
    K_ICM20948_ACCEL_FS_2G = 0U,
    K_ICM20948_ACCEL_FS_4G = 1U,
    K_ICM20948_ACCEL_FS_8G = 2U,
    K_ICM20948_ACCEL_FS_16G = 3U
} ICM20948_AccelFsSel_t;

/** @brief ACCEL_CONFIG register bit layout. */
typedef union
{
    spp_uint8_t value;
    struct
    {
        spp_uint8_t accelFchoice : 1;
        spp_uint8_t accelFsSel   : 2;
        spp_uint8_t accelDlpfCfg : 3;
        spp_uint8_t reserved0    : 2;
    } bits;
} ICM20948_RegAccelConfig_t;

/** @brief ACCEL_CONFIG_2 register bit layout. */
typedef union
{
    spp_uint8_t value;
    struct
    {
        spp_uint8_t dec3Cfg   : 2;
        spp_uint8_t reserved0 : 6;
    } bits;
} ICM20948_RegAccelConfig2_t;

/** @brief I2C_MST_CTRL register bit layout. */
typedef union
{
    spp_uint8_t value;
    struct
    {
        spp_uint8_t i2cMstClk : 4;
        spp_uint8_t i2cMstPnt : 1;
        spp_uint8_t reserved0 : 3;
    } bits;
} ICM20948_RegI2cCtrl_t;

/** @brief I2C_SLVx_ADDR register bit layout. */
typedef union
{
    spp_uint8_t value;
    struct
    {
        spp_uint8_t i2cId        : 7;
        spp_uint8_t readNotWrite : 1;
    } bits;
} ICM20948_RegI2cSlvAddr_t;

/** @brief I2C_SLVx_CTRL register bit layout. */
typedef union
{
    spp_uint8_t value;
    struct
    {
        spp_uint8_t length   : 4;
        spp_uint8_t group    : 1;
        spp_uint8_t regDis   : 1;
        spp_uint8_t byteSwap : 1;
        spp_uint8_t enable   : 1;
    } bits;
} ICM20948_RegI2cSlvCtrl_t;

/** @brief I2C_MST_ODR_CONFIG register bit layout. */
typedef union
{
    spp_uint8_t value;
    struct
    {
        spp_uint8_t i2cMstOdr : 4;
        spp_uint8_t reserved0 : 4;
    } bits;
} ICM20948_RegI2cMstOdrConfig_t;

/**
 * @brief Parsed sensor sample from the DMP FIFO.
 */
typedef struct
{
    float ax;             /**< Accelerometer X (g)   */
    float ay;             /**< Accelerometer Y (g)   */
    float az;             /**< Accelerometer Z (g)   */
    float gx;             /**< Gyroscope X (dps)     */
    float gy;             /**< Gyroscope Y (dps)     */
    float gz;             /**< Gyroscope Z (dps)     */
    float mx;             /**< Magnetometer X (uT)   */
    float my;             /**< Magnetometer Y (uT)   */
    float mz;             /**< Magnetometer Z (uT)   */
    float qw;             /**< Quaternion W (uT)     */
    float qx;             /**< Quaternion X (uT)     */
    float qy;             /**< Quaternion Y (uT)     */
    float qz;             /**< Quaternion Z (uT)     */
    float accuracy;       /**< Quaternion accuracy   */
    spp_bool_t dataReady; /**< Set when a FIFO packet has been parsed. */
} ICM20948_SensorData_t;

/**
 * @brief ICM20948 interrupt and GPIO context.
 */
typedef struct
{
    volatile spp_bool_t drdyFlag; /**< Set by ISR on data-ready interrupt. */
    SPP_GpioIsrCtx_t isr_ctx;     /**< ISR context (points at drdyFlag).   */
    spp_uint32_t intPin;          /**< GPIO interrupt pin number.           */
    spp_uint32_t intIntrType;     /**< Interrupt trigger type (1=rising).   */
    spp_uint32_t intPull;         /**< Pull resistor: 0=none, 1=up, 2=down. */
} ICM20948_Data_t;

/**
 * @brief ICM20948 sensor instance.
 *
 * One static instance (@c s_icmData) is kept inside icm20948.c; its hardware
 * fields are set from K_ICM20948_* constants by SPP_SERVICES_ICM20948_init.
 */
typedef struct
{
    /* Hardware configuration — set at declaration */
    spp_uint8_t spiDevIdx;    /**< SPI device handle index.            */
    spp_uint32_t intPin;      /**< GPIO interrupt pin number.          */
    spp_uint32_t intIntrType; /**< Interrupt edge type (1=rising).     */
    spp_uint32_t intPull;     /**< Pull resistor (0=none, 1=up).       */

    /* Runtime state — filled in by init, do not set manually */
    void *p_spi;                    /**< SPI device handle.              */
    ICM20948_Data_t icmData;        /**< Interrupt flag and ISR context. */
    ICM20948_SensorData_t lastData; /**< Last parsed FIFO sample.        */
    spp_uint16_t seq;               /**< Packet sequence counter.        */
} ICM20948_t;


/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */
const SPP_SERVICE_ProducerContract_t *SPP_SERVICES_ICM20948_getProducerContract(void);

#endif /* SPP_ICM20948_H */
