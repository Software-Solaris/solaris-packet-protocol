/**
 * @file task.h
 * @brief OSAL Task Management Interface
 * @version 1.0.0
 * @date 2024
 * 
 * This file provides the task management interface for the OSAL layer.
 */

#ifndef SPP_OSAL_TASK_H
#define SPP_OSAL_TASK_H

#include <stdint.h>
#include <stdbool.h>
#include "core/returntypes.h"
#include "core/types.h"
#include "core/macros.h"

void * SPP_OSAL_GetTaskStorage();


void* SPP_OSAL_TaskCreate(void *p_function, const char *const task_name, 
                            const uint32_t stack_depth,void *const p_custom_data,
                            spp_uint32_t priority, void * p_storage);

retval_t SPP_OSAL_TaskDelete(void *p_task);



#endif /* SPP_OSAL_TASK_H */ 