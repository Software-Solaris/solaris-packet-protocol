/**
 * @file spi.h
 * @brief SPP SPI HAL API — dispatches through the registered HAL port.
 *
 * Naming conventions used in this file:
 * - Public functions: SPP_HAL_spi*()
 * - Pointer parameters: p_*
 */

#ifndef SPP_HAL_SPI_H
#define SPP_HAL_SPI_H

#include "spp/core/types.h"
#include "spp/core/returntypes.h"

/* ----------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------- */

/**
 * @brief Initialise the SPI bus.
 *
 * Safe to call multiple times; subsequent calls are no-ops.
 *
 * @return K_SPP_OK on success.
 */
SPP_RetVal_t SPP_HAL_spiBusInit(void);

/**
 * @brief Obtain the SPI device handle for the given device index.
 *
 * @param[in] deviceIdx  Zero-based device index (0 = first device, etc.).
 *
 * @return Opaque handle on success, NULL if index is out of range.
 */
void *SPP_HAL_spiGetHandle(spp_uint8_t deviceIdx);

/**
 * @brief Initialise a specific SPI device.
 *
 * @param[in] p_handle  Handle returned by @ref SPP_HAL_spiGetHandle().
 *
 * @return K_SPP_OK on success, K_SPP_ERROR_NULL_POINTER if handle is NULL.
 */
SPP_RetVal_t SPP_HAL_spiDeviceInit(void *p_handle);

/**
 * @brief Perform a full-duplex SPI transaction.
 *
 * @param[in,out] p_handle  SPI device handle.
 * @param[in,out] p_data    TX data in, RX data out (in-place).
 * @param[in]     length    Number of bytes in the transaction.
 *
 * @return K_SPP_OK on success, K_SPP_ERROR_ON_SPI_TRANSACTION on failure.
 */
SPP_RetVal_t SPP_HAL_spiTransmit(void *p_handle, spp_uint8_t *p_data,
                               spp_uint8_t length);

#endif /* SPP_HAL_SPI_H */
