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
     * @brief Performs SPI register transactions.
     *
     * The ESP32 port uses the MSB of the command byte to distinguish
     * reads from writes and processes the buffer accordingly.
     *
     * @param p_handle SPI device handle.
     * @param p_data Transmission buffer, also used for received data on reads.
     * @param length Buffer length in bytes.
     * @return K_SPP_OK on success, error code otherwise.
     */
    SPP_RetVal_t (*spiTransmit)(void *p_handle, spp_uint8_t *p_data, spp_uint8_t length);

    /**
     * @brief Remove and re-add a SPI device with a new clock speed.
     *
     * Intended for devices that require a lower speed during initialisation
     * and a higher speed for normal operation (e.g. SD cards).
     *
     * @param[in] p_handle  Handle returned by @c spiGetHandle.
     * @param[in] speedHz   New clock frequency in Hz.
     *
     * @return K_SPP_OK on success, K_SPP_ERROR on failure.
     */
    SPP_RetVal_t (*spiDeviceSetSpeed)(void *p_handle, spp_uint32_t speedHz);

    /**
     * @brief Performs a raw full-duplex SPI transaction.
     * @param p_handle SPI device handle.
     * @param p_data Transmission and reception buffer.
     * @param length Number of bytes to transfer.
     * @return K_SPP_OK on success, error code otherwise.
     */
    SPP_RetVal_t (*spiTransmitRaw)(void *p_handle, spp_uint8_t *p_data, spp_uint8_t length);


} SPP_HALSpi_t;

/**
* @brief UART port struct that contains all the pointers to the UART functions
*/
typedef struct
{
    /**
     * @brief Initialise the UART port.
     *
     * @return K_SPP_OK on success.
     */
    SPP_RetVal_t (*uartPortInit)(void);

    /**
     * @brief Transmits data through the UART port.
     * @param p_data Pointer to the data to transmit.
     * @param len Number of bytes to transmit.
     * @return K_SPP_OK on success, error code otherwise.
     */
    SPP_RetVal_t (*uartTransmit)(const void *p_data, spp_uint32_t len);

    /**
     * @brief Reads data from the UART.
     *
     * @param[out] p_data      Pointer to the buffer where received data will be stored.
     * @param[in]  len         Number of bytes to read.
     * @param[out] p_readBytes Pointer to the variable where the number of bytes read will be stored.
     * 
     * @return K_SPP_OK on success, error code otherwise.
     */

    SPP_RetVal_t (*uartRead)(void *p_data, spp_uint32_t len, spp_uint32_t *p_readBytes);

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

    /**
     * @brief Reads the logic level of a GPIO pin.
     * @param pin GPIO pin number.
     * @return Current logic level (0 or 1).
     */
    spp_uint16_t (*gpioRead)(spp_uint32_t pin);

    /**
     * @brief Configures a GPIO as output and sets its logic level.
     * @param pin GPIO pin number.
     * @param level Logic level to set.
     * @return K_SPP_OK on success, error code otherwise.
     */
    SPP_RetVal_t (*gpioWrite)(spp_uint32_t pin, spp_uint8_t level);

} SPP_HALGpio_t;


/**
 * @brief Storage struct that contains all the pointers to the storage functions
 */
typedef struct
{
    /**
     * @brief Initialize the storage device.
     *
     * @return K_SPP_OK on success, K_SPP_ERROR on failure.
     */
    SPP_RetVal_t (*storageInit)(void);

    /**
     * @brief Write consecutive blocks to the storage device.
     *
     * @param[in]     p_buffer    Data to write.
     * @param[in]     first_block First logical block to write.
     * @param[in]     count       Number of blocks to write.
     *
     * @return K_SPP_OK on success, K_SPP_ERROR on failure.
     */
    SPP_RetVal_t (*storageWrite)(const void *p_buffer, spp_uint32_t first_block, spp_uint16_t count);

} SPP_HALStorage_t;


/**
* @brief Time struct that contains all the pointers to the Time functions
*/
typedef struct
{
    /**
     * @brief Return the current hardware time in milliseconds.
     *
     * Used to timestamp packets.
     *
     * @return Elapsed time in ms.
     */
    spp_uint32_t (*getTimeMs)(void);

    /**
     * @brief Return the current hardware time in microseconds.
     *
     * Used to timestamp packets.
     *
     * @return Elapsed time in us.
     */
    spp_uint32_t (*getTimeUs)(void);

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
SPP_RetVal_t SPP_HAL_init(void *p_halPorts);

/**
 * @brief Returns the registered HAL port.
 * @return Pointer to the HAL port, or NULL if not initialized.
 */
const SPP_HalPort_t *SPP_HAL_getPort(void);

#endif /* SPP_HAL_PORT_H */
