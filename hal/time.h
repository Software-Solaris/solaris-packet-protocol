/**
 * @file time.h
 * @brief SPP HAL time API — dispatches through the registered HAL port.
 *
 * Naming conventions used in this file:
 * - Public functions: SPP_HAL_*()
 */

#ifndef SPP_HAL_TIME_H
#define SPP_HAL_TIME_H

#include "spp/core/types.h"

/* ----------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------- */

/**
 * @brief Return the current hardware time in milliseconds.
 *
 * @return Elapsed time in ms (monotonically non-decreasing).
 */
spp_uint32_t SPP_HAL_getTimeMs(void);

/**
 * @brief Block for the requested number of milliseconds.
 *
 * @param[in] ms  Number of milliseconds to delay.
 */
void SPP_HAL_delayMs(spp_uint32_t ms);

#endif /* SPP_HAL_TIME_H */
