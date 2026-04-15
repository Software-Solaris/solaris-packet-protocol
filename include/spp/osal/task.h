/**
 * @file task.h
 * @brief SPP task management API — dispatches through the registered OSAL port.
 *
 * Naming conventions used in this file:
 * - Public functions: SPP_Osal_task*()
 * - Pointer parameters: p_*
 */

#ifndef SPP_OSAL_TASK_H
#define SPP_OSAL_TASK_H

#include "spp/core/types.h"
#include "spp/core/returntypes.h"

/* ----------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------- */

/**
 * @brief Create an OS task.
 *
 * @param[in] p_fn        Task entry function (must not return).
 * @param[in] p_name      Human-readable name for debugging.
 * @param[in] stackWords  Stack allocation in words.
 * @param[in] p_arg       Argument forwarded to @p p_fn.
 * @param[in] prio        Task priority (use K_SPP_OSAL_PRIO_* values).
 *
 * @return Opaque task handle on success, NULL on failure.
 */
void *SPP_Osal_taskCreate(void (*p_fn)(void *), const char *p_name,
                           spp_uint32_t stackWords, void *p_arg,
                           spp_uint32_t prio);

/**
 * @brief Delete a task.  Pass NULL to delete the calling task.
 *
 * @param[in] p_handle  Task handle or NULL.
 */
void SPP_Osal_taskDelete(void *p_handle);

/**
 * @brief Delay the calling task for at least @p ms milliseconds.
 *
 * @param[in] ms  Delay duration in milliseconds.
 */
void SPP_Osal_taskDelayMs(spp_uint32_t ms);

/**
 * @brief Return the current system tick in milliseconds.
 *
 * @return Monotonic elapsed time in ms.
 */
spp_uint32_t SPP_Osal_getTickMs(void);

#endif /* SPP_OSAL_TASK_H */
