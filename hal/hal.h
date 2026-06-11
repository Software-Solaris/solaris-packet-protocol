/**
 * @file hal.h
 * @brief This file contains the structs that contain all the pointers to the functions that are used by the SPP library.
 *        The structs are used to abstract the hardware and to make the code more portable.
 *        It also contains the public functions needed to initialize all these structs.
 */

#ifndef SPP_HAL_PORT_H
#define SPP_HAL_PORT_H

#include "spp/core/types.h"
#include "spp/core/returnTypes.h"

/* ----------------------------------------------------------------
 * STRUCTS
 * ---------------------------------------------------------------- */

/**
* @brief SPI bus struct that contains all the pointers to the SPI functions
*/
typedef struct
{
    /**
     * @brief Initialise the SPI bus.
     *
     * May be called multiple times; subsequent calls must be no-ops.
     *
     * @return K_SPP_OK on success.
     */
    SPP_RetVal_t (*spiBusInit)(void);

    /**
     * @brief Obtain the handle for the next SPI device slot.
     *
     * Devices are numbered sequentially starting from 0.  The port
     * determines which physical device each index maps to.
     *
     * @param[in] deviceIdx  Zero-based device index.
     *
     * @return Opaque device handle, or NULL if the index is out of range.
     */
    void *(*spiGetHandle)(spp_uint8_t deviceIdx);

    /**
     * @brief Initialise a specific SPI device.
     *
     * @param[in] p_handle  Handle returned by @c spiGetHandle.
     *
     * @return K_SPP_OK on success, K_SPP_ERROR_NULL_POINTER if handle is NULL.
     */
    SPP_RetVal_t (*spiDeviceInit)(void *p_handle);

    /**
     * @brief Perform a full-duplex SPI transaction.
     *
     * The buffer @p p_data is used for both TX and RX.  The port inspects
     * the MSB of the first byte to determine read (1) vs write (0).
     *
     * @param[in,out] p_handle  SPI device handle.
     * @param[in,out] p_data    TX data in, RX data out.
     * @param[in]     length    Number of bytes in the transaction.
     *
     * @return K_SPP_OK on success, K_SPP_ERROR_ON_SPI_TRANSACTION on failure.
     */
    SPP_RetVal_t (*spiTransmit)(void *p_handle, spp_uint8_t *p_data, spp_uint8_t length);

} SPP_HALSpi_t;

/**
* @brief UART port struct that contains all the pointers to the UART functions
*/
typedef struct
{
    /**
     * @brief Initialise the UART port.
     *
     * @param[in] p_cfg Pointer to UART init configuration.
     *
     * @return K_SPP_OK on success.
     */
    SPP_RetVal_t (*uartPortInit)(void *p_cfg);

} SPP_HALUart_t;

/**
* @brief GPIO struct that contains all the pointers to the GPIO functions
*/
typedef struct
{
    /**
     * @brief Configure a GPIO pin as an interrupt input.
     *
     * @param[in] pin       GPIO pin number.
     * @param[in] intrType  Platform-specific interrupt trigger type.
     * @param[in] pull      Pull resistor: 0 = none, 1 = pull-up, 2 = pull-down.
     *
     * @return K_SPP_OK on success.
     */
    SPP_RetVal_t (*gpioConfigInterrupt)(spp_uint32_t pin, spp_uint32_t intrType, spp_uint32_t pull);

    /**
     * @brief Register an ISR handler for a GPIO pin.
     *
     * @param[in] pin        GPIO pin number.
     * @param[in] p_isrCtx   Pointer to @ref SPP_GpioIsrCtx_t for this pin.
     *
     * @return K_SPP_OK on success.
     */
    SPP_RetVal_t (*gpioRegisterIsr)(spp_uint32_t pin, void *p_isrCtx);
} SPP_HALGpio_t;

/**
* @brief Storage struct that contains all the pointers to the Storage functions
*/
typedef struct
{
    /**
     * @brief Mount the storage filesystem.  NULL if storage is not used.
     *
     * @param[in] p_cfg  Pointer to @ref SPP_StorageInitCfg_t.
     *
     * @return K_SPP_OK on success, K_SPP_ERROR on failure.
     */
    SPP_RetVal_t (*storageMount)(void *p_cfg);

    /**
     * @brief Unmount the storage filesystem.  NULL if storage is not used.
     *
     * @param[in] p_cfg  Pointer to @ref SPP_StorageInitCfg_t.
     *
     * @return K_SPP_OK on success.
     */
    SPP_RetVal_t (*storageUnmount)(void *p_cfg);
} SPP_HALStorage_t;

/**
* @brief Time struct that contains all the pointers to the Time functions
*/
typedef struct
{
    /**
     * @brief Return the current hardware time in milliseconds.
     *
     * Used to timestamp packets.  Must be monotonically non-decreasing.
     *
     * @return Elapsed time in ms.
     */
    spp_uint32_t (*getTimeMs)(void);

    /**
     * @brief Block for the requested number of milliseconds.
     *
     * Used during sensor initialisation sequences that require a fixed delay.
     * On baremetal targets this is a busy-wait; on RTOS targets it may yield.
     *
     * @param[in] ms  Number of milliseconds to delay.
     */
    void (*delayMs)(spp_uint32_t ms);
} SPP_HALTime_t;


/**
* @brief Ports struct that contains all the pointers to the Ports functions
*/
typedef struct
{
    SPP_HALSpi_t spi;
    SPP_HALGpio_t gpio;
    SPP_HALStorage_t storage;
    SPP_HALTime_t time;
    SPP_HALUart_t uart;

} SPP_HalPort_t;


/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */

/**
* @brief This functions initializes the ports struct and initializes all the peripherals.
*/
SPP_RetVal_t SPP_HAL_init(const SPP_HalPort_t *p_port);

/**
* @brief This function is used to get the pointers to the functions that are used by the SPP library.
* The structs are used to abstract the hardware and to make the code more portable.
* It also contains the public functions needed to initialize all these structs.
*
* @param[in] p_ports  Pointer to the ports struct that will be initialized.
* @return NULL if the pointer to the ports struct is NULL.
* @return Pointer to the ports struct, if it has beeen initialized.
 */
const SPP_HalPort_t *SPP_HAL_getPort(void);


#endif /* SPP_HAL_PORT_H */
