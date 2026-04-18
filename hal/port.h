/**
 * @file port.h
 * @brief SPP HAL port contract — the interface every hardware port must implement.
 *
 * To port SPP to a new MCU (ESP32, STM32, RP2040…):
 *   1. Allocate a static @ref SPP_HalPort_t.
 *   2. Fill every mandatory function pointer.  Optional pointers may be NULL.
 *   3. Call @ref SPP_CORE_setHalPort() before @ref SPP_CORE_init().
 *
 * Naming conventions used in this file:
 * - Types: SPP_HalPort_t
 * - Pointer parameters: p_*
 */

#ifndef SPP_HAL_PORT_H
#define SPP_HAL_PORT_H

#include "spp/core/types.h"
#include "spp/core/returnTypes.h"

/* ----------------------------------------------------------------
 * HAL port struct
 * ---------------------------------------------------------------- */

/**
 * @brief Hardware abstraction layer port descriptor.
 *
 * Register one instance per build target via @ref SPP_CORE_setHalPort().
 * Pointers marked as optional may be set to NULL if the feature is not used.
 */
typedef struct
{
    /* ---- SPI bus ----------------------------------------------- */

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
    SPP_RetVal_t (*spiTransmit)(void *p_handle, spp_uint8_t *p_data,
                             spp_uint8_t length);

    /* ---- GPIO / interrupts ------------------------------------- */

    /**
     * @brief Configure a GPIO pin as an interrupt input.
     *
     * @param[in] pin       GPIO pin number.
     * @param[in] intrType  Platform-specific interrupt trigger type.
     * @param[in] pull      Pull resistor: 0 = none, 1 = pull-up, 2 = pull-down.
     *
     * @return K_SPP_OK on success.
     */
    SPP_RetVal_t (*gpioConfigInterrupt)(spp_uint32_t pin, spp_uint32_t intrType,
                                    spp_uint32_t pull);

    /**
     * @brief Register an ISR handler for a GPIO pin.
     *
     * @param[in] pin        GPIO pin number.
     * @param[in] p_isrCtx   Pointer to @ref SPP_GpioIsrCtx_t for this pin.
     *
     * @return K_SPP_OK on success.
     */
    SPP_RetVal_t (*gpioRegisterIsr)(spp_uint32_t pin, void *p_isrCtx);

    /* ---- Storage (optional) ------------------------------------ */

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

    /* ---- Time -------------------------------------------------- */

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

} SPP_HalPort_t;

#endif /* SPP_HAL_PORT_H */
