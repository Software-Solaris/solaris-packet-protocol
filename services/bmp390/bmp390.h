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
 * To use the SPP service registry, allocate a @ref BMP390_ServiceCtx_t and a
 * @ref BMP390_ServiceCfg_t, then call SPP_SERVICES_register() with
 * @ref g_bmp390ServiceDesc.
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
#include "spp/hal/gpio.h"
#include "spp/services/service.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Initialisation Constants
 * ========================================================================= */

/** @brief Timeout in ms for waiting for the BMP390 data-ready interrupt. */
#define K_BMP390_DRDY_TIMEOUT_MS     5000U

/* ============================================================================
 * Data Types — driver context
 * ========================================================================= */

/**
 * @brief BMP390 device context.
 *
 * Groups the SPI handler, DRDY flag, ISR context and GPIO interrupt
 * configuration required to operate one BMP390 sensor instance.
 */
typedef struct
{
    void              *p_handler_spi;  /**< SPI device handle.                         */
    volatile spp_bool_t drdyFlag;      /**< Set by ISR when data-ready fires.          */
    SPP_GpioIsrCtx_t   isr_ctx;       /**< ISR context (points at drdyFlag).          */
    spp_uint32_t       intPin;         /**< GPIO pin number for the interrupt.         */
    spp_uint32_t       intIntrType;    /**< Interrupt trigger type.                    */
    spp_uint32_t       intPull;        /**< Pull resistor: 0=none, 1=up, 2=down.       */
} BMP390_Data_t;

/** @brief Chip-select GPIO pin for the BMP390 (informational; set by HAL). */
#define K_BMP390_PIN_NUM_CS 18

/* ============================================================================
 * Configuration and Identity Check
 * ========================================================================= */

#define K_BMP390_CHIP_ID_REG     0x00
#define K_BMP390_CHIP_ID_VALUE   0x60
#define K_BMP390_SOFT_RESET_REG  0x7E
#define BMP390_SOFT_RESET_CMD    0xB6
#define K_BMP390_IF_CONF_REG     0x1A
#define BMP390_IF_CONF_SPI       0x00

/* ============================================================================
 * Measurement Configuration
 * ========================================================================= */

#define K_BMP390_REG_PWRCTRL   0x1B
#define BMP390_VALUE_PWRCTRL   0x33
#define K_BMP390_REG_OSR       0x1C
#define BMP390_VALUE_OSR       0x00
#define K_BMP390_REG_ODR       0x1D
#define BMP390_VALUE_ODR       0x02
#define K_BMP390_REG_IIR       0x1F
#define BMP390_VALUE_IIR       0x02
#define K_BMP390_REG_STATUS    0x03
#define K_BMP390_STATUS_DRDY_TEMP 0x40
#define BMP390_STATUS_DRDY_PRES   0x20

/* ============================================================================
 * Temperature Calibration and Reading
 * ========================================================================= */

#define K_BMP390_TEMP_CALIB_REG_START 0x31

/** @brief Raw temperature calibration coefficients (as read from sensor). */
typedef struct
{
    uint16_t par_t1; /**< T1 (unsigned 16-bit). */
    int16_t  par_t2; /**< T2 (signed 16-bit).   */
    int8_t   par_t3; /**< T3 (signed 8-bit).    */
    float    t_lin;  /**< Linearised temperature. */
} BMP390_temp_calib_t;

/** @brief Scaled temperature calibration parameters. */
typedef struct
{
    float PAR_T1; /**< Scaled T1 = raw_t1 * 2^8.   */
    float PAR_T2; /**< Scaled T2 = raw_t2 / 2^30.  */
    float PAR_T3; /**< Scaled T3 = raw_t3 / 2^48.  */
} BMP390_temp_params_t;

#define K_BMP390_TEMP_RAW_REG 0x07

/* ============================================================================
 * Pressure Calibration and Reading
 * ========================================================================= */

#define K_BMP390_PRESS_CALIB_REG_START 0x36

/** @brief Raw pressure calibration coefficients. */
typedef struct
{
    spp_uint16_t par_p1;
    spp_uint16_t par_p2;
    spp_int8_t   par_p3;
    spp_int8_t   par_p4;
    spp_uint16_t par_p5;
    spp_uint16_t par_p6;
    spp_int8_t   par_p7;
    spp_int8_t   par_p8;
    spp_int16_t  par_p9;
    spp_int8_t   par_p10;
    spp_int8_t   par_p11;
} BMP390_press_calib_t;

/** @brief Scaled pressure calibration parameters. */
typedef struct
{
    float PAR_P1;
    float PAR_P2;
    float PAR_P3;
    float PAR_P4;
    float PAR_P5;
    float PAR_P6;
    float PAR_P7;
    float PAR_P8;
    float PAR_P9;
    float PAR_P10;
    float PAR_P11;
} BMP390_press_params_t;

#define K_BMP390_PRESS_RAW_REG 0x04

/* ============================================================================
 * Interrupt Configuration
 * ========================================================================= */

#define K_BMP390_REG_INT_CTRL        0x19
#define K_BMP390_INT_CTRL_DRDY_EN    0x40
#define K_BMP390_INT_CTRL_LEVEL      0x02

/* ============================================================================
 * Service types
 * ========================================================================= */

/**
 * @brief Configuration for the BMP390 service instance.
 *
 * Passed by the caller to SPP_SERVICES_register() and stored by reference in
 * the context.  Must remain valid for the lifetime of the service.
 */
typedef struct
{
    spp_uint8_t  spiDevIdx;   /**< SPI device handle index.        */
    spp_uint32_t intPin;      /**< GPIO pin for DRDY interrupt.    */
    spp_uint32_t intIntrType; /**< Interrupt edge type (1=rising). */
    spp_uint32_t intPull;     /**< Pull resistor (0=none,1=up).    */
} BMP390_ServiceCfg_t;

/**
 * @brief Runtime context for the BMP390 service instance.
 *
 * Allocated statically by the caller; size equals sizeof(BMP390_ServiceCtx_t).
 */
typedef struct
{
    void                      *p_spi;   /**< SPI device handle.                */
    BMP390_Data_t              bmpData; /**< Sensor driver context.            */
    spp_uint16_t               seq;     /**< Packet sequence counter.          */
    const BMP390_ServiceCfg_t *p_cfg;   /**< Back-pointer to config struct.    */
} BMP390_ServiceCtx_t;

/**
 * @brief BMP390 service descriptor — pass to SPP_SERVICES_register().
 */
extern const SPP_ServiceDesc_t g_bmp390ServiceDesc;

/**
 * @brief Service task — call from the superloop when drdyFlag is set.
 *
 * Clears drdyFlag, reads altitude/pressure/temperature, and publishes
 * a packet via SPP_SERVICES_PUBSUB_publish().
 *
 * @param[in,out] p_ctx  Pointer to the service context.
 */
void SPP_SERVICES_BMP390_serviceTask(BMP390_ServiceCtx_t *p_ctx);

/* ============================================================================
 * Driver API (low-level — used internally and for testing)
 * ========================================================================= */

void     SPP_SERVICES_BMP390_init(void *p_data);
SPP_RetVal_t SPP_SERVICES_BMP390_softReset(void *p_spi);
SPP_RetVal_t SPP_SERVICES_BMP390_enableSpiMode(void *p_spi);
SPP_RetVal_t SPP_SERVICES_BMP390_configCheck(void *p_spi);
SPP_RetVal_t SPP_SERVICES_BMP390_auxConfig(void *p_spi);
SPP_RetVal_t SPP_SERVICES_BMP390_prepareMeasure(void *p_spi);
SPP_RetVal_t SPP_SERVICES_BMP390_waitDrdy(BMP390_Data_t *p_bmp, spp_uint32_t timeout_ms);
SPP_RetVal_t SPP_SERVICES_BMP390_readRawTempCoeffs(void *p_spi, BMP390_temp_calib_t *tcalib);
SPP_RetVal_t SPP_SERVICES_BMP390_calibrateTempParams(void *p_spi, BMP390_temp_params_t *out);
SPP_RetVal_t SPP_SERVICES_BMP390_readRawTemp(void *p_spi, uint32_t *raw_temp);
float    SPP_SERVICES_BMP390_compensateTemperature(spp_uint32_t raw_temp, BMP390_temp_params_t *params);
SPP_RetVal_t SPP_SERVICES_BMP390_auxGetTemp(void *p_spi, const BMP390_temp_params_t *temp_params,
                             spp_uint32_t *raw_temp, float *comp_temp);
SPP_RetVal_t SPP_SERVICES_BMP390_readRawPressCoeffs(void *p_spi, BMP390_press_calib_t *pcalib);
SPP_RetVal_t SPP_SERVICES_BMP390_calibratePressParams(void *p_spi, BMP390_press_params_t *out);
SPP_RetVal_t SPP_SERVICES_BMP390_readRawPress(void *p_spi, spp_uint32_t *raw_press);
float    SPP_SERVICES_BMP390_compensatePressure(spp_uint32_t raw_press, float t_lin,
                                    BMP390_press_params_t *p);
SPP_RetVal_t SPP_SERVICES_BMP390_auxGetPress(void *p_spi, const BMP390_press_params_t *press_params,
                              float t_lin, spp_uint32_t *raw_press, float *comp_press);
SPP_RetVal_t SPP_SERVICES_BMP390_getAltitude(void *p_spi, BMP390_Data_t *p_bmp, float *altitude_m,
                            float *pressure_pa, float *temperature_c);
SPP_RetVal_t SPP_SERVICES_BMP390_intEnableDrdy(void *p_spi);

#ifdef __cplusplus
}
#endif

#endif /* SPP_BMP390_H */
