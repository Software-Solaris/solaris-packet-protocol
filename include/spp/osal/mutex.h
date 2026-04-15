/**
 * @file mutex.h
 * @brief SPP mutex API — dispatches through the registered OSAL port.
 *
 * Naming conventions used in this file:
 * - Public functions: SPP_Osal_mutex*()
 * - Pointer parameters: p_*
 */

#ifndef SPP_OSAL_MUTEX_H
#define SPP_OSAL_MUTEX_H

#include "spp/core/types.h"
#include "spp/core/returntypes.h"

/* ----------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------- */

/**
 * @brief Create a non-recursive mutex.
 *
 * @return Opaque mutex handle on success, NULL on failure.
 */
void *SPP_Osal_mutexCreate(void);

/**
 * @brief Acquire the mutex.
 *
 * @param[in] p_m        Mutex handle.
 * @param[in] timeoutMs  Maximum wait time in ms; 0 = non-blocking.
 *
 * @return SPP_OK on success, SPP_ERROR on timeout.
 */
retval_t SPP_Osal_mutexLock(void *p_m, spp_uint32_t timeoutMs);

/**
 * @brief Release the mutex.
 *
 * @param[in] p_m  Mutex handle.
 *
 * @return SPP_OK on success.
 */
retval_t SPP_Osal_mutexUnlock(void *p_m);

#endif /* SPP_OSAL_MUTEX_H */
