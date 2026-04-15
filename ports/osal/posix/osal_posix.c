/**
 * @file osal_posix.c
 * @brief POSIX OSAL port — used for host-side unit testing with Cgreen.
 *
 * Implements @ref SPP_OsalPort_t using POSIX pthreads, mutexes, and a simple
 * event-group emulation with condition variables.  Not intended for production
 * embedded use; optimised for test simplicity and coverage.
 */

#include "spp/osal/port.h"
#include "spp/core/returntypes.h"
#include "spp/util/macros.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/time.h>

/* ----------------------------------------------------------------
 * Queue implementation (ring buffer + mutex + condition)
 * ---------------------------------------------------------------- */

typedef struct
{
    uint8_t     *p_buf;
    uint32_t     itemSize;
    uint32_t     capacity;
    uint32_t     head;
    uint32_t     tail;
    uint32_t     count;
    pthread_mutex_t lock;
    pthread_cond_t  notEmpty;
    pthread_cond_t  notFull;
} PosixQueue_t;

static void *posixQueueCreate(uint32_t len, uint32_t itemSize)
{
    PosixQueue_t *p_q = malloc(sizeof(PosixQueue_t));
    if (p_q == NULL) return NULL;

    p_q->p_buf    = malloc(len * itemSize);
    p_q->itemSize = itemSize;
    p_q->capacity = len;
    p_q->head     = 0U;
    p_q->tail     = 0U;
    p_q->count    = 0U;
    pthread_mutex_init(&p_q->lock, NULL);
    pthread_cond_init(&p_q->notEmpty, NULL);
    pthread_cond_init(&p_q->notFull, NULL);
    return p_q;
}

static SPP_RetVal_t posixQueueSend(void *p_q, const void *p_item, uint32_t timeoutMs)
{
    PosixQueue_t *p_queue = (PosixQueue_t *)p_q;
    pthread_mutex_lock(&p_queue->lock);

    if (p_queue->count >= p_queue->capacity)
    {
        pthread_mutex_unlock(&p_queue->lock);
        return K_SPP_ERROR;
    }

    memcpy(p_queue->p_buf + (p_queue->head * p_queue->itemSize), p_item, p_queue->itemSize);
    p_queue->head = (p_queue->head + 1U) % p_queue->capacity;
    p_queue->count++;

    pthread_cond_signal(&p_queue->notEmpty);
    pthread_mutex_unlock(&p_queue->lock);
    (void)timeoutMs;
    return K_SPP_OK;
}

static SPP_RetVal_t posixQueueRecv(void *p_q, void *p_item, uint32_t timeoutMs)
{
    PosixQueue_t *p_queue = (PosixQueue_t *)p_q;
    pthread_mutex_lock(&p_queue->lock);

    if (p_queue->count == 0U)
    {
        pthread_mutex_unlock(&p_queue->lock);
        return K_SPP_ERROR;
    }

    memcpy(p_item, p_queue->p_buf + (p_queue->tail * p_queue->itemSize), p_queue->itemSize);
    p_queue->tail = (p_queue->tail + 1U) % p_queue->capacity;
    p_queue->count--;

    pthread_cond_signal(&p_queue->notFull);
    pthread_mutex_unlock(&p_queue->lock);
    (void)timeoutMs;
    return K_SPP_OK;
}

static uint32_t posixQueueCount(void *p_q)
{
    PosixQueue_t *p_queue = (PosixQueue_t *)p_q;
    pthread_mutex_lock(&p_queue->lock);
    uint32_t count = p_queue->count;
    pthread_mutex_unlock(&p_queue->lock);
    return count;
}

/* ----------------------------------------------------------------
 * Mutex implementation
 * ---------------------------------------------------------------- */

static void *posixMutexCreate(void)
{
    pthread_mutex_t *p_m = malloc(sizeof(pthread_mutex_t));
    if (p_m != NULL)
    {
        pthread_mutex_init(p_m, NULL);
    }
    return p_m;
}

static SPP_RetVal_t posixMutexLock(void *p_m, uint32_t timeoutMs)
{
    (void)timeoutMs;
    return (pthread_mutex_lock((pthread_mutex_t *)p_m) == 0) ? K_SPP_OK : K_SPP_ERROR;
}

static SPP_RetVal_t posixMutexUnlock(void *p_m)
{
    return (pthread_mutex_unlock((pthread_mutex_t *)p_m) == 0) ? K_SPP_OK : K_SPP_ERROR;
}

/* ----------------------------------------------------------------
 * Event group implementation (bitmask + mutex + cond)
 * ---------------------------------------------------------------- */

typedef struct
{
    uint32_t        bits;
    pthread_mutex_t lock;
    pthread_cond_t  cond;
} PosixEventGroup_t;

static void *posixEventCreate(void)
{
    PosixEventGroup_t *p_eg = malloc(sizeof(PosixEventGroup_t));
    if (p_eg == NULL) return NULL;
    p_eg->bits = 0U;
    pthread_mutex_init(&p_eg->lock, NULL);
    pthread_cond_init(&p_eg->cond, NULL);
    return p_eg;
}

static SPP_RetVal_t posixEventWait(void *p_e, uint32_t bits, spp_bool_t clearOnExit,
                                spp_bool_t waitAll, uint32_t timeoutMs,
                                uint32_t *p_actualBits)
{
    PosixEventGroup_t *p_eg = (PosixEventGroup_t *)p_e;
    pthread_mutex_lock(&p_eg->lock);

    uint32_t matched;
    if (waitAll)
    {
        matched = (p_eg->bits & bits) == bits ? bits : 0U;
    }
    else
    {
        matched = p_eg->bits & bits;
    }

    if (matched == 0U)
    {
        /* Simple non-blocking check for test purposes. */
        (void)timeoutMs;
        if (p_actualBits != NULL) *p_actualBits = p_eg->bits;
        pthread_mutex_unlock(&p_eg->lock);
        return K_SPP_ERROR_TIMEOUT;
    }

    if (p_actualBits != NULL) *p_actualBits = p_eg->bits;
    if (clearOnExit) p_eg->bits &= ~bits;
    pthread_mutex_unlock(&p_eg->lock);
    return K_SPP_OK;
}

static SPP_RetVal_t posixEventSetFromIsr(void *p_e, uint32_t bits,
                                      uint32_t *p_prev, spp_bool_t *p_yield)
{
    PosixEventGroup_t *p_eg = (PosixEventGroup_t *)p_e;
    pthread_mutex_lock(&p_eg->lock);
    if (p_prev != NULL) *p_prev = p_eg->bits;
    p_eg->bits |= bits;
    pthread_cond_broadcast(&p_eg->cond);
    pthread_mutex_unlock(&p_eg->lock);
    if (p_yield != NULL) *p_yield = false;
    return K_SPP_OK;
}

/* ----------------------------------------------------------------
 * Task implementation
 * ---------------------------------------------------------------- */

static void *posixTaskCreate(void (*p_fn)(void *), const char *p_name,
                              uint32_t stackWords, void *p_arg, uint32_t prio)
{
    (void)p_name; (void)stackWords; (void)prio;
    pthread_t *p_thread = malloc(sizeof(pthread_t));
    if (p_thread == NULL) return NULL;
    pthread_create(p_thread, NULL, (void *(*)(void *))p_fn, p_arg);
    return p_thread;
}

static void posixTaskDelete(void *p_handle)
{
    if (p_handle != NULL)
    {
        pthread_cancel(*(pthread_t *)p_handle);
    }
}

static void posixTaskDelayMs(uint32_t ms)
{
    usleep((useconds_t)(ms * 1000U));
}

static uint32_t posixGetTickMs(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint32_t)((tv.tv_sec * 1000UL) + (tv.tv_usec / 1000UL));
}

/* ----------------------------------------------------------------
 * Port descriptor (extern — used by tests and posix main)
 * ---------------------------------------------------------------- */

const SPP_OsalPort_t g_posixOsalPort = {
    .taskCreate       = posixTaskCreate,
    .taskDelete       = posixTaskDelete,
    .taskDelayMs      = posixTaskDelayMs,
    .getTickMs        = posixGetTickMs,
    .queueCreate      = posixQueueCreate,
    .queueSend        = posixQueueSend,
    .queueRecv        = posixQueueRecv,
    .queueCount       = posixQueueCount,
    .mutexCreate      = posixMutexCreate,
    .mutexLock        = posixMutexLock,
    .mutexUnlock      = posixMutexUnlock,
    .eventCreate      = posixEventCreate,
    .eventWait        = posixEventWait,
    .eventSetFromIsr  = posixEventSetFromIsr,
};
