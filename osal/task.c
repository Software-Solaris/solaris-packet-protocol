/**
 * @file task.c
 * @brief OSAL Task Management Implementation
 * @version 1.0.0
 * @date 2024
 * 
 * This file provides the default (weak) implementation of task management functions.
 */

#include "osal/task.h"
#include "core/returntypes.h"
#include "core/types.h"  
#include <stddef.h>   // define NULL
#include <stdbool.h>


/**
 * @brief Default (weak) task creation
 */
__attribute__((weak)) SppRetVal_t SPP_OSAL_TaskCreate(void* task_function, const char* name,
                    spp_uint32_t stack_size, void* p_parameters, SppPriority_t priority,
                    SppTaskHandle_t* task_handle)
{
    if (task_function == NULL || task_handle == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    *task_handle = (void*)0x12345678;
    (void)name; (void)stack_size; (void)p_parameters; (void)priority;
    return SPP_OK;
}

/**
 * @brief Default (weak) task deletion
 */
__attribute__((weak)) SppRetVal_t
SPP_OSAL_TaskDelete(SppTaskHandle_t task_handle)
{
    (void)task_handle;
    return SPP_OK;
}

/**
 * @brief Default (weak) task delay
 */
__attribute__((weak)) SppRetVal_t
SPP_OSAL_TaskDelay(spp_uint32_t delay_ms)
{
    volatile spp_uint32_t count = delay_ms * 1000u;
    while (count--) { 
         // Busy wait
    }
    return SPP_OK;
}

/**
 * @brief Default (weak) task suspend
 */
__attribute__((weak)) SppRetVal_t
SPP_OSAL_TaskSuspend(SppTaskHandle_t task_handle)
{
    (void)task_handle;
    return SPP_OK;
}

/**
 * @brief Default (weak) "suspend all" (no-op stub)
 */
__attribute__((weak)) SppRetVal_t SPP_OSAL_SuspendAll(void)
{
    return SPP_OK;
}

/**
 * @brief Default (weak) "resume all" (no-op stub)
 */
__attribute__((weak)) SppRetVal_t SPP_OSAL_ResumeAll(void)
{
    return SPP_OK;
}

/**
 * @brief Default (weak) task resume
 */
__attribute__((weak)) SppRetVal_t
SPP_OSAL_TaskResume(SppTaskHandle_t task_handle)
{
    (void)task_handle;
    return SPP_OK;
}

/**
 * @brief Default (weak) get current task
 */
__attribute__((weak)) void*
SPP_OSAL_TaskGetCurrent(void)
{
    return (void*)0x11111111;
}

/**
 * @brief Default (weak) get task state
 */
__attribute__((weak)) SppTaskState
SPP_OSAL_TaskGetState(SppTaskHandle_t task_handle)
{
    (void)task_handle;
    return SPP_OSAL_TASK_RUNNING;
}

/**
 * @brief Default (weak) task yield
 */ 
__attribute__((weak)) SppRetVal_t
SPP_OSAL_TaskYield(void)
{
    return SPP_OK;
}

/**
 * @brief Default (weak) periodic delay (delay-until)
 */
__attribute__((weak)) SppRetVal_t
SPP_OSAL_TaskDelayUntil(spp_uint32_t period_ms)
{
    (void)period_ms;
    return SPP_OK;
}

/**
 * @brief Default (weak) set task priority
 */
__attribute__((weak)) SppRetVal_t
SPP_OSAL_TaskPrioritySet(SppTaskHandle_t task_handle, SppPriority_t priority)
{
    if (task_handle == NULL) return SPP_ERROR_NULL_POINTER;
    (void)priority;
    return SPP_OK;
}

/**
 * @brief Default (weak) get task priority
 */
__attribute__((weak)) SppPriority_t
SPP_OSAL_TaskPriorityGet(SppTaskHandle_t task_handle)
{
    (void)task_handle;
    return SPP_OSAL_PRIORITY_NORMAL;
}

/**
 * @brief Default (weak) idle hook register
 */
__attribute__((weak)) SppRetVal_t
SPP_OSAL_IdleHookRegister(bool (*idle_function)(void))
{
    (void)idle_function;
    return SPP_OK;
}
