// Aqui van los weaks de las funciones que crees en queues.c




/**
 * @file queue.c
 * @brief OSAL Queue Management Implementation
 * @version 1.0.0
 * @date 2024
 * 
 * This file provides the default (weak) implementation of queue management functions.
 */

#include "queue.h"
#include "core/returntypes.h"
#include "types.h"
#include <stdlib.h>
#include <string.h>

/* 
__attribute__((weak)) retval_t OSAL_QueueCreate(osal_queue_handle_t* queue_handle, uint32_t queue_length, uint32_t item_size)
{
    if (queue_handle == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
    // Default implementation - simulate queue creation
    *queue_handle = (void*)0xABCDEF00;
    (void)queue_length;
    (void)item_size;
    return SPP_OK;
} */


/**
 * @brief Default (weak) queue creation
 */
__attribute__((weak)) void* SPP_OSAL_QueueCreate(uint32_t queue_length, uint32_t item_size)
{
    (void)queue_length;
    (void)item_size;
    return NULL; // Esto da error???
}

/**
 * @brief Default (weak) queue creation
 */
__attribute__((weak)) void* SPP_OSAL_QueueCreateStatic(uint32_t queue_length, uint32_t item_size, uint8_t* queue_storage, void* queue_buffer)
{
    (void)queue_length;
    (void)item_size;
    (void)queue_storage;
    (void)queue_buffer;
    return NULL; // Esto da error???
}

/**
 * @brief Default (weak) messages waiting
 */
__attribute__((weak)) uint32_t SPP_OSAL_QueueMessagesWaiting(void* queue_handle)
{
    (void)queue_handle;
    return 0; // Esto da error???
}

/**
 * @brief Default (weak) queue deletion
 */
__attribute__((weak)) retval_t OSAL_QueueDelete(osal_queue_handle_t queue_handle)
{
    // Default implementation - just return OK
    (void)queue_handle;
    return SPP_OK;
}

/**
 * @brief Default (weak) queue send
 */
__attribute__((weak)) retval_t OSAL_QueueSend(osal_queue_handle_t queue_handle, const void* item, uint32_t timeout_ms)
{
    if (queue_handle == NULL || item == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
    // Default implementation - always succeeds
    (void)timeout_ms;
    return SPP_OK;
}

/**
 * @brief Default (weak) queue send from ISR
 */
__attribute__((weak)) retval_t OSAL_QueueSendFromISR(osal_queue_handle_t queue_handle, const void* item, bool* higher_priority_task_woken)
{
    if (queue_handle == NULL || item == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
    // Default implementation - always succeeds
    if (higher_priority_task_woken != NULL) {
        *higher_priority_task_woken = false;
    }
    return SPP_OK;
}

/**
 * @brief Default (weak) queue receive
 */
__attribute__((weak)) retval_t OSAL_QueueReceive(osal_queue_handle_t queue_handle, void* item, uint32_t timeout_ms)
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
 * @brief Default (weak) queue receive from ISR
 */
__attribute__((weak)) retval_t OSAL_QueueReceiveFromISR(osal_queue_handle_t queue_handle, void* item, bool* higher_priority_task_woken)
{
    if (queue_handle == NULL || item == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
    // Default implementation - return dummy data
    memset(item, 0xBB, sizeof(uint32_t)); // Assume 4-byte items
    if (higher_priority_task_woken != NULL) {
        *higher_priority_task_woken = false;
    }
    return SPP_OK;
}

/**
 * @brief Default (weak) queue peek
 */
__attribute__((weak)) retval_t OSAL_QueuePeek(osal_queue_handle_t queue_handle, void* item, uint32_t timeout_ms)
{
    if (queue_handle == NULL || item == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
    // Default implementation - return dummy data
    memset(item, 0xCC, sizeof(uint32_t)); // Assume 4-byte items
    (void)timeout_ms;
    return SPP_OK;
}

/**
 * @brief Default (weak) get queue count
 */
__attribute__((weak)) uint32_t OSAL_QueueGetCount(osal_queue_handle_t queue_handle)
{
    // Default implementation - return dummy count
    (void)queue_handle;
    return 5; // Simulate 5 items in queue
}

/**
 * @brief Default (weak) get queue space
 */
__attribute__((weak)) uint32_t OSAL_QueueGetSpace(osal_queue_handle_t queue_handle)
{
    // Default implementation - return dummy space
    (void)queue_handle;
    return 10; // Simulate 10 free spaces
}

/**
 * @brief Default (weak) check if queue is full
 */
__attribute__((weak)) bool OSAL_QueueIsFull(osal_queue_handle_t queue_handle)
{
    // Default implementation - never full
    (void)queue_handle;
    return false;
}

/**
 * @brief Default (weak) check if queue is empty
 */
__attribute__((weak)) bool OSAL_QueueIsEmpty(osal_queue_handle_t queue_handle)
{
    // Default implementation - never empty
    (void)queue_handle;
    return false;
}

/**
 * @brief Default (weak) reset queue
 */
__attribute__((weak)) retval_t OSAL_QueueReset(osal_queue_handle_t queue_handle)
{
    // Default implementation - just return OK
    (void)queue_handle;
    return SPP_OK;
} 