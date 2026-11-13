/**
 * @file storagePort.h
 * @brief ESP32-S3 SD card storage port implementation.
 */

#ifndef SPP_PORTS_HAL_ESP32_STORAGE_PORT_H
#define SPP_PORTS_HAL_ESP32_STORAGE_PORT_H

#include "spp/core/types.h"
#include "spp/core/returnTypes.h"

/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */

/**
 * @brief Initializes the SD card using SDSPI.
 *
 * Requires the SPI bus to be initialized.
 * Attempts SD card initialization up to five times.
 *
 * @return K_SPP_OK on success, K_SPP_ERROR otherwise.
 */
SPP_RetVal_t SPP_PORTS_HAL_ESP32_storageInit(void);

/**
 * @brief Writes raw 512-byte sectors directly to the SD card.
 *
 * @param p_buffer Pointer to the data to write.
 * @param first_block First sector to write.
 * @param count Number of consecutive sectors.
 * @return K_SPP_OK on success, K_SPP_ERROR_NULL_POINTER
 *         for a NULL buffer, K_SPP_ERROR otherwise.
 */
SPP_RetVal_t SPP_PORTS_HAL_ESP32_storageWrite(const void *p_buffer, spp_uint32_t first_block, spp_uint16_t count);

#endif /* SPP_PORTS_HAL_ESP32_STORAGE_PORT_H */
