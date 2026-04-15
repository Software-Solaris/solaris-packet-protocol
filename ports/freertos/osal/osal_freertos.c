/**
 * @file osal_freertos.c
 * @brief FreeRTOS OSAL port for SPP.
 *
 * Implements @ref SPP_OsalPort_t using FreeRTOS static-allocation APIs.
 * Define @c SPP_NO_RTOS=0 (the default) to use this port.
 *
 * Static task/event-group pools avoid heap fragmentation on bare-metal targets.
 */

#include "spp/osal/port.h"
#include "spp/core/returntypes.h"
#include "spp/util/macros.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

/* ----------------------------------------------------------------
 * Static task pool
 * ---------------------------------------------------------------- */

/** @brief Words per task stack in the static pool. */
#define K_FREERTOS_STACK_WORDS (K_SPP_STACK_SIZE / sizeof(StackType_t))

typedef struct
{
    StaticTask_t   tcb;
    StackType_t    stack[K_FREERTOS_STACK_WORDS];
    spp_bool_t     inUse;
} FreeRtosTaskStorage_t;

static FreeRtosTaskStorage_t s_taskPool[K_SPP_MAX_TASKS];

/* ----------------------------------------------------------------
 * Static event group pool
 * ---------------------------------------------------------------- */

static StaticEventGroup_t s_egPool[K_SPP_MAX_EVENT_GROUPS];
static spp_bool_t         s_egInUse[K_SPP_MAX_EVENT_GROUPS];

/* ----------------------------------------------------------------
 * Helper: ms → ticks (never zero)
 * ---------------------------------------------------------------- */

static TickType_t msToTicks(spp_uint32_t ms)
{
    TickType_t ticks = pdMS_TO_TICKS(ms);
    return (ticks == 0U) ? 1U : ticks;
}

/* ----------------------------------------------------------------
 * Task functions
 * ---------------------------------------------------------------- */

static void *freertosTaskCreate(void (*p_fn)(void *), const char *p_name,
                                 spp_uint32_t stackWords, void *p_arg,
                                 spp_uint32_t prio)
{
    (void)stackWords; /* Pool uses fixed K_FREERTOS_STACK_WORDS. */

    FreeRtosTaskStorage_t *p_slot = NULL;
    for (spp_uint32_t i = 0U; i < K_SPP_MAX_TASKS; i++)
    {
        if (!s_taskPool[i].inUse)
        {
            p_slot = &s_taskPool[i];
            p_slot->inUse = true;
            break;
        }
    }
    if (p_slot == NULL) return NULL;

    TaskHandle_t handle = xTaskCreateStatic(
        (TaskFunction_t)p_fn,
        p_name,
        K_FREERTOS_STACK_WORDS,
        p_arg,
        (UBaseType_t)prio,
        p_slot->stack,
        &p_slot->tcb);

    return (void *)handle;
}

static void freertosTaskDelete(void *p_handle)
{
    vTaskDelete((TaskHandle_t)p_handle);
}

static void freertosTaskDelayMs(spp_uint32_t ms)
{
    vTaskDelay(msToTicks(ms));
}

static spp_uint32_t freertosGetTickMs(void)
{
    return (spp_uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

/* ----------------------------------------------------------------
 * Queue functions
 * ---------------------------------------------------------------- */

static void *freertosQueueCreate(spp_uint32_t len, spp_uint32_t itemSize)
{
    return (void *)xQueueCreate((UBaseType_t)len, (UBaseType_t)itemSize);
}

static retval_t freertosQueueSend(void *p_q, const void *p_item, spp_uint32_t timeoutMs)
{
    BaseType_t ret = xQueueSend((QueueHandle_t)p_q, p_item, msToTicks(timeoutMs));
    return (ret == pdTRUE) ? SPP_OK : SPP_ERROR;
}

static retval_t freertosQueueRecv(void *p_q, void *p_item, spp_uint32_t timeoutMs)
{
    BaseType_t ret = xQueueReceive((QueueHandle_t)p_q, p_item, msToTicks(timeoutMs));
    return (ret == pdTRUE) ? SPP_OK : SPP_ERROR;
}

static spp_uint32_t freertosQueueCount(void *p_q)
{
    return (spp_uint32_t)uxQueueMessagesWaiting((QueueHandle_t)p_q);
}

/* ----------------------------------------------------------------
 * Mutex functions
 * ---------------------------------------------------------------- */

static void *freeRtosMutexCreate(void)
{
    return (void *)xSemaphoreCreateMutex();
}

static retval_t freertos_mutexLock(void *p_m, spp_uint32_t timeoutMs)
{
    BaseType_t ret = xSemaphoreTake((SemaphoreHandle_t)p_m, msToTicks(timeoutMs));
    return (ret == pdTRUE) ? SPP_OK : SPP_ERROR;
}

static retval_t freertos_mutexUnlock(void *p_m)
{
    BaseType_t ret = xSemaphoreGive((SemaphoreHandle_t)p_m);
    return (ret == pdTRUE) ? SPP_OK : SPP_ERROR;
}

/* ----------------------------------------------------------------
 * Event group functions
 * ---------------------------------------------------------------- */

static void *freertosEventCreate(void)
{
    for (spp_uint32_t i = 0U; i < K_SPP_MAX_EVENT_GROUPS; i++)
    {
        if (!s_egInUse[i])
        {
            s_egInUse[i] = true;
            return (void *)xEventGroupCreateStatic(&s_egPool[i]);
        }
    }
    return NULL;
}

static retval_t freertosEventWait(void *p_e, spp_uint32_t bits,
                                   spp_bool_t clearOnExit, spp_bool_t waitAll,
                                   spp_uint32_t timeoutMs,
                                   spp_uint32_t *p_actualBits)
{
    EventBits_t result = xEventGroupWaitBits(
        (EventGroupHandle_t)p_e,
        (EventBits_t)bits,
        clearOnExit ? pdTRUE : pdFALSE,
        waitAll     ? pdTRUE : pdFALSE,
        msToTicks(timeoutMs));

    if (p_actualBits != NULL) *p_actualBits = (spp_uint32_t)result;

    if (waitAll)
    {
        return ((result & bits) == bits) ? SPP_OK : SPP_ERROR_TIMEOUT;
    }
    return ((result & bits) != 0U) ? SPP_OK : SPP_ERROR_TIMEOUT;
}

static retval_t freertosEventSetFromIsr(void *p_e, spp_uint32_t bits,
                                         spp_uint32_t *p_prev,
                                         spp_bool_t   *p_yield)
{
    BaseType_t higherPrioWoken = pdFALSE;
    EventBits_t prev = xEventGroupSetBitsFromISR(
        (EventGroupHandle_t)p_e, (EventBits_t)bits, &higherPrioWoken);

    if (p_prev  != NULL) *p_prev  = (spp_uint32_t)prev;
    if (p_yield != NULL) *p_yield = (higherPrioWoken == pdTRUE);

    if (higherPrioWoken == pdTRUE)
    {
        portYIELD_FROM_ISR();
    }
    return SPP_OK;
}

/* ----------------------------------------------------------------
 * Port descriptor (extern — linked by spp_port_wrapper)
 * ---------------------------------------------------------------- */

const SPP_OsalPort_t g_freertosOsalPort = {
    .taskCreate      = freertosTaskCreate,
    .taskDelete      = freertosTaskDelete,
    .taskDelayMs     = freertosTaskDelayMs,
    .getTickMs       = freertosGetTickMs,
    .queueCreate     = freertosQueueCreate,
    .queueSend       = freertosQueueSend,
    .queueRecv       = freertosQueueRecv,
    .queueCount      = freertosQueueCount,
    .mutexCreate     = freeRtosMutexCreate,
    .mutexLock       = freertos_mutexLock,
    .mutexUnlock     = freertos_mutexUnlock,
    .eventCreate     = freertosEventCreate,
    .eventWait       = freertosEventWait,
    .eventSetFromIsr = freertosEventSetFromIsr,
};
