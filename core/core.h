/**
 * @file core.h
 * @brief SPP core initialisation and port registration API.
 *
 * Typical startup:
 *  1. @ref SPP_CORE_boot()         — register port, init subsystems, wire log→pub/sub
 *  2. @ref SPP_SERVICES_register() — register application services
 *
 * Naming conventions used in this file:
 * - Public functions: SPP_CORE_*()
 * - Pointer parameters: p_*
 */

#ifndef SPP_CORE_H
#define SPP_CORE_H

#include "spp/core/returnTypes.h"
#include "spp/hal/port.h"

/* ----------------------------------------------------------------
 * Boot
 * ---------------------------------------------------------------- */

/**
 * @brief Register the HAL port, initialise core subsystems, and wire log output.
 *
 * Single call that replaces the old setHalPort() + init() + registerOutput()
 * sequence.  After this returns, SPP_LOG* macros print to stdout and also
 * publish K_SPP_APID_LOG packets through pub/sub.
 *
 * @param[in] p_port  Pointer to the populated @ref SPP_HalPort_t.
 *
 * @return K_SPP_OK on success, or an error code on failure.
 */
SPP_RetVal_t SPP_CORE_boot(const SPP_HalPort_t *p_port);

/* ----------------------------------------------------------------
 * Lower-level API (available if you need finer control)
 * ---------------------------------------------------------------- */

SPP_RetVal_t         SPP_CORE_setHalPort(const SPP_HalPort_t *p_port);
SPP_RetVal_t         SPP_CORE_init(void);
const SPP_HalPort_t *SPP_CORE_getHalPort(void);

#endif /* SPP_CORE_H */
