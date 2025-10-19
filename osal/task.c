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
__attribute__((weak)) retval_t OSAL_TaskCreate(spp_task_handle_t *handle, const spp_task_attr_t *attr)

{
    return SPP_ERROR;
}

/**
 * @brief Default (weak) task deletion
 */
__attribute__((weak)) retval_t OSAL_TaskDelete(osal_task_handle_t task_handle)
{
    // Default implementation - just return OK
    (void)task_handle;
    return SPP_OK;
}

/**
 * @brief Default (weak) task delay
 */
__attribute__((weak)) retval_t OSAL_TaskDelay(uint32_t delay_ms)
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
__attribute__((weak)) retval_t OSAL_TaskSuspend(osal_task_handle_t task_handle)
{
    // Default implementation - just return OK
    (void)task_handle;
    return SPP_OK;
}

/**
 * @brief Default (weak) task resume
 */
__attribute__((weak)) retval_t OSAL_TaskResume(osal_task_handle_t task_handle)
{
    // Default implementation - just return OK
    (void)task_handle;
    return SPP_OK;
}

/**
 * @brief Default (weak) get current task
 */
__attribute__((weak)) osal_task_handle_t OSAL_TaskGetCurrent(void)
{
    // Default implementation - return dummy handle
    return (void*)0x11111111;
}

/**
 * @brief Default (weak) get task state
 */
__attribute__((weak)) osal_task_state_t OSAL_TaskGetState(osal_task_handle_t task_handle)
{
    // Default implementation - always return running
    (void)task_handle;
    return OSAL_TASK_RUNNING;
}

/**
 * @brief Default (weak) task yield
 */
__attribute__((weak)) retval_t OSAL_TaskYield(void)
{
    // Default implementation - just return OK
    return SPP_OK;
} 