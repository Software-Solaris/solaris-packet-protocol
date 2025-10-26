/**
 * @file mutex.h
 * @brief OSAL Mutex Management Interface
 * @version 1.0.0
 * @date 2024
 * 
 * This file provides the mutex management interface for the OSAL layer.
 */

#ifndef SPP_OSAL_MUTEX_H
#define SPP_OSAL_MUTEX_H

#include <stdint.h>
#include <stdbool.h>
#include "core/returntypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief OSAL Mutex handle type
 */
typedef void* osal_mutex_handle_t;

/**
 * @brief Mutex type enumeration
 */
typedef enum {
    OSAL_MUTEX_NORMAL = 0,
    OSAL_MUTEX_RECURSIVE = 1
} osal_mutex_type_t;

/**
 * @brief Create a mutex
 * 
 * @param mutex_handle Pointer to store mutex handle
 * @param type Mutex type (normal or recursive)
 * @return SppRetVal_t SPP_OK on success, error code otherwise
 */
SppRetVal_t OSAL_MutexCreate(osal_mutex_handle_t* mutex_handle, osal_mutex_type_t type);

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
 * @brief Try to take a mutex without blocking
 * 
 * @param mutex_handle Mutex handle
 * @return SppRetVal_t SPP_OK on success, SPP_ERROR_TIMEOUT if mutex not available
 */
SppRetVal_t OSAL_MutexTryTake(osal_mutex_handle_t mutex_handle);

/**
 * @brief Get mutex holder task
 * 
 * @param mutex_handle Mutex handle
 * @return void* Task handle that currently holds the mutex (NULL if not held)
 */
void* OSAL_MutexGetHolder(osal_mutex_handle_t mutex_handle);

/**
 * @brief Check if mutex is held by current task
 * 
 * @param mutex_handle Mutex handle
 * @return bool true if mutex is held by current task, false otherwise
 */
bool OSAL_MutexIsHeldByCurrentTask(osal_mutex_handle_t mutex_handle);

#ifdef __cplusplus
}
#endif

#endif /* SPP_OSAL_MUTEX_H */ 