/**
 * @file osal_dispatch.c
 * @brief OSAL dispatch — forwards every call through the registered port.
 *
 * If no port has been registered the functions return safe default values
 * (NULL, 0, or SPP_ERROR_NO_PORT) rather than dereferencing a NULL pointer.
 */

#include "spp/osal/task.h"
#include "spp/osal/queue.h"
#include "spp/osal/mutex.h"
#include "spp/osal/event.h"
#include "spp/core/core.h"
#include "spp/core/returntypes.h"

/* ----------------------------------------------------------------
 * Internal helper
 * ---------------------------------------------------------------- */

/** @brief Retrieve the registered OSAL port; used by every dispatch fn. */
static inline const SPP_OsalPort_t *getPort(void)
{
    return SPP_Core_getOsalPort();
}

/* ----------------------------------------------------------------
 * Task dispatch
 * ---------------------------------------------------------------- */

void *SPP_Osal_taskCreate(void (*p_fn)(void *), const char *p_name,
                           spp_uint32_t stackWords, void *p_arg,
                           spp_uint32_t prio)
{
    const SPP_OsalPort_t *p_port = getPort();
    if ((p_port == NULL) || (p_port->taskCreate == NULL))
    {
        return NULL;
    }
    return p_port->taskCreate(p_fn, p_name, stackWords, p_arg, prio);
}

void SPP_Osal_taskDelete(void *p_handle)
{
    const SPP_OsalPort_t *p_port = getPort();
    if ((p_port != NULL) && (p_port->taskDelete != NULL))
    {
        p_port->taskDelete(p_handle);
    }
}

void SPP_Osal_taskDelayMs(spp_uint32_t ms)
{
    const SPP_OsalPort_t *p_port = getPort();
    if ((p_port != NULL) && (p_port->taskDelayMs != NULL))
    {
        p_port->taskDelayMs(ms);
    }
}

spp_uint32_t SPP_Osal_getTickMs(void)
{
    const SPP_OsalPort_t *p_port = getPort();
    if ((p_port == NULL) || (p_port->getTickMs == NULL))
    {
        return 0U;
    }
    return p_port->getTickMs();
}

/* ----------------------------------------------------------------
 * Queue dispatch
 * ---------------------------------------------------------------- */

void *SPP_Osal_queueCreate(spp_uint32_t len, spp_uint32_t itemSize)
{
    const SPP_OsalPort_t *p_port = getPort();
    if ((p_port == NULL) || (p_port->queueCreate == NULL))
    {
        return NULL;
    }
    return p_port->queueCreate(len, itemSize);
}

retval_t SPP_Osal_queueSend(void *p_q, const void *p_item,
                              spp_uint32_t timeoutMs)
{
    const SPP_OsalPort_t *p_port = getPort();
    if ((p_port == NULL) || (p_port->queueSend == NULL))
    {
        return SPP_ERROR_NO_PORT;
    }
    return p_port->queueSend(p_q, p_item, timeoutMs);
}

retval_t SPP_Osal_queueRecv(void *p_q, void *p_item,
                              spp_uint32_t timeoutMs)
{
    const SPP_OsalPort_t *p_port = getPort();
    if ((p_port == NULL) || (p_port->queueRecv == NULL))
    {
        return SPP_ERROR_NO_PORT;
    }
    return p_port->queueRecv(p_q, p_item, timeoutMs);
}

spp_uint32_t SPP_Osal_queueCount(void *p_q)
{
    const SPP_OsalPort_t *p_port = getPort();
    if ((p_port == NULL) || (p_port->queueCount == NULL))
    {
        return 0U;
    }
    return p_port->queueCount(p_q);
}

/* ----------------------------------------------------------------
 * Mutex dispatch
 * ---------------------------------------------------------------- */

void *SPP_Osal_mutexCreate(void)
{
    const SPP_OsalPort_t *p_port = getPort();
    if ((p_port == NULL) || (p_port->mutexCreate == NULL))
    {
        return NULL;
    }
    return p_port->mutexCreate();
}

retval_t SPP_Osal_mutexLock(void *p_m, spp_uint32_t timeoutMs)
{
    const SPP_OsalPort_t *p_port = getPort();
    if ((p_port == NULL) || (p_port->mutexLock == NULL))
    {
        return SPP_ERROR_NO_PORT;
    }
    return p_port->mutexLock(p_m, timeoutMs);
}

retval_t SPP_Osal_mutexUnlock(void *p_m)
{
    const SPP_OsalPort_t *p_port = getPort();
    if ((p_port == NULL) || (p_port->mutexUnlock == NULL))
    {
        return SPP_ERROR_NO_PORT;
    }
    return p_port->mutexUnlock(p_m);
}

/* ----------------------------------------------------------------
 * Event group dispatch
 * ---------------------------------------------------------------- */

void *SPP_Osal_eventCreate(void)
{
    const SPP_OsalPort_t *p_port = getPort();
    if ((p_port == NULL) || (p_port->eventCreate == NULL))
    {
        return NULL;
    }
    return p_port->eventCreate();
}

retval_t SPP_Osal_eventWait(void *p_e, spp_uint32_t bits,
                              spp_bool_t clearOnExit, spp_bool_t waitAll,
                              spp_uint32_t timeoutMs,
                              spp_uint32_t *p_actualBits)
{
    const SPP_OsalPort_t *p_port = getPort();
    if ((p_port == NULL) || (p_port->eventWait == NULL))
    {
        return SPP_ERROR_NO_PORT;
    }
    return p_port->eventWait(p_e, bits, clearOnExit, waitAll,
                             timeoutMs, p_actualBits);
}

retval_t SPP_Osal_eventSetFromIsr(void *p_e, spp_uint32_t bits,
                                   spp_uint32_t *p_prev,
                                   spp_bool_t   *p_yield)
{
    const SPP_OsalPort_t *p_port = getPort();
    if ((p_port == NULL) || (p_port->eventSetFromIsr == NULL))
    {
        return SPP_ERROR_NO_PORT;
    }
    return p_port->eventSetFromIsr(p_e, bits, p_prev, p_yield);
}
