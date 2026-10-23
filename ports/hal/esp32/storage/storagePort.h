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

SPP_RetVal_t SPP_PORTS_HAL_ESP32_storageInit(void);
SPP_RetVal_t SPP_PORTS_HAL_ESP32_storageWrite(const void *p_buffer, spp_uint32_t first_block,
                                               spp_uint16_t count);

#endif /* SPP_PORTS_HAL_ESP32_STORAGE_PORT_H */
