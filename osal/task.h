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

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief OSAL Task handle type
 */
typedef void* osal_task_handle_t;

/**
 * @brief Task function pointer type
 */
typedef void (*osal_task_function_t)(void* parameters);

/**
 * @brief Task priority levels
 */
typedef enum {
    OSAL_PRIORITY_IDLE = 0,
    OSAL_PRIORITY_LOW = 1,
    OSAL_PRIORITY_NORMAL = 2,
    OSAL_PRIORITY_HIGH = 3,
    OSAL_PRIORITY_CRITICAL = 4
} osal_priority_t;

/**
 * @brief Task state enumeration
 */
typedef enum {
    OSAL_TASK_READY = 0,
    OSAL_TASK_RUNNING = 1,
    OSAL_TASK_BLOCKED = 2,
    OSAL_TASK_SUSPENDED = 3,
    OSAL_TASK_DELETED = 4
} osal_task_state_t;

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
retval_t SPP_OSAL_TaskCreate(osal_task_function_t task_function, const char* name,
                         uint32_t stack_size, void* parameters, osal_priority_t priority,
                         osal_task_handle_t* task_handle);

/**
 * @brief Delete a task
 * 
 * @param task_handle Task handle (NULL for current task)
 * @return retval_t SPP_OK on success, error code otherwise
 */
retval_t SPP_OSAL_TaskDelete(osal_task_handle_t task_handle);

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
retval_t OSAL_TaskSuspend(osal_task_handle_t task_handle);

/**
 * @brief Resume a task
 * 
 * @param task_handle Task handle
 * @return retval_t SPP_OK on success, error code otherwise
 */
retval_t OSAL_TaskResume(osal_task_handle_t task_handle);

/**
 * @brief Get current task handle
 * 
 * @return osal_task_handle_t Current task handle
 */
osal_task_handle_t OSAL_TaskGetCurrent(void);

/**
 * @brief Get task state
 * 
 * @param task_handle Task handle
 * @return osal_task_state_t Task state
 */
osal_task_state_t OSAL_TaskGetState(osal_task_handle_t task_handle);

/**
 * @brief Yield CPU to other tasks
 * 
 * @return retval_t SPP_OK on success, error code otherwise
 */
retval_t OSAL_TaskYield(void);

#ifdef __cplusplus
}
#endif

#endif /* SPP_OSAL_TASK_H */ 