/**
 * @file task.h
 * @brief OSAL Task Management Interface
 * @version 1.0.0
 * @date 2024
 *
 * This file provides the task management interface for the OSAL layer.
 */

#ifndef SPP_OSAL_TASK_H
#define SPP_OSAL_TASK_H

#include "spp/core/returntypes.h"
#include "spp/core/types.h"
#include "spp/core/macros.h"
#include <stdint.h>

/* ============================================================================
 *  Task Storage
 * ========================================================================= */

/**
 * @brief Get a pointer to the next available task storage slot
 *
 * Returns a pointer to a pre-allocated task storage structure from the
 * static task pool. Used for static task creation.
 *
 * @return void* Pointer to available task storage, NULL if none available
 */
void *SPP_OSAL_GetTaskStorage();

/* ============================================================================
 *  Task Lifecycle
 * ========================================================================= */

/**
 * @brief Create a new OSAL task
 *
 * Creates a new task with the specified parameters and assigns it to the
 * provided storage slot.
 *
 * @param[in] p_function    Pointer to the task entry function
 * @param[in] task_name     Null-terminated string with the task name
 * @param[in] stack_depth   Stack size in bytes for the new task
 * @param[in] p_custom_data Pointer to user-defined data passed to the task
 * @param[in] priority      Task priority level
 * @param[in] p_storage     Pointer to pre-allocated task storage
 *
 * @return void* Handle to the created task, NULL on failure
 */
void *SPP_OSAL_TaskCreate(void *p_function, const char *const task_name, const uint32_t stack_depth,
                          void *const p_custom_data, spp_uint32_t priority, void *p_storage);

/**
 * @brief Delete an existing OSAL task
 *
 * Deletes the task identified by the given handle. If p_task is NULL,
 * the calling task deletes itself.
 *
 * @param[in] p_task Handle of the task to delete, or NULL for self-deletion
 *
 * @return retval_t SPP_OK on success, SPP_ERROR on failure
 */
retval_t SPP_OSAL_TaskDelete(void *p_task);

/**
 * @brief Create a delay
 *
 * @param blocktime_ms Time of the delay in milliseconds
 * @return void
 */
void SPP_OSAL_TaskDelay(spp_uint32_t blocktime_ms);


#endif /* SPP_OSAL_TASK_H */
