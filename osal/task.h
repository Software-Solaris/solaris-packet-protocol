/**
 * @file task.h
 * @brief OSAL Task Management Implementation
 * @version 1.0.0
 * @date 2024
 *
 * This file provides the default (weak) implementation of task management functions.
 */

#ifndef SPP_OSAL_TASK_H
#define SPP_OSAL_TASK_H

#include "core/returntypes.h"
#include "core/types.h"
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SPP_OSAL_STACK_BYTES
#define SPP_OSAL_STACK_BYTES 4096
#endif

SppRetVal_t SPP_OSAL_TaskCreate(void* task_function, const char* name,
                    spp_uint32_t stack_size, void* p_parameters, SppPriority_t priority,
                    SppTaskHandle_t* task_handle);

SppRetVal_t  SPP_OSAL_TaskDelete(SppTaskHandle_t task_handle);
SppRetVal_t  SPP_OSAL_TaskDelay(spp_uint32_t delay_ms);
SppRetVal_t  SPP_OSAL_TaskSuspend(SppTaskHandle_t task_handle);
SppRetVal_t  SPP_OSAL_TaskResume(SppTaskHandle_t task_handle);
void*        SPP_OSAL_TaskGetCurrent(void);
SppTaskState SPP_OSAL_TaskGetState(SppTaskHandle_t task_handle);
SppRetVal_t  SPP_OSAL_TaskYield(void);
SppRetVal_t  SPP_OSAL_TaskDelayUntil(spp_uint32_t period_ms);
SppRetVal_t  SPP_OSAL_TaskPrioritySet(SppTaskHandle_t task_handle, SppPriority_t priority);
SppPriority_t SPP_OSAL_TaskPriorityGet(SppTaskHandle_t task_handle);
SppRetVal_t  SPP_OSAL_SuspendAll(void);
SppRetVal_t  SPP_OSAL_ResumeAll(void);
SppRetVal_t  SPP_OSAL_IdleHookRegister(bool (*idle_function)(void));

#ifdef __cplusplus
}
#endif

#endif /* SPP_OSAL_TASK_H */
