/**
 * @file timePort.h
 * @brief ESP32-S3 time port implementation.
 */

#ifndef SPP_PORTS_HAL_ESP32_TIME_PORT_H
#define SPP_PORTS_HAL_ESP32_TIME_PORT_H

#include "spp/core/types.h"
#include "spp/core/returnTypes.h"

/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */

spp_uint32_t SPP_PORTS_HAL_ESP32_getTimeMs(void);
spp_uint32_t SPP_PORTS_HAL_ESP32_getTimeUs(void);
void SPP_PORTS_HAL_ESP32_delayMs(spp_uint32_t ms);

#endif /* SPP_PORTS_HAL_ESP32_TIME_PORT_H */
