/**
 * @file storage.h
 * @brief SPP storage HAL API — dispatches through the registered HAL port.
 *
 */

#ifndef SPP_HAL_STORAGE_H
#define SPP_HAL_STORAGE_H

#include "spp/core/returnTypes.h"

/* ----------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------- */

/**
 * @brief Initializes the storage device through the registered HAL port.
 * @return K_SPP_OK on success, K_SPP_ERROR on initialization failure,
 *         or K_SPP_ERROR_NULL_POINTER if the HAL is not initialized.
 */
SPP_RetVal_t SPP_HAL_STORAGE_init(void);

/**
 * @brief Writes consecutive storage blocks through the HAL.
 *
 * @param p_buffer Pointer to the data buffer (512 bytes per block on ESP32).
 * @param first_block Index of the first block to write.
 * @param count Number of consecutive blocks to write.
 * @return K_SPP_OK on success, K_SPP_ERROR on write failure,
 *         or K_SPP_ERROR_NULL_POINTER for an invalid pointer.
 */
SPP_RetVal_t SPP_HAL_STORAGE_write(const void *p_buffer, spp_uint32_t first_block, spp_uint16_t count);

#endif /* SPP_HAL_STORAGE_H */
