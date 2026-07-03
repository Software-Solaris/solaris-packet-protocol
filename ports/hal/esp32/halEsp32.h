/**
 * @file halEsp32.h
 * @brief ESP32-S3 HAL port descriptor for SPP.
 *
 * Provides the platform-specific HAL port for the ESP32-S3 target.
 * Call SPP_PORTS_ESP32S3_getHalPorts() to obtain a pointer to the static
 * @ref SPP_HalPort_t and pass it to SPP_HAL_init().
 *
 * Naming conventions used in this file:
 * - Public functions: SPP_PORTS_ESP32S3_*()
 */

#ifndef SPP_PORTS_HAL_ESP32_H
#define SPP_PORTS_HAL_ESP32_H

#include "spp/hal/hal.h"

/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */

/**
 * @brief  Returns a pointer to the ESP32-S3 HAL port descriptor.
 * @return Pointer to the static @ref SPP_HalPort_t.
 */
const SPP_HalPort_t *SPP_PORTS_ESP32S3_getHalPorts(void);

#endif /* SPP_PORTS_HAL_ESP32_H */
