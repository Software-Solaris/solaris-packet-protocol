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
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief OSAL Task handle type
 */
typedef void* SPP_OSAL_task_handle_t;

/**
 * @brief Task function pointer type
 */
typedef void (*SPP_OSAL_task_function_t)(void* parameters);

/**
 * @brief Task priority levels
 */
typedef enum {
    SPP_OSAL_PRIORITY_IDLE = 0,
    SPP_OSAL_PRIORITY_LOW = 1,
    SPP_OSAL_PRIORITY_NORMAL = 2,
    SPP_OSAL_PRIORITY_HIGH = 3,
    SPP_OSAL_PRIORITY_CRITICAL = 4
} SPP_OSAL_priority_t;

/**
 * @brief Task state enumeration
 */
typedef enum {
    SPP_OSAL_TASK_READY = 0,
    SPP_OSAL_TASK_RUNNING = 1,
    SPP_OSAL_TASK_BLOCKED = 2,
    SPP_OSAL_TASK_SUSPENDED = 3,
    SPP_OSAL_TASK_DELETED = 4
} SPP_OSAL_task_state_t;

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
retval_t SPP_OSAL_TaskCreate(SPP_OSAL_task_function_t task_function, const char* name,
                         uint32_t stack_size, void* parameters, SPP_OSAL_priority_t priority,
                         SPP_OSAL_task_handle_t* task_handle);

/**
 * @brief Delete a task
 * 
 * @param task_handle Task handle (NULL for current task)
 * @return retval_t SPP_OK on success, error code otherwise
 */
retval_t SPP_OSAL_TaskDelete(SPP_OSAL_task_handle_t task_handle);

/**
 * @brief Delay task execution
 * 
 * @param delay_ms Delay in milliseconds
 * @return retval_t SPP_OK on success, error code otherwise
 */
retval_t SPP_OSAL_TaskDelay(uint32_t delay_ms);

/**
 * @brief Suspend a task
 * 
 * @param task_handle Task handle
 * @return retval_t SPP_OK on success, error code otherwise
 */
retval_t SPP_OSAL_TaskSuspend(SPP_OSAL_task_handle_t task_handle);

/**
 * @brief Resume a task
 * 
 * @param task_handle Task handle
 * @return retval_t SPP_OK on success, error code otherwise
 */
retval_t SPP_OSAL_TaskResume(SPP_OSAL_task_handle_t task_handle);

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
retval_t SPP_OSAL_TaskYield(void);

#ifdef __cplusplus
}
#endif

#endif /* SPP_OSAL_TASK_H */
