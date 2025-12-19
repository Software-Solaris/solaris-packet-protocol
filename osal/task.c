/**
 * @file task.c
 * @brief OSAL Task Management Implementation
 * @version 1.0.0
 * @date 2024
 * 
 * This file provides the default (weak) implementation of task management functions.
 */

#include "osal/task.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

__attribute__((weak))  void * SPP_OSAL_GetTaskStorage(){
    return NULL;
}


// __attribute__((weak)) void* SPP_OSAL_TaskCreate(void *p_function, const char *const task_name, 
//                             const uint32_t stack_depth,void *const p_custom_data,
//                             spp_uint32_t priority, void * p_storage)

// {
//     // If no implementation is defined, then error is returned
//     return NULL;
// }

__attribute__((weak)) retval_t SPP_OSAL_TaskDelete(void *p_task){
    return SPP_ERROR;
}


/**
 * @brief Default (weak) task delay
 */
__attribute__((weak)) void SPP_OSAL_TaskDelay(spp_uint32_t blocktime_ms)
{
    return;
}

