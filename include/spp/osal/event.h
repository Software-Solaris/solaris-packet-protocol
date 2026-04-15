/**
 * @file event.h
 * @brief SPP event group API — dispatches through the registered OSAL port.
 *
 * Event groups provide a bitfield-based signalling mechanism safe to use
 * from both task and ISR contexts.
 *
 * Naming conventions used in this file:
 * - Types: SPP_GpioIsrCtx_t
 * - Public functions: SPP_Osal_event*()
 * - Pointer parameters: p_*
 */

#ifndef SPP_OSAL_EVENT_H
#define SPP_OSAL_EVENT_H

#include "spp/core/types.h"
#include "spp/core/returntypes.h"

/* ----------------------------------------------------------------
 * ISR context helper type
 * ---------------------------------------------------------------- */

/**
 * @brief Context structure passed to a GPIO ISR.
 *
 * The ISR uses this to call @ref SPP_Osal_eventSetFromIsr() with the
 * correct event group handle and bit mask.
 */
typedef struct
{
    void         *p_eventGroup; /**< Event group handle created by SPP_Osal_eventCreate(). */
    spp_uint32_t  bits;         /**< Bitmask of event bits to set from the ISR.            */
} SPP_GpioIsrCtx_t;

/* ----------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------- */

/**
 * @brief Create an event group.
 *
 * @return Opaque event group handle on success, NULL on failure.
 */
void *SPP_Osal_eventCreate(void);

/**
 * @brief Wait for event bits from a task context.
 *
 * @param[in]  p_e          Event group handle.
 * @param[in]  bits         Bitmask of bits to wait for.
 * @param[in]  clearOnExit  If true, clear matched bits before returning.
 * @param[in]  waitAll      If true, wait for ALL bits; else any bit suffices.
 * @param[in]  timeoutMs    Maximum wait time in ms.
 * @param[out] p_actualBits Bits set at the time of return (may be NULL).
 *
 * @return SPP_OK when satisfied, SPP_ERROR_TIMEOUT otherwise.
 */
retval_t SPP_Osal_eventWait(void *p_e, spp_uint32_t bits,
                              spp_bool_t clearOnExit, spp_bool_t waitAll,
                              spp_uint32_t timeoutMs,
                              spp_uint32_t *p_actualBits);

/**
 * @brief Set event bits from an ISR context.
 *
 * @param[in]  p_e      Event group handle.
 * @param[in]  bits     Bitmask of bits to set.
 * @param[out] p_prev   Previous value of the bits (may be NULL).
 * @param[out] p_yield  Set to true if a higher-priority task was unblocked
 *                      (caller should trigger a context switch).
 *
 * @return SPP_OK on success.
 */
retval_t SPP_Osal_eventSetFromIsr(void *p_e, spp_uint32_t bits,
                                   spp_uint32_t *p_prev,
                                   spp_bool_t   *p_yield);

#endif /* SPP_OSAL_EVENT_H */
