/**
 * @file core.h
 * @brief SPP core initialisation and port registration API.
 *
 * Call order at startup:
 *  1. @ref SPP_CORE_setHalPort()   — register your hardware port
 *  2. @ref SPP_CORE_init()         — initialise logging and databank
 *  3. @ref SPP_SERVICES_register()  — register application services
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
 * Port registration
 * ---------------------------------------------------------------- */

/**
 * @brief Register the HAL port for the current hardware.
 *
 * Must be called before @ref SPP_CORE_init().  The pointer must remain valid
 * for the lifetime of the application.
 *
 * @param[in] p_port  Pointer to the populated @ref SPP_HalPort_t.
 *
 * @return K_SPP_OK on success, K_SPP_ERROR_NULL_POINTER if @p p_port is NULL.
 */
SPP_RetVal_t SPP_CORE_setHalPort(const SPP_HalPort_t *p_port);

/* ----------------------------------------------------------------
 * Core initialisation
 * ---------------------------------------------------------------- */

/**
 * @brief Initialise the SPP core subsystems.
 *
 * Initialises the logging service and the packet databank.  The HAL port
 * must be registered before calling this function.
 *
 * @return K_SPP_OK on success.
 * @return K_SPP_ERROR_NOT_INITIALIZED if the HAL port has not been registered.
 */
SPP_RetVal_t SPP_CORE_init(void);

/**
 * @brief Return a pointer to the currently registered HAL port.
 *
 * Used internally by HAL dispatch functions.  Exposed for testing.
 *
 * @return Pointer to the registered @ref SPP_HalPort_t, or NULL.
 */
const SPP_HalPort_t *SPP_CORE_getHalPort(void);

#endif /* SPP_CORE_H */
