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

SPP_RetVal_t SPP_HAL_STORAGE_init(void);
SPP_RetVal_t SPP_HAL_STORAGE_write(const void *p_buffer, spp_uint32_t first_block,
                                   spp_uint16_t count); // size = 512 x count

#endif /* SPP_HAL_STORAGE_H */
