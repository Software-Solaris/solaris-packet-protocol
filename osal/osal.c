/**
 * @file osal.c
 * @brief Operating System Abstraction Layer Implementation
 * @version 1.0.0
 * @date 2024
 * 
 * This file provides the default (weak) implementation of OSAL functions
 * that can be overridden by OS-specific implementations.
 */

#include "core/returntypes.h"
#include "osal.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief Default (weak) OSAL initialization
 */
__attribute__((weak)) SppRetVal_t OSAL_Init(void)
{
    // Default implementation - just return OK
    return SPP_OK;
}

/**
 * @brief Default (weak) OSAL deinitialization
 */
__attribute__((weak)) SppRetVal_t OSAL_Deinit(void)
{
    // Default implementation - just return OK
    return SPP_OK;
}

/**
 * @brief Default (weak) task creation
 */
__attribute__((weak)) SppRetVal_t OSAL_TaskCreate(osal_task_function_t task_function, const char* name,
                                              uint32_t stack_size, void* parameters, osal_priority_t priority,
                                              osal_task_handle_t* task_handle)
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
__attribute__((weak)) SppRetVal_t OSAL_TaskDelete(osal_task_handle_t task_handle)
{
    // Default implementation - just return OK
    (void)task_handle;
    return SPP_OK;
}

/**
 * @brief Default (weak) task delay
 */
__attribute__((weak)) SppRetVal_t OSAL_TaskDelay(uint32_t delay_ms)
{
    // Default implementation - busy wait (not recommended for real use)
    volatile uint32_t count = delay_ms * 1000;
    while (count--) {
        // Busy wait
    }
    return SPP_OK;
}

/**
 * @brief Default (weak) mutex creation
 */
__attribute__((weak)) SppRetVal_t OSAL_MutexCreate(osal_mutex_handle_t* mutex_handle)
{
    if (mutex_handle == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
    // Default implementation - simulate mutex creation
    *mutex_handle = (void*)0x87654321;
    return SPP_OK;
}

/**
 * @brief Default (weak) mutex deletion
 */
__attribute__((weak)) SppRetVal_t OSAL_MutexDelete(osal_mutex_handle_t mutex_handle)
{
    // Default implementation - just return OK
    (void)mutex_handle;
    return SPP_OK;
}

/**
 * @brief Default (weak) mutex take
 */
__attribute__((weak)) SppRetVal_t OSAL_MutexTake(osal_mutex_handle_t mutex_handle, uint32_t timeout_ms)
{
    if (mutex_handle == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
    // Default implementation - always succeeds immediately
    (void)timeout_ms;
    return SPP_OK;
}

/**
 * @brief Default (weak) mutex give
 */
__attribute__((weak)) SppRetVal_t OSAL_MutexGive(osal_mutex_handle_t mutex_handle)
{
    if (mutex_handle == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
    // Default implementation - just return OK
    return SPP_OK;
}

/**
 * @brief Default (weak) queue creation
 */
__attribute__((weak)) SppRetVal_t OSAL_QueueCreate(osal_queue_handle_t* queue_handle, uint32_t queue_length, uint32_t item_size)
{
    if (queue_handle == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
    // Default implementation - simulate queue creation
    *queue_handle = (void*)0xABCDEF00;
    (void)queue_length;
    (void)item_size;
    return SPP_OK;
}

/**
 * @brief Default (weak) queue deletion
 */
__attribute__((weak)) SppRetVal_t OSAL_QueueDelete(osal_queue_handle_t queue_handle)
{
    // Default implementation - just return OK
    (void)queue_handle;
    return SPP_OK;
}

/**
 * @brief Default (weak) queue send
 */
__attribute__((weak)) SppRetVal_t OSAL_QueueSend(osal_queue_handle_t queue_handle, const void* item, uint32_t timeout_ms)
{
    if (queue_handle == NULL || item == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
    // Default implementation - always succeeds
    (void)timeout_ms;
    return SPP_OK;
}

/**
 * @brief Default (weak) queue receive
 */
__attribute__((weak)) SppRetVal_t OSAL_QueueReceive(osal_queue_handle_t queue_handle, void* item, uint32_t timeout_ms)
{
    if (queue_handle == NULL || item == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
    // Default implementation - return dummy data
    memset(item, 0xAA, sizeof(uint32_t)); // Assume 4-byte items
    (void)timeout_ms;
    return SPP_OK;
}

/**
 * @brief Default (weak) semaphore creation
 */
__attribute__((weak)) SppRetVal_t OSAL_SemaphoreCreate(osal_semaphore_handle_t* semaphore_handle, uint32_t max_count, uint32_t initial_count)
{
    if (semaphore_handle == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
    // Default implementation - simulate semaphore creation
    *semaphore_handle = (void*)0xFEDCBA00;
    (void)max_count;
    (void)initial_count;
    return SPP_OK;
}

/**
 * @brief Default (weak) semaphore deletion
 */
__attribute__((weak)) SppRetVal_t OSAL_SemaphoreDelete(osal_semaphore_handle_t semaphore_handle)
{
    // Default implementation - just return OK
    (void)semaphore_handle;
    return SPP_OK;
}

/**
 * @brief Default (weak) semaphore take
 */
__attribute__((weak)) SppRetVal_t OSAL_SemaphoreTake(osal_semaphore_handle_t semaphore_handle, uint32_t timeout_ms)
{
    if (semaphore_handle == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
    // Default implementation - always succeeds
    (void)timeout_ms;
    return SPP_OK;
}

/**
 * @brief Default (weak) semaphore give
 */
__attribute__((weak)) SppRetVal_t OSAL_SemaphoreGive(osal_semaphore_handle_t semaphore_handle)
{
    if (semaphore_handle == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
    // Default implementation - just return OK
    return SPP_OK;
}

/**
 * @brief Default (weak) get tick count
 */
__attribute__((weak)) uint32_t OSAL_GetTickCount(void)
{
    // Default implementation - return a dummy value
    static uint32_t tick_count = 0;
    return ++tick_count;
}

/**
 * @brief Default (weak) convert milliseconds to ticks
 */
__attribute__((weak)) uint32_t OSAL_MsToTicks(uint32_t ms)
{
    // Default implementation - assume 1ms = 1 tick
    return ms;
}

/**
 * @brief Default (weak) start scheduler
 */
__attribute__((weak)) SppRetVal_t OSAL_StartScheduler(void)
{
    // Default implementation - just return OK (no scheduler to start)
    return SPP_OK;
} 