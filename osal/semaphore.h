/**
 * @file semaphore.h
 * @brief OSAL Semaphore Management Interface
 * @version 1.0.0
 * @date 2024
 * 
 * This file provides the semaphore management interface for the OSAL layer.
 */

#ifndef SPP_OSAL_SEMAPHORE_H
#define SPP_OSAL_SEMAPHORE_H

#include <stdint.h>
#include <stdbool.h>
#include "core/returntypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief OSAL Semaphore handle type
 */
typedef void* osal_semaphore_handle_t;

/**
 * @brief Create a counting semaphore
 * 
 * @param semaphore_handle Pointer to store semaphore handle
 * @param max_count Maximum count value
 * @param initial_count Initial count value
 * @return SppRetVal_t SPP_OK on success, error code otherwise
 */
SppRetVal_t OSAL_SemaphoreCreate(osal_semaphore_handle_t* semaphore_handle, uint32_t max_count, uint32_t initial_count);

/**
 * @brief Create a binary semaphore
 * 
 * @param semaphore_handle Pointer to store semaphore handle
 * @return SppRetVal_t SPP_OK on success, error code otherwise
 */
SppRetVal_t OSAL_SemaphoreCreateBinary(osal_semaphore_handle_t* semaphore_handle);

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
 * @brief Give a semaphore from ISR
 * 
 * @param semaphore_handle Semaphore handle
 * @param higher_priority_task_woken Pointer to flag indicating if higher priority task was woken
 * @return SppRetVal_t SPP_OK on success, error code otherwise
 */
SppRetVal_t OSAL_SemaphoreGiveFromISR(osal_semaphore_handle_t semaphore_handle, bool* higher_priority_task_woken);

/**
 * @brief Try to take a semaphore without blocking
 * 
 * @param semaphore_handle Semaphore handle
 * @return SppRetVal_t SPP_OK on success, SPP_ERROR_TIMEOUT if semaphore not available
 */
SppRetVal_t OSAL_SemaphoreTryTake(osal_semaphore_handle_t semaphore_handle);

/**
 * @brief Get current semaphore count
 * 
 * @param semaphore_handle Semaphore handle
 * @return uint32_t Current semaphore count
 */
uint32_t OSAL_SemaphoreGetCount(osal_semaphore_handle_t semaphore_handle);

#ifdef __cplusplus
}
#endif

#endif /* SPP_OSAL_SEMAPHORE_H */ 