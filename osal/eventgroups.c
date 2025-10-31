/**
 * @file eventgroups.c
 * @brief OSAL Event Groups Implementation
 * @version 1.0.0
 * @date 2024
 * 
 * This file provides the default (weak) implementation of event groups functions.
 */

#include "eventgroups.h"
#include "core/returntypes.h"

/**
 * @brief Default (weak) event group creation
 */
__attribute__((weak)) retval_t OSAL_EventGroupCreate(osal_eventgroup_handle_t* event_group)
{
    if (event_group != NULL) {
        *event_group = NULL;
    }
    return SPP_ERROR_NOT_INITIALIZED;
}

/**
 * @brief Default (weak) event group set bits
 */
__attribute__((weak)) retval_t OSAL_EventGroupSetBits(osal_eventgroup_handle_t event_group, 
                               osal_eventbits_t bits_to_set,
                               osal_eventbits_t* previous_bits)
{
    (void)event_group;
    (void)bits_to_set;
    if (previous_bits != NULL) {
        *previous_bits = 0;
    }
    return SPP_ERROR_NOT_INITIALIZED;
}

/**
 * @brief Default (weak) event group set bits from ISR
 */
__attribute__((weak)) retval_t OSAL_EventGroupSetBitsFromISR(osal_eventgroup_handle_t event_group,
                                      osal_eventbits_t bits_to_set,
                                      osal_eventbits_t* previous_bits,
                                      spp_uint8_t* higher_priority_task_woken)
{
    (void)event_group;
    (void)bits_to_set;
    if (previous_bits != NULL) {
        *previous_bits = 0;
    }
    if (higher_priority_task_woken != NULL) {
        *higher_priority_task_woken = 0;
    }
    return SPP_ERROR_NOT_INITIALIZED;
}

/**
 * @brief Default (weak) event group wait bits
 */
__attribute__((weak)) retval_t OSAL_EventGroupWaitBits(osal_eventgroup_handle_t event_group,
                                osal_eventbits_t bits_to_wait,
                                spp_uint8_t clear_on_exit,
                                spp_uint8_t wait_for_all_bits,
                                spp_uint32_t timeout_ms,
                                osal_eventbits_t* actual_bits)
{
    (void)event_group;
    (void)bits_to_wait;
    (void)clear_on_exit;
    (void)wait_for_all_bits;
    (void)timeout_ms;
    if (actual_bits != NULL) {
        *actual_bits = 0;
    }
    return SPP_ERROR_NOT_INITIALIZED;
}

/**
 * @brief Default (weak) event group sync
 */
__attribute__((weak)) retval_t OSAL_EventGroupSync(osal_eventgroup_handle_t event_group,
                            osal_eventbits_t bits_to_set,
                            osal_eventbits_t bits_to_wait,
                            spp_uint32_t timeout_ms,
                            osal_eventbits_t* achieved_bits)
{
    (void)event_group;
    (void)bits_to_set;
    (void)bits_to_wait;
    (void)timeout_ms;
    if (achieved_bits != NULL) {
        *achieved_bits = 0;
    }
    return SPP_ERROR_NOT_INITIALIZED;
}

/**
 * @brief Default (weak) event group clear bits
 */
__attribute__((weak)) retval_t OSAL_EventGroupClearBits(osal_eventgroup_handle_t event_group,
                                 osal_eventbits_t bits_to_clear,
                                 osal_eventbits_t* previous_bits)
{
    (void)event_group;
    (void)bits_to_clear;
    if (previous_bits != NULL) {
        *previous_bits = 0;
    }
    return SPP_ERROR_NOT_INITIALIZED;
}

/**
 * @brief Default (weak) event group get bits
 */
__attribute__((weak)) retval_t OSAL_EventGroupGetBits(osal_eventgroup_handle_t event_group,
                               osal_eventbits_t* current_bits)
{
    (void)event_group;
    if (current_bits != NULL) {
        *current_bits = 0;
    }
    return SPP_ERROR_NOT_INITIALIZED;
}

/**
 * @brief Default (weak) event group get bits from ISR
 */
__attribute__((weak)) retval_t OSAL_EventGroupGetBitsFromISR(osal_eventgroup_handle_t event_group,
                                      osal_eventbits_t* current_bits)
{
    (void)event_group;
    if (current_bits != NULL) {
        *current_bits = 0;
    }
    return SPP_ERROR_NOT_INITIALIZED;
}

/**
 * @brief Default (weak) event group delete
 */
__attribute__((weak)) retval_t OSAL_EventGroupDelete(osal_eventgroup_handle_t event_group)
{
    (void)event_group;
    return SPP_ERROR_NOT_INITIALIZED;
}