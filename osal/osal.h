/**
 * @file osal.h
 * @brief Operating System Abstraction Layer
 * @version 1.0.0
 * @date 2024
 * 
 * This file provides the main Operating System Abstraction Layer (OSAL) interface
 * for the Solaris Packet Protocol library.
 */

#ifndef SPP_OSAL_H
#define SPP_OSAL_H

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
 * @brief OSAL Mutex handle type
 */
typedef void* osal_mutex_handle_t;

/**
 * @brief OSAL Queue handle type
 */
typedef void* osal_queue_handle_t;

/**
 * @brief OSAL Semaphore handle type
 */
typedef void* osal_semaphore_handle_t;

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
 * @brief Initialize OSAL
 * 
 * @return SppRetVal_t SPP_OK on success, error code otherwise
 */
SppRetVal_t OSAL_Init(void);

/**
 * @brief Deinitialize OSAL
 * 
 * @return SppRetVal_t SPP_OK on success, error code otherwise
 */
SppRetVal_t OSAL_Deinit(void);

/**
 * @brief Create a task
 * 
 * @param task_function Task function pointer
 * @param name Task name
 * @param stack_size Stack size in bytes
 * @param parameters Task parameters
 * @param priority Task priority
 * @param task_handle Pointer to store task handle
 * @return SppRetVal_t SPP_OK on success, error code otherwise
 */
SppRetVal_t OSAL_TaskCreate(osal_task_function_t task_function, const char* name,
                         uint32_t stack_size, void* parameters, osal_priority_t priority,
                         osal_task_handle_t* task_handle);

/**
 * @brief Delete a task
 * 
 * @param task_handle Task handle (NULL for current task)
 * @return SppRetVal_t SPP_OK on success, error code otherwise
 */
SppRetVal_t OSAL_TaskDelete(osal_task_handle_t task_handle);

/**
 * @brief Delay task execution
 * 
 * @param delay_ms Delay in milliseconds
 * @return SppRetVal_t SPP_OK on success, error code otherwise
 */
SppRetVal_t OSAL_TaskDelay(uint32_t delay_ms);

/**
 * @brief Create a mutex
 * 
 * @param mutex_handle Pointer to store mutex handle
 * @return SppRetVal_t SPP_OK on success, error code otherwise
 */
SppRetVal_t OSAL_MutexCreate(osal_mutex_handle_t* mutex_handle);

/**
 * @brief Delete a mutex
 * 
 * @param mutex_handle Mutex handle
 * @return SppRetVal_t SPP_OK on success, error code otherwise
 */
SppRetVal_t OSAL_MutexDelete(osal_mutex_handle_t mutex_handle);

/**
 * @brief Take (lock) a mutex
 * 
 * @param mutex_handle Mutex handle
 * @param timeout_ms Timeout in milliseconds (0 = no wait, UINT32_MAX = wait forever)
 * @return SppRetVal_t SPP_OK on success, error code otherwise
 */
SppRetVal_t OSAL_MutexTake(osal_mutex_handle_t mutex_handle, uint32_t timeout_ms);

/**
 * @brief Give (unlock) a mutex
 * 
 * @param mutex_handle Mutex handle
 * @return SppRetVal_t SPP_OK on success, error code otherwise
 */
SppRetVal_t OSAL_MutexGive(osal_mutex_handle_t mutex_handle);

/**
 * @brief Create a queue
 * 
 * @param queue_handle Pointer to store queue handle
 * @param queue_length Maximum number of items in queue
 * @param item_size Size of each item in bytes
 * @return SppRetVal_t SPP_OK on success, error code otherwise
 */
SppRetVal_t OSAL_QueueCreate(osal_queue_handle_t* queue_handle, uint32_t queue_length, uint32_t item_size);

/**
 * @brief Delete a queue
 * 
 * @param queue_handle Queue handle
 * @return SppRetVal_t SPP_OK on success, error code otherwise
 */
SppRetVal_t OSAL_QueueDelete(osal_queue_handle_t queue_handle);

/**
 * @brief Send item to queue
 * 
 * @param queue_handle Queue handle
 * @param item Pointer to item to send
 * @param timeout_ms Timeout in milliseconds
 * @return SppRetVal_t SPP_OK on success, error code otherwise
 */
SppRetVal_t OSAL_QueueSend(osal_queue_handle_t queue_handle, const void* item, uint32_t timeout_ms);

/**
 * @brief Receive item from queue
 * 
 * @param queue_handle Queue handle
 * @param item Pointer to buffer for received item
 * @param timeout_ms Timeout in milliseconds
 * @return SppRetVal_t SPP_OK on success, error code otherwise
 */
SppRetVal_t OSAL_QueueReceive(osal_queue_handle_t queue_handle, void* item, uint32_t timeout_ms);

/**
 * @brief Create a semaphore
 * 
 * @param semaphore_handle Pointer to store semaphore handle
 * @param max_count Maximum count value
 * @param initial_count Initial count value
 * @return SppRetVal_t SPP_OK on success, error code otherwise
 */
SppRetVal_t OSAL_SemaphoreCreate(osal_semaphore_handle_t* semaphore_handle, uint32_t max_count, uint32_t initial_count);

/**
 * @brief Delete a semaphore
 * 
 * @param semaphore_handle Semaphore handle
 * @return SppRetVal_t SPP_OK on success, error code otherwise
 */
SppRetVal_t OSAL_SemaphoreDelete(osal_semaphore_handle_t semaphore_handle);

/**
 * @brief Take a semaphore
 * 
 * @param semaphore_handle Semaphore handle
 * @param timeout_ms Timeout in milliseconds
 * @return SppRetVal_t SPP_OK on success, error code otherwise
 */
SppRetVal_t OSAL_SemaphoreTake(osal_semaphore_handle_t semaphore_handle, uint32_t timeout_ms);

/**
 * @brief Give a semaphore
 * 
 * @param semaphore_handle Semaphore handle
 * @return SppRetVal_t SPP_OK on success, error code otherwise
 */
SppRetVal_t OSAL_SemaphoreGive(osal_semaphore_handle_t semaphore_handle);

/**
 * @brief Get system tick count
 * 
 * @return uint32_t Current tick count
 */
uint32_t OSAL_GetTickCount(void);

/**
 * @brief Convert milliseconds to ticks
 * 
 * @param ms Milliseconds
 * @return uint32_t Equivalent ticks
 */
uint32_t OSAL_MsToTicks(uint32_t ms);

/**
 * @brief Start the RTOS scheduler
 * 
 * @return SppRetVal_t SPP_OK on success, error code otherwise
 */
SppRetVal_t OSAL_StartScheduler(void);

#ifdef __cplusplus
}
#endif

#endif /* SPP_OSAL_H */ 