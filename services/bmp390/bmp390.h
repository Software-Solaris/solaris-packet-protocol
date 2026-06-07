/**
 * @file bmp390.h
 * @brief BMP390 barometric pressure sensor driver and service.
 *
 * Provides SPI-based communication with the Bosch BMP390 sensor for reading
 * calibrated temperature, pressure and derived altitude.  Supports data-ready
 * interrupt via GPIO event groups.
 *
 * The driver follows a three-step workflow:
 * 1. Initialise and configure the sensor (SPP_SERVICES_BMP390_init, SPP_SERVICES_BMP390_auxConfig,
 *    SPP_SERVICES_BMP390_prepareMeasure).
 * 2. Wait for the data-ready interrupt (SPP_SERVICES_BMP390_waitDrdy).
 * 3. Read compensated altitude/pressure/temperature (SPP_SERVICES_BMP390_getAltitude).
 *
 * To use the SPP service registry, declare a @ref BMP390_t with the hardware
 * pin fields filled in, then call SPP_SERVICES_register() with @ref g_bmp390Module.
 *
 * Naming conventions used in this file:
 * - Constants/macros:  K_BMP390_*
 * - Types:             BMP390_*_t
 * - Public functions:  BMP390_*()
 * - Pointer params:    p_*
 */

#ifndef SPP_BMP390_H
#define SPP_BMP390_H

#include <stdint.h>
#include "spp/core/returnTypes.h"
#include "spp/core/types.h"
#include "spp/hal/gpio/gpio.h"
#include "spp/services/service.h"

/* ----------------------------------------------------------------
 * DEFINES
 * ---------------------------------------------------------------- */

/* Service */
#define K_BMP_SERVICE_TASK_PRIO       (5U)    /**< FreeRTOS task priority for the BMP390 service. */
#define K_BMP_SERVICE_TASK_DELAY_MS   (200U)  /**< Delay in ms between measurement cycles. */
#define K_BMP_SERVICE_TASK_STACK_SIZE (4096U) /**< Stack size in bytes for the BMP390 service task. */
#define K_BMP_SERVICE_PAYLOAD_LEN     (12U)   /**< Payload length in bytes (temp + press + alt). */

/* SPI */
#define K_BMP390_SPI_READ    0x80U /**< SPI read flag: OR with register address. */
#define K_BMP390_SPI_WRITE   0x00U /**< SPI write flag: OR with register address. */
#define K_BMP390_PIN_NUM_CS  18U   /**< Chip-select GPIO pin (informational; set by HAL). */
#define K_BMP390_SPI_BUS_IDX 1U    /**< SPI bus index (informational; set by HAL). */

/* Identity and reset */
#define K_BMP390_CHIP_ID_REG    0x00 /**< Chip ID register address. */
#define K_BMP390_CHIP_ID_VALUE  0x60 /**< Expected chip ID value. */
#define K_BMP390_SOFT_RESET_REG 0x7E /**< Soft-reset register address. */
#define BMP390_SOFT_RESET_CMD   0xB6 /**< Soft-reset command byte. */
#define K_BMP390_IF_CONF_REG    0x1A /**< Interface configuration register address. */
#define BMP390_IF_CONF_SPI      0x00 /**< SPI interface mode value. */

/* Measurement configuration */
#define K_BMP390_REG_PWRCTRL 0x1B /**< Power control register address. */
#define BMP390_VALUE_PWRCTRL 0x33 /**< Power control value (temp+press on, normal mode). */
#define K_BMP390_REG_OSR     0x1C /**< Oversampling register address. */
#define BMP390_VALUE_OSR     0x00 /**< Oversampling value (no oversampling). */
#define K_BMP390_REG_ODR     0x1D /**< Output data rate register address. */
#define BMP390_VALUE_ODR     0x02 /**< Output data rate value. */
#define K_BMP390_REG_IIR     0x1F /**< IIR filter register address. */
#define BMP390_VALUE_IIR     0x02 /**< IIR filter coefficient value. */

/* Status */
#define K_BMP390_REG_STATUS       0x03 /**< Status register address. */
#define K_BMP390_STATUS_DRDY_TEMP 0x40 /**< Status bit: temperature data ready. */
#define BMP390_STATUS_DRDY_PRES   0x20 /**< Status bit: pressure data ready. */

/* Temperature */
#define K_BMP390_TEMP_CALIB_REG_START 0x31  /**< Start address of temperature calibration regs. */
#define K_BMP390_TEMP_RAW_REG         0x07  /**< Start address of raw temperature data regs. */
#define K_BMP390_DRDY_TIMEOUT_MS      5000U /**< Timeout in ms for the data-ready interrupt wait. */

/* Pressure */
#define K_BMP390_PRESS_CALIB_REG_START 0x36 /**< Start address of pressure calibration regs. */
#define K_BMP390_PRESS_RAW_REG         0x04 /**< Start address of raw pressure data regs. */

/* Interrupt */
#define K_BMP390_REG_INT_CTRL     0x19 /**< Interrupt control register address. */
#define K_BMP390_INT_CTRL_DRDY_EN 0x40 /**< Interrupt control: enable data-ready interrupt. */
#define K_BMP390_INT_CTRL_LEVEL   0x02 /**< Interrupt control: active-high level. */
#define K_BMP390_INT_PIN_NUM      10U  /**< Interrupt GPIO pin */
#define K_BMP390_INT_INTR_TYPE    1U   /**< Interrupt trigger type (1=rising). */
#define K_BMP390_INT_PULL         0U   /**< Pull resistor: 0=none 1=up 2=down. */


/* ----------------------------------------------------------------
 * STRUCTS AND ENUMS
 * ---------------------------------------------------------------- */

/** @brief Raw temperature calibration coefficients (as read from sensor). */
typedef struct
{
    uint16_t par_t1; /**< T1 (unsigned 16-bit). */
    int16_t par_t2;  /**< T2 (signed 16-bit).   */
    int8_t par_t3;   /**< T3 (signed 8-bit).    */
    float t_lin;     /**< Linearised temperature. */
} BMP390_temp_calib_t;

/** @brief Scaled temperature calibration parameters. */
typedef struct
{
    float PAR_T1; /**< Scaled T1 = raw_t1 * 2^8.   */
    float PAR_T2; /**< Scaled T2 = raw_t2 / 2^30.  */
    float PAR_T3; /**< Scaled T3 = raw_t3 / 2^48.  */
} BMP390_temp_params_t;

/** @brief Raw pressure calibration coefficients. */
typedef struct
{
    spp_uint16_t par_p1; /**< P1 raw coefficient. */
    spp_uint16_t par_p2; /**< P2 raw coefficient. */
    spp_int8_t par_p3;   /**< P3 raw coefficient. */
    spp_int8_t par_p4;   /**< P4 raw coefficient. */
    spp_uint16_t par_p5; /**< P5 raw coefficient. */
    spp_uint16_t par_p6; /**< P6 raw coefficient. */
    spp_int8_t par_p7;   /**< P7 raw coefficient. */
    spp_int8_t par_p8;   /**< P8 raw coefficient. */
    spp_int16_t par_p9;  /**< P9 raw coefficient. */
    spp_int8_t par_p10;  /**< P10 raw coefficient. */
    spp_int8_t par_p11;  /**< P11 raw coefficient. */
} BMP390_press_calib_t;

/** @brief Scaled pressure calibration parameters. */
typedef struct
{
    float PAR_P1;  /**< Scaled P1.  */
    float PAR_P2;  /**< Scaled P2.  */
    float PAR_P3;  /**< Scaled P3.  */
    float PAR_P4;  /**< Scaled P4.  */
    float PAR_P5;  /**< Scaled P5.  */
    float PAR_P6;  /**< Scaled P6.  */
    float PAR_P7;  /**< Scaled P7.  */
    float PAR_P8;  /**< Scaled P8.  */
    float PAR_P9;  /**< Scaled P9.  */
    float PAR_P10; /**< Scaled P10. */
    float PAR_P11; /**< Scaled P11. */
} BMP390_press_params_t;

/* ----------------------------------------------------------------
* STRUCTS AND ENUMS
* ---------------------------------------------------------------- */

typedef struct
{
    spp_uint8_t spiDevIdx; /**< SPI device handle index.        */
    void *p_spiHandler;    /**< SPI device handle.              */
} BMP390_SpiConfig_t;
typedef struct
{
    spp_uint32_t intPin;          /**< GPIO pin for DRDY interrupt.    */
    spp_uint32_t intIntrType;     /**< Interrupt trigger type (1=rising). */
    spp_uint32_t intPull;         /**< Pull resistor: 0=none 1=up 2=down. */
    volatile spp_bool_t drdyFlag; /**< Interrupt flag.                 */
} BMP390_GpioConfig_t;
/**
 * @brief BMP390 sensor instance.
 *
 * Declare one static instance with the hardware pin fields filled in, then
 * pass its address to SPP_SERVICES_register().  All other fields are
 * zero-initialised by the compiler and filled in by the init callback.
 */
typedef struct
{
    BMP390_GpioConfig_t gpioConfig; /**< GPIO configuration.        */
    BMP390_SpiConfig_t spiConfig;   /**< SPI configuration.        */
    spp_uint16_t seq;               /**< Packet sequence counter.        */
} BMP390_t;


/* ----------------------------------------------------------------
*  PUBLIC FUNCTIONS
* ---------------------------------------------------------------- */
const SPP_SERVICE_ProducerContract_t *SPP_SERVICES_BMP390_getProducerContract(void);

#endif /* SPP_BMP390_H */
