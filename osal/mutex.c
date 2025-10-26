/**
 * @file mutex.c
 * @brief OSAL Mutex Management Implementation
 * @version 1.0.0
 * @date 2024
 * 
 * This file provides the default (weak) implementation of mutex management functions.
 */

#include "mutex.h"
#include "core/returntypes.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief Default (weak) mutex creation
 */
__attribute__((weak)) SppRetVal_t OSAL_MutexCreate(osal_mutex_handle_t* mutex_handle, osal_mutex_type_t type)
{
    if (mutex_handle == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
    // Default implementation - simulate mutex creation
    *mutex_handle = (void*)0x87654321;
    (void)type;
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
 * @brief Default (weak) mutex try take
 */
__attribute__((weak)) SppRetVal_t OSAL_MutexTryTake(osal_mutex_handle_t mutex_handle)
{
    if (mutex_handle == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
    // Default implementation - always succeeds
    return SPP_OK;
}

/**
 * @brief Default (weak) get mutex holder
 */
__attribute__((weak)) void* OSAL_MutexGetHolder(osal_mutex_handle_t mutex_handle)
{
    // Default implementation - return dummy task handle
    (void)mutex_handle;
    return (void*)0x11111111;
}

/**
 * @brief Default (weak) check if mutex is held by current task
 */
__attribute__((weak)) bool OSAL_MutexIsHeldByCurrentTask(osal_mutex_handle_t mutex_handle)
{
    // Default implementation - always return true
    (void)mutex_handle;
    return true;
} 