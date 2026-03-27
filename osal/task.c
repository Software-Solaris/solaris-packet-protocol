/**
 * @file task.c
 * @brief OSAL Task Management Implementation
 * @version 1.0.0
 * @date 2024
 *
 * This file provides the default (weak) implementation of task management functions.
 */

#include "spp/osal/task.h"
#include <stddef.h>

/* ============================================================================
 *  Task Storage
 * ========================================================================= */

/**
 * @brief Default (weak) implementation of task storage retrieval
 *
 * @return void* Always returns NULL in the default implementation
 */
__attribute__((weak)) void *SPP_OSAL_GetTaskStorage()
{
    return NULL;
}

/* ============================================================================
 *  Task Lifecycle
 * ========================================================================= */

/**
 * @brief Default (weak) implementation of task deletion
 *
 * @param[in] p_task Handle of the task to delete
 *
 * @return retval_t SPP_ERROR in the default implementation
 */
__attribute__((weak)) retval_t SPP_OSAL_TaskDelete(void *p_task)
{
    return SPP_ERROR;
}


/**
 * @brief Default (weak) implementation of task delay
 *
 * @param blocktime_ms Time of the delay in milliseconds
 */
__attribute__((weak)) void SPP_OSAL_TaskDelay(spp_uint32_t blocktime_ms)
{
    return;
}
