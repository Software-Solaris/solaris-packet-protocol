/**
 * @file eventgroups.h
 * @brief OSAL Event Groups Interface
 * @version 1.0.0
 * @date 2024
 *
 * This file provides the event groups interface for the OSAL layer.
 */

#ifndef SPP_OSAL_EVENTGROUPS_H
#define SPP_OSAL_EVENTGROUPS_H

#include "spp/core/returntypes.h"
#include "spp/core/types.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Event bits type
     */
    typedef spp_uint32_t osal_eventbits_t;

    /* ============================================================================
 *  Event Group Storage
 * ========================================================================= */

    /**
     * @brief Get a pointer to the next available event group buffer
     *
     * Returns a pointer to a pre-allocated event group buffer from the
     * static pool. Used for static event group creation.
     *
     * @return void* Pointer to available event group buffer, NULL if none available
     */
    void *SPP_OSAL_GetEventGroupsBuffer();

    /* ============================================================================
 *  Event Group Lifecycle
 * ========================================================================= */

    /**
     * @brief Create a new event group
     *
     * Creates an event group using the provided buffer for static allocation.
     *
     * @param[in] event_group_buffer Pointer to pre-allocated event group buffer
     *
     * @return void* Handle to the created event group, NULL on failure
     */
    void *SPP_OSAL_EventGroupCreate(void *event_group_buffer);

    /* ============================================================================
 *  Event Group Operations
 * ========================================================================= */

    /**
     * @brief Wait for specific bits to be set in an event group
     *
     * Blocks the calling task until the specified bits are set, or the
     * timeout expires.
     *
     * @param[in]  event_group       Event group handle
     * @param[in]  bits_to_wait      Bitmask of bits to wait for
     * @param[in]  clear_on_exit     If non-zero, clear the bits before returning
     * @param[in]  wait_for_all_bits If non-zero, wait for all bits; otherwise wait for any
     * @param[in]  timeout_ms        Timeout in milliseconds
     * @param[out] actual_bits       Pointer to store the bits that were actually set
     *
     * @return retval_t SPP_OK on success, SPP_ERROR on timeout or failure
     */
    retval_t OSAL_EventGroupWaitBits(void *event_group, osal_eventbits_t bits_to_wait,
                                     spp_uint8_t clear_on_exit, spp_uint8_t wait_for_all_bits,
                                     spp_uint32_t timeout_ms, osal_eventbits_t *actual_bits);

    /**
     * @brief Set bits in an event group from an ISR context
     *
     * Sets the specified bits in the event group. Safe to call from an
     * interrupt service routine.
     *
     * @param[in]  event_group              Event group handle
     * @param[in]  bits_to_set              Bitmask of bits to set
     * @param[out] previous_bits            Pointer to store the previous bits value
     * @param[out] higher_priority_task_woken Pointer to flag for context switch request
     *
     * @return retval_t SPP_OK on success, error code otherwise
     */
    retval_t OSAL_EventGroupSetBitsFromISR(void *event_group, osal_eventbits_t bits_to_set,
                                           osal_eventbits_t *previous_bits,
                                           spp_uint8_t *higher_priority_task_woken);

#ifdef __cplusplus
}
#endif

#endif /* SPP_OSAL_EVENTGROUPS_H */
