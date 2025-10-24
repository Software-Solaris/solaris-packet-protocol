/**
 * @file task.c
 * @brief OSAL Task Management Implementation
 * @version 1.0.0
 * @date 2024
 * 
 * This file provides the default (weak) implementation of task management functions.
 */

#include "task.h"
#include "core/returntypes.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief Default (weak) task creation
 */
__attribute__((weak)) retval_t SPP_OSAL_TaskCreate(SPP_OSAL_task_function_t task_function, const char* name,
                                              uint32_t stack_size, void* parameters, SPP_OSAL_priority_t priority,
                                              SPP_OSAL_task_handle_t* task_handle)
{
    if (task_function == NULL || task_handle == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
    // Default implementation - simulate task creation
    *task_handle = (void*)0x12345678;
    (void)name;
    (void)stack_size;
    (void)parameters;
    (void)priority;
    
    return SPP_OK;
}

/**
 * @brief Default (weak) task deletion
 */
__attribute__((weak)) retval_t SPP_OSAL_TaskDelete(SPP_OSAL_task_handle_t task_handle)
{
    // Default implementation - just return OK
    (void)task_handle;
    return SPP_OK;
}

/**
 * @brief Default (weak) task delay
 */
__attribute__((weak)) retval_t SPP_OSAL_TaskDelay(uint32_t delay_ms)
{
    // Default implementation - busy wait (not recommended for real use)
    volatile uint32_t count = delay_ms * 1000;
    while (count--) {
        // Busy wait
    }
    return SPP_OK;
}

/**
 * @brief Default (weak) task suspend
 */
__attribute__((weak)) retval_t SPP_OSAL_TaskSuspend(SPP_OSAL_task_handle_t task_handle)
{
    // Default implementation - just return OK
    (void)task_handle;
    return SPP_OK;
}

/**
 * @brief Default (weak) task resume
 */
__attribute__((weak)) retval_t SPP_OSAL_TaskResume(SPP_OSAL_task_handle_t task_handle)
{
    // Default implementation - just return OK
    (void)task_handle;
    return SPP_OK;
}

/**
 * @brief Default (weak) get current task
 */
__attribute__((weak)) SPP_OSAL_task_handle_t SPP_OSAL_TaskGetCurrent(void)
{
    // Default implementation - return dummy handle
    return (void*)0x11111111;
}

/**
 * @brief Default (weak) get task state
 */
__attribute__((weak)) SPP_OSAL_task_state_t SPP_OSAL_TaskGetState(SPP_OSAL_task_handle_t task_handle)
{
    // Default implementation - always return running
    (void)task_handle;
    return SPP_OSAL_TASK_RUNNING;
}

/**
 * @brief Default (weak) task yield
 */
__attribute__((weak)) retval_t SPP_OSAL_TaskYield(void)
{
    // Default implementation - just return OK
    return SPP_OK;
}
