/**
 * @file osal_baremetal.c
 * @brief Bare-metal cooperative scheduler OSAL port for SPP.
 *
 * Implements @ref SPP_OsalPort_t without any OS.  Tasks are registered in a
 * static table and executed cooperatively in round-robin order by
 * @ref SPP_Baremetal_run(), which must be called from the application's
 * main loop.
 *
 * Queues, mutexes, and event groups are implemented as in-memory structures
 * with no blocking — all timeout parameters are ignored.
 *
 * @note  Define @c SPP_BAREMETAL_GET_TICK_MS to a function or macro that
 *        returns the current millisecond tick from your hardware timer.
 *        Default: returns 0.
 */

#include "spp/osal/port.h"
#include "spp/core/returntypes.h"
#include "spp/util/macros.h"

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

/* ----------------------------------------------------------------
 * Hardware tick hook
 * ---------------------------------------------------------------- */

#ifndef SPP_BAREMETAL_GET_TICK_MS
#define SPP_BAREMETAL_GET_TICK_MS() (0U)
#endif

/* ----------------------------------------------------------------
 * Cooperative task table
 * ---------------------------------------------------------------- */

typedef struct
{
    void (*p_fn)(void *);
    void        *p_arg;
    spp_uint32_t delayMs;
    spp_uint32_t nextRunMs;
    spp_bool_t   inUse;
} BmTask_t;

static BmTask_t s_tasks[K_SPP_MAX_TASKS];
static spp_uint32_t s_taskCount = 0U;

/**
 * @brief Run one scheduler tick — call this from your main loop.
 *
 * Iterates over all registered tasks and calls any whose deadline has passed.
 */
void SPP_Baremetal_run(void)
{
    spp_uint32_t now = (spp_uint32_t)SPP_BAREMETAL_GET_TICK_MS();
    for (spp_uint32_t i = 0U; i < s_taskCount; i++)
    {
        if (s_tasks[i].inUse && (now >= s_tasks[i].nextRunMs))
        {
            s_tasks[i].p_fn(s_tasks[i].p_arg);
            s_tasks[i].nextRunMs = now + s_tasks[i].delayMs;
        }
    }
}

/* ----------------------------------------------------------------
 * Task functions
 * ---------------------------------------------------------------- */

static void *bmTaskCreate(void (*p_fn)(void *), const char *p_name,
                           spp_uint32_t stackWords, void *p_arg,
                           spp_uint32_t prio)
{
    (void)p_name; (void)stackWords; (void)prio;
    if (s_taskCount >= K_SPP_MAX_TASKS) return NULL;

    BmTask_t *p_t = &s_tasks[s_taskCount];
    p_t->p_fn      = p_fn;
    p_t->p_arg     = p_arg;
    p_t->delayMs   = 0U; /* Tasks yield via taskDelayMs each iteration. */
    p_t->nextRunMs = 0U;
    p_t->inUse     = true;
    s_taskCount++;
    return (void *)p_t;
}

static void bmTaskDelete(void *p_handle)
{
    if (p_handle != NULL)
    {
        ((BmTask_t *)p_handle)->inUse = false;
    }
}

static void bmTaskDelayMs(spp_uint32_t ms)
{
    /* In cooperative bare-metal, a "delay" sets the task's next run time.
     * The SPP_Baremetal_run() loop enforces the delay.
     * When called from within a task function, record the delay on the
     * current task slot (identified by scanning the task table). */
    spp_uint32_t now = (spp_uint32_t)SPP_BAREMETAL_GET_TICK_MS();
    /* Best-effort: update all tasks that have a zero delay (first call). */
    for (spp_uint32_t i = 0U; i < s_taskCount; i++)
    {
        if (s_tasks[i].inUse && (s_tasks[i].delayMs == 0U))
        {
            s_tasks[i].delayMs   = ms;
            s_tasks[i].nextRunMs = now + ms;
        }
    }
}

static spp_uint32_t bmGetTickMs(void)
{
    return (spp_uint32_t)SPP_BAREMETAL_GET_TICK_MS();
}

/* ----------------------------------------------------------------
 * Queue (simple ring buffer, no blocking)
 * ---------------------------------------------------------------- */

typedef struct
{
    uint8_t     *p_buf;
    uint32_t     itemSize;
    uint32_t     capacity;
    uint32_t     head;
    uint32_t     tail;
    uint32_t     count;
} BmQueue_t;

static BmQueue_t s_queuePool[K_SPP_MAX_TASKS]; /* Reuse pool size for queues. */
static spp_uint32_t s_queueCount = 0U;

static uint8_t s_queueBufs[K_SPP_MAX_TASKS][64U]; /* Fixed 64-byte items max. */

static void *bmQueueCreate(spp_uint32_t len, spp_uint32_t itemSize)
{
    if (s_queueCount >= K_SPP_MAX_TASKS) return NULL;
    BmQueue_t *p_q  = &s_queuePool[s_queueCount];
    p_q->p_buf      = s_queueBufs[s_queueCount];
    p_q->itemSize   = itemSize;
    p_q->capacity   = len;
    p_q->head = p_q->tail = p_q->count = 0U;
    s_queueCount++;
    (void)len; /* Capacity bounded by buf size. */
    return p_q;
}

static SPP_RetVal_t bmQueueSend(void *p_q, const void *p_item, spp_uint32_t timeoutMs)
{
    (void)timeoutMs;
    BmQueue_t *p_queue = (BmQueue_t *)p_q;
    if (p_queue->count >= p_queue->capacity) return K_SPP_ERROR;
    memcpy(p_queue->p_buf + (p_queue->head * p_queue->itemSize), p_item, p_queue->itemSize);
    p_queue->head = (p_queue->head + 1U) % p_queue->capacity;
    p_queue->count++;
    return K_SPP_OK;
}

static SPP_RetVal_t bmQueueRecv(void *p_q, void *p_item, spp_uint32_t timeoutMs)
{
    (void)timeoutMs;
    BmQueue_t *p_queue = (BmQueue_t *)p_q;
    if (p_queue->count == 0U) return K_SPP_ERROR;
    memcpy(p_item, p_queue->p_buf + (p_queue->tail * p_queue->itemSize), p_queue->itemSize);
    p_queue->tail = (p_queue->tail + 1U) % p_queue->capacity;
    p_queue->count--;
    return K_SPP_OK;
}

static spp_uint32_t bmQueueCount(void *p_q)
{
    return ((BmQueue_t *)p_q)->count;
}

/* ----------------------------------------------------------------
 * Mutex (bare-metal: no contention, always succeed)
 * ---------------------------------------------------------------- */

static spp_uint32_t s_mutexDummy = 0U;

static void    *bmMutexCreate(void)                      { return &s_mutexDummy; }
static SPP_RetVal_t bmMutexLock(void *p_m, spp_uint32_t t)   { (void)p_m; (void)t; return K_SPP_OK; }
static SPP_RetVal_t bmMutexUnlock(void *p_m)                 { (void)p_m; return K_SPP_OK; }

/* ----------------------------------------------------------------
 * Event group (bare-metal: simple bitmask, no blocking)
 * ---------------------------------------------------------------- */

static spp_uint32_t s_egPool[K_SPP_MAX_EVENT_GROUPS];
static spp_uint32_t s_egCount = 0U;

static void *bmEventCreate(void)
{
    if (s_egCount >= K_SPP_MAX_EVENT_GROUPS) return NULL;
    s_egPool[s_egCount] = 0U;
    return (void *)&s_egPool[s_egCount++];
}

static SPP_RetVal_t bmEventWait(void *p_e, spp_uint32_t bits, spp_bool_t clear,
                             spp_bool_t waitAll, spp_uint32_t timeoutMs,
                             spp_uint32_t *p_actual)
{
    (void)timeoutMs;
    spp_uint32_t *p_bits = (spp_uint32_t *)p_e;
    spp_uint32_t matched = waitAll
        ? (((*p_bits & bits) == bits) ? bits : 0U)
        : (*p_bits & bits);

    if (p_actual != NULL) *p_actual = *p_bits;
    if (matched == 0U) return K_SPP_ERROR_TIMEOUT;
    if (clear) *p_bits &= ~bits;
    return K_SPP_OK;
}

static SPP_RetVal_t bmEventSetFromIsr(void *p_e, spp_uint32_t bits,
                                   spp_uint32_t *p_prev, spp_bool_t *p_yield)
{
    spp_uint32_t *p_bits = (spp_uint32_t *)p_e;
    if (p_prev  != NULL) *p_prev  = *p_bits;
    *p_bits |= bits;
    if (p_yield != NULL) *p_yield = false;
    return K_SPP_OK;
}

/* ----------------------------------------------------------------
 * Port descriptor
 * ---------------------------------------------------------------- */

const SPP_OsalPort_t g_baremetalOsalPort = {
    .taskCreate      = bmTaskCreate,
    .taskDelete      = bmTaskDelete,
    .taskDelayMs     = bmTaskDelayMs,
    .getTickMs       = bmGetTickMs,
    .queueCreate     = bmQueueCreate,
    .queueSend       = bmQueueSend,
    .queueRecv       = bmQueueRecv,
    .queueCount      = bmQueueCount,
    .mutexCreate     = bmMutexCreate,
    .mutexLock       = bmMutexLock,
    .mutexUnlock     = bmMutexUnlock,
    .eventCreate     = bmEventCreate,
    .eventWait       = bmEventWait,
    .eventSetFromIsr = bmEventSetFromIsr,
};
