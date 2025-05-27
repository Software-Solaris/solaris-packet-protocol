/**
 * @file freertos_osal_impl.c
 * @brief FreeRTOS OSAL Implementation for SPP
 * @version 1.0.0
 * @date 2024
 * 
 * This file provides FreeRTOS specific OSAL implementation for the SPP layer.
 */

#include "freertos_osal_impl.h"
#include <string.h>
#include <stdio.h>

#ifdef SPP_FREERTOS_AVAILABLE
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#endif

static bool freertos_initialized = false;

/**
 * @brief Initialize FreeRTOS OSAL implementation
 */
retval_t FreeRTOS_OSAL_Init(void)
{
#ifdef SPP_FREERTOS_AVAILABLE
    if (freertos_initialized) {
        return SPP_ERROR_ALREADY_INITIALIZED;
    }
    
    // FreeRTOS initialization is typically done by the application
    // This function can be used for any additional setup
    freertos_initialized = true;
    return SPP_OK;
#else
    return SPP_ERROR_NOT_SUPPORTED;
#endif
}

/**
 * @brief Deinitialize FreeRTOS OSAL implementation
 */
retval_t FreeRTOS_OSAL_Deinit(void)
{
    freertos_initialized = false;
    return SPP_OK;
}

/**
 * @brief Check if FreeRTOS is available
 */
bool FreeRTOS_OSAL_IsAvailable(void)
{
#ifdef SPP_FREERTOS_AVAILABLE
    return true;
#else
    return false;
#endif
}

/**
 * @brief Get FreeRTOS version information
 */
retval_t FreeRTOS_OSAL_GetVersion(char* version_string, size_t buffer_size)
{
    if (version_string == NULL || buffer_size == 0) {
        return SPP_ERROR_NULL_POINTER;
    }
    
#ifdef SPP_FREERTOS_AVAILABLE
    snprintf(version_string, buffer_size, "FreeRTOS v%d.%d.%d", 
             tskKERNEL_VERSION_MAJOR, tskKERNEL_VERSION_MINOR, tskKERNEL_VERSION_BUILD);
#else
    snprintf(version_string, buffer_size, "FreeRTOS not available");
#endif
    
    return SPP_OK;
}

// ============================================================================
// Task Management Implementation
// ============================================================================

/**
 * @brief FreeRTOS Task Creation (OSAL Override)
 */
retval_t OSAL_TaskCreate(osal_task_function_t task_function, const char* name,
                         uint32_t stack_size, void* parameters, osal_priority_t priority,
                         osal_task_handle_t* task_handle)
{
    if (task_function == NULL || task_handle == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
#ifdef SPP_FREERTOS_AVAILABLE
    UBaseType_t freertos_priority;
    
    // Convert OSAL priority to FreeRTOS priority
    switch (priority) {
        case OSAL_PRIORITY_IDLE:
            freertos_priority = 0;
            break;
        case OSAL_PRIORITY_LOW:
            freertos_priority = 1;
            break;
        case OSAL_PRIORITY_NORMAL:
            freertos_priority = 2;
            break;
        case OSAL_PRIORITY_HIGH:
            freertos_priority = 3;
            break;
        case OSAL_PRIORITY_CRITICAL:
            freertos_priority = 4;
            break;
        default:
            freertos_priority = 2;
            break;
    }
    
    BaseType_t result = xTaskCreate(
        (TaskFunction_t)task_function,
        name ? name : "SPP_Task",
        stack_size / sizeof(StackType_t),
        parameters,
        freertos_priority,
        (TaskHandle_t*)task_handle
    );
    
    return (result == pdPASS) ? SPP_OK : SPP_ERROR_MEMORY_ALLOCATION;
#else
    // Fallback to weak implementation
    *task_handle = (void*)0x12345678;
    (void)name;
    (void)stack_size;
    (void)parameters;
    (void)priority;
    return SPP_OK;
#endif
}

/**
 * @brief FreeRTOS Task Deletion (OSAL Override)
 */
retval_t OSAL_TaskDelete(osal_task_handle_t task_handle)
{
#ifdef SPP_FREERTOS_AVAILABLE
    vTaskDelete((TaskHandle_t)task_handle);
    return SPP_OK;
#else
    (void)task_handle;
    return SPP_OK;
#endif
}

/**
 * @brief FreeRTOS Task Delay (OSAL Override)
 */
retval_t OSAL_TaskDelay(uint32_t delay_ms)
{
#ifdef SPP_FREERTOS_AVAILABLE
    TickType_t delay_ticks = pdMS_TO_TICKS(delay_ms);
    vTaskDelay(delay_ticks);
    return SPP_OK;
#else
    // Fallback to busy wait
    volatile uint32_t count = delay_ms * 1000;
    while (count--) {
        // Busy wait
    }
    return SPP_OK;
#endif
}

/**
 * @brief FreeRTOS Task Suspend (OSAL Override)
 */
retval_t OSAL_TaskSuspend(osal_task_handle_t task_handle)
{
#ifdef SPP_FREERTOS_AVAILABLE
    vTaskSuspend((TaskHandle_t)task_handle);
    return SPP_OK;
#else
    (void)task_handle;
    return SPP_OK;
#endif
}

/**
 * @brief FreeRTOS Task Resume (OSAL Override)
 */
retval_t OSAL_TaskResume(osal_task_handle_t task_handle)
{
#ifdef SPP_FREERTOS_AVAILABLE
    vTaskResume((TaskHandle_t)task_handle);
    return SPP_OK;
#else
    (void)task_handle;
    return SPP_OK;
#endif
}

/**
 * @brief FreeRTOS Get Current Task (OSAL Override)
 */
osal_task_handle_t OSAL_TaskGetCurrent(void)
{
#ifdef SPP_FREERTOS_AVAILABLE
    return (osal_task_handle_t)xTaskGetCurrentTaskHandle();
#else
    return (void*)0x11111111;
#endif
}

/**
 * @brief FreeRTOS Get Task State (OSAL Override)
 */
osal_task_state_t OSAL_TaskGetState(osal_task_handle_t task_handle)
{
#ifdef SPP_FREERTOS_AVAILABLE
    eTaskState freertos_state = eTaskGetState((TaskHandle_t)task_handle);
    
    switch (freertos_state) {
        case eReady:
            return OSAL_TASK_READY;
        case eRunning:
            return OSAL_TASK_RUNNING;
        case eBlocked:
            return OSAL_TASK_BLOCKED;
        case eSuspended:
            return OSAL_TASK_SUSPENDED;
        case eDeleted:
            return OSAL_TASK_DELETED;
        default:
            return OSAL_TASK_READY;
    }
#else
    (void)task_handle;
    return OSAL_TASK_RUNNING;
#endif
}

/**
 * @brief FreeRTOS Task Yield (OSAL Override)
 */
retval_t OSAL_TaskYield(void)
{
#ifdef SPP_FREERTOS_AVAILABLE
    taskYIELD();
    return SPP_OK;
#else
    return SPP_OK;
#endif
}

// ============================================================================
// Queue Management Implementation
// ============================================================================

/**
 * @brief FreeRTOS Queue Creation (OSAL Override)
 */
retval_t OSAL_QueueCreate(osal_queue_handle_t* queue_handle, uint32_t queue_length, uint32_t item_size)
{
    if (queue_handle == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
#ifdef SPP_FREERTOS_AVAILABLE
    QueueHandle_t queue = xQueueCreate(queue_length, item_size);
    if (queue == NULL) {
        return SPP_ERROR_MEMORY_ALLOCATION;
    }
    
    *queue_handle = (osal_queue_handle_t)queue;
    return SPP_OK;
#else
    *queue_handle = (void*)0xABCDEF00;
    (void)queue_length;
    (void)item_size;
    return SPP_OK;
#endif
}

/**
 * @brief FreeRTOS Queue Deletion (OSAL Override)
 */
retval_t OSAL_QueueDelete(osal_queue_handle_t queue_handle)
{
#ifdef SPP_FREERTOS_AVAILABLE
    if (queue_handle != NULL) {
        vQueueDelete((QueueHandle_t)queue_handle);
    }
    return SPP_OK;
#else
    (void)queue_handle;
    return SPP_OK;
#endif
}

/**
 * @brief FreeRTOS Queue Send (OSAL Override)
 */
retval_t OSAL_QueueSend(osal_queue_handle_t queue_handle, const void* item, uint32_t timeout_ms)
{
    if (queue_handle == NULL || item == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
#ifdef SPP_FREERTOS_AVAILABLE
    TickType_t timeout_ticks = (timeout_ms == UINT32_MAX) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    BaseType_t result = xQueueSend((QueueHandle_t)queue_handle, item, timeout_ticks);
    return (result == pdTRUE) ? SPP_OK : SPP_ERROR_TIMEOUT;
#else
    (void)timeout_ms;
    return SPP_OK;
#endif
}

/**
 * @brief FreeRTOS Queue Send from ISR (OSAL Override)
 */
retval_t OSAL_QueueSendFromISR(osal_queue_handle_t queue_handle, const void* item, bool* higher_priority_task_woken)
{
    if (queue_handle == NULL || item == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
#ifdef SPP_FREERTOS_AVAILABLE
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t result = xQueueSendFromISR((QueueHandle_t)queue_handle, item, &xHigherPriorityTaskWoken);
    
    if (higher_priority_task_woken != NULL) {
        *higher_priority_task_woken = (xHigherPriorityTaskWoken == pdTRUE);
    }
    
    return (result == pdTRUE) ? SPP_OK : SPP_ERROR_TIMEOUT;
#else
    if (higher_priority_task_woken != NULL) {
        *higher_priority_task_woken = false;
    }
    return SPP_OK;
#endif
}

/**
 * @brief FreeRTOS Queue Receive (OSAL Override)
 */
retval_t OSAL_QueueReceive(osal_queue_handle_t queue_handle, void* item, uint32_t timeout_ms)
{
    if (queue_handle == NULL || item == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
#ifdef SPP_FREERTOS_AVAILABLE
    TickType_t timeout_ticks = (timeout_ms == UINT32_MAX) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    BaseType_t result = xQueueReceive((QueueHandle_t)queue_handle, item, timeout_ticks);
    return (result == pdTRUE) ? SPP_OK : SPP_ERROR_TIMEOUT;
#else
    memset(item, 0xAA, sizeof(uint32_t));
    (void)timeout_ms;
    return SPP_OK;
#endif
}

// ============================================================================
// Mutex Management Implementation
// ============================================================================

/**
 * @brief FreeRTOS Mutex Creation (OSAL Override)
 */
retval_t OSAL_MutexCreate(osal_mutex_handle_t* mutex_handle, osal_mutex_type_t type)
{
    if (mutex_handle == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
#ifdef SPP_FREERTOS_AVAILABLE
    SemaphoreHandle_t mutex;
    
    if (type == OSAL_MUTEX_RECURSIVE) {
        mutex = xSemaphoreCreateRecursiveMutex();
    } else {
        mutex = xSemaphoreCreateMutex();
    }
    
    if (mutex == NULL) {
        return SPP_ERROR_MEMORY_ALLOCATION;
    }
    
    *mutex_handle = (osal_mutex_handle_t)mutex;
    return SPP_OK;
#else
    *mutex_handle = (void*)0x87654321;
    (void)type;
    return SPP_OK;
#endif
}

/**
 * @brief FreeRTOS Mutex Take (OSAL Override)
 */
retval_t OSAL_MutexTake(osal_mutex_handle_t mutex_handle, uint32_t timeout_ms)
{
    if (mutex_handle == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
#ifdef SPP_FREERTOS_AVAILABLE
    TickType_t timeout_ticks = (timeout_ms == UINT32_MAX) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    BaseType_t result = xSemaphoreTake((SemaphoreHandle_t)mutex_handle, timeout_ticks);
    return (result == pdTRUE) ? SPP_OK : SPP_ERROR_TIMEOUT;
#else
    (void)timeout_ms;
    return SPP_OK;
#endif
}

/**
 * @brief FreeRTOS Mutex Give (OSAL Override)
 */
retval_t OSAL_MutexGive(osal_mutex_handle_t mutex_handle)
{
    if (mutex_handle == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
#ifdef SPP_FREERTOS_AVAILABLE
    BaseType_t result = xSemaphoreGive((SemaphoreHandle_t)mutex_handle);
    return (result == pdTRUE) ? SPP_OK : SPP_ERROR_INVALID_STATE;
#else
    return SPP_OK;
#endif
}

// ============================================================================
// Semaphore Management Implementation
// ============================================================================

/**
 * @brief FreeRTOS Counting Semaphore Creation (OSAL Override)
 */
retval_t OSAL_SemaphoreCreate(osal_semaphore_handle_t* semaphore_handle, uint32_t max_count, uint32_t initial_count)
{
    if (semaphore_handle == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
    if (initial_count > max_count) {
        return SPP_ERROR_INVALID_PARAMETER;
    }
    
#ifdef SPP_FREERTOS_AVAILABLE
    SemaphoreHandle_t semaphore = xSemaphoreCreateCounting(max_count, initial_count);
    if (semaphore == NULL) {
        return SPP_ERROR_MEMORY_ALLOCATION;
    }
    
    *semaphore_handle = (osal_semaphore_handle_t)semaphore;
    return SPP_OK;
#else
    *semaphore_handle = (void*)0xDEADBEEF;
    (void)max_count;
    (void)initial_count;
    return SPP_OK;
#endif
}

/**
 * @brief FreeRTOS Binary Semaphore Creation (OSAL Override)
 */
retval_t OSAL_SemaphoreCreateBinary(osal_semaphore_handle_t* semaphore_handle)
{
    if (semaphore_handle == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
#ifdef SPP_FREERTOS_AVAILABLE
    SemaphoreHandle_t semaphore = xSemaphoreCreateBinary();
    if (semaphore == NULL) {
        return SPP_ERROR_MEMORY_ALLOCATION;
    }
    
    *semaphore_handle = (osal_semaphore_handle_t)semaphore;
    return SPP_OK;
#else
    *semaphore_handle = (void*)0xBEEFDEAD;
    return SPP_OK;
#endif
}

/**
 * @brief FreeRTOS Semaphore Take (OSAL Override)
 */
retval_t OSAL_SemaphoreTake(osal_semaphore_handle_t semaphore_handle, uint32_t timeout_ms)
{
    if (semaphore_handle == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
#ifdef SPP_FREERTOS_AVAILABLE
    TickType_t timeout_ticks = (timeout_ms == UINT32_MAX) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    BaseType_t result = xSemaphoreTake((SemaphoreHandle_t)semaphore_handle, timeout_ticks);
    return (result == pdTRUE) ? SPP_OK : SPP_ERROR_TIMEOUT;
#else
    (void)timeout_ms;
    return SPP_OK;
#endif
}

/**
 * @brief FreeRTOS Semaphore Give (OSAL Override)
 */
retval_t OSAL_SemaphoreGive(osal_semaphore_handle_t semaphore_handle)
{
    if (semaphore_handle == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
#ifdef SPP_FREERTOS_AVAILABLE
    BaseType_t result = xSemaphoreGive((SemaphoreHandle_t)semaphore_handle);
    return (result == pdTRUE) ? SPP_OK : SPP_ERROR_INVALID_STATE;
#else
    return SPP_OK;
#endif
} 