/**
 * @file halRp2350.h
 * @brief RP2350 HAL port descriptor for SPP.
 *
 * Provides the platform-specific HAL port for the RP2350 / RP2350B target.
 * Call SPP_PORTS_RP2350_getHalPorts() to obtain a pointer to the static
 * @ref SPP_HalPort_t and pass it to SPP_HAL_init().
 *
 * Naming conventions used in this file:
 * - Public functions: SPP_PORTS_RP2350_*()
 */

#ifndef SPP_PORTS_HAL_RP2350_H
#define SPP_PORTS_HAL_RP2350_H

#include "spp/hal/hal.h"

/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */

/**
 * @brief  Returns a pointer to the RP2350 HAL port descriptor.
 * @return Pointer to the static @ref SPP_HalPort_t.
 */
const SPP_HalPort_t *SPP_PORTS_RP2350_getHalPorts(void);

#endif /* SPP_PORTS_HAL_RP2350_H */
