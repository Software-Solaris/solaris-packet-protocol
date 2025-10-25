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

#include <stdint.h>
#include <stdbool.h>
#include "core/returntypes.h"
#include "core/types.h"

#ifdef __cplusplus
extern "C" {
#endif



/**
 * @brief Create a task
 * 
 * @param task_function Task function pointer
 * @param name Task name
 * @param stack_size Stack size in bytes
 * @param parameters Task parameters
 * @param priority Task priority
 * @param task_handle Pointer to store task handle
 * @return retval_t SPP_OK on success, error code otherwise
 */
SPP_OSAL_retval_t SPP_OSAL_TaskCreate(SPP_OSAL_task_function_t task_function, const char* name,
                         SPP_uint32_t stack_size, void* parameters, SPP_OSAL_priority_t priority,
                         SPP_OSAL_task_handle_t* task_handle);

/**
 * @brief Delete a task
 * 
 * @param task_handle Task handle (NULL for current task)
 * @return retval_t SPP_OK on success, error code otherwise
 */
SPP_OSAL_retval_t SPP_OSAL_TaskDelete(SPP_OSAL_task_handle_t task_handle);

/**
 * @brief Delay task execution
 * 
 * @param delay_ms Delay in milliseconds
 * @return retval_t SPP_OK on success, error code otherwise
 */
SPP_OSAL_retval_t SPP_OSAL_TaskDelay(SPP_uint32_t delay_ms);

/**
 * @brief Suspend a task
 * 
 * @param task_handle Task handle
 * @return retval_t SPP_OK on success, error code otherwise
 */
SPP_OSAL_retval_t SPP_OSAL_TaskSuspend(SPP_OSAL_task_handle_t task_handle);

/**
 * @brief Resume a task
 * 
 * @param task_handle Task handle
 * @return retval_t SPP_OK on success, error code otherwise
 */
SPP_OSAL_retval_t SPP_OSAL_TaskResume(SPP_OSAL_task_handle_t task_handle);

/**
 * @brief Get current task handle
 * 
 * @return SPP_OSAL_task_handle_t Current task handle
 */
SPP_OSAL_task_handle_t SPP_OSAL_TaskGetCurrent(void);

/**
 * @brief Get task state
 * 
 * @param task_handle Task handle
 * @return SPP_OSAL_task_state_t Task state
 */
SPP_OSAL_task_state_t SPP_OSAL_TaskGetState(SPP_OSAL_task_handle_t task_handle);

/**
 * @brief Yield CPU to other tasks
 * 
 * @return retval_t SPP_OK on success, error code otherwise
 */
SPP_OSAL_retval_t SPP_OSAL_TaskYield(void);


/**
 * @brief Periodic delay helper (delay-until).
 * blocks the task until the next period based on the previous wake time.
 * 
 * @param io_previous_wake_tick In/Out: marker of the previous wake time
 * @param period_ms periodo en miliseconds
 * @return SPP_OK everything ok, error code otherwise
 */
SPP_OSAL_retval_t SPP_OSAL_TaskDelayUntil(SPP_uint32_t* const io_previous_wake_tick,
                                          SPP_uint32_t period_ms);

/**
 * @brief Set task priority at runtime.
 * 
 * @param task handle of the task
 * @param priority new priority
 * @return SPP_OK everything ok, error if handle invalid
 */
SPP_OSAL_retval_t SPP_OSAL_TaskPrioritySet(SPP_OSAL_task_handle_t task,
                                           SPP_OSAL_priority_t priority);

/**
 * @brief Get task priority.
 * 
 * @param task handle of the task
 * @param out_priority [out] actual priority
 * @return SPP_OK everything ok, error if handle invalid
 */
SPP_OSAL_retval_t SPP_OSAL_TaskPriorityGet(SPP_OSAL_task_handle_t task,
                                           SPP_OSAL_priority_t* out_priority);



/**
 * @brief Register an idle hook callback.
 * 
 * @param hook Callback to execute during idle time
 * @return SPP_OK if everything okay
 */
SPP_OSAL_retval_t SPP_OSAL_IdleHookRegister(SPP_OSAL_IdleHook hook);

#ifdef __cplusplus
}
#endif

#endif /* SPP_OSAL_TASK_H */
