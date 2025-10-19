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
 * @brief Task parameters
 */
typedef struct {
    const char *name;
    spp_task_func_t entry;
    void *arg;
    unsigned int stack_size;
    unsigned int priority;
    int core;   // -1 for any core
} spp_task_attr_t;

/**
 * @brief Create a new task/thread in the underlying RTOS.
 *
 * This function creates a task using the attributes provided in @p attr. 
 * The implementation depends on the RTOS in use (e.g., FreeRTOS, Zephyr, ThreadX).
 * 
 * @param[out] handle   Pointer to a task handle where the created task identifier will be stored.
 * @param[in]  attr     Pointer to a structure containing the task attributes:
 *                      - name:       Human-readable task name (for debugging).
 *                      - entry:      Function pointer to the task entry function.
 *                      - arg:        Parameter passed to the task function.
 *                      - stack_size: Stack size in bytes.
 *                      - priority:   Task priority (higher value = higher priority).
 *                      - core:       Core affinity (-1 for any, or specific core number if supported).
 *
 * @return retval_t
 *         - SPP_OK if the task was created successfully.
 *         - Error code otherwise.
 */
retval_t OSAL_TaskCreate(spp_task_handle_t *handle, const spp_task_attr_t *attr);


/**
 * @brief Delete a task
 * 
 * @param task_handle Task handle (NULL for current task)
 * @return retval_t SPP_OK on success, error code otherwise
 */
retval_t OSAL_TaskDelete(osal_task_handle_t task_handle);

/**
 * @brief Delay task execution
 * 
 * @param delay_ms Delay in milliseconds
 * @return retval_t SPP_OK on success, error code otherwise
 */
retval_t OSAL_TaskDelay(uint32_t delay_ms);

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