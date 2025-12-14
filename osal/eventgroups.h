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

#include "core/returntypes.h"
#include "core/types.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Event bits type
 */
typedef spp_uint32_t osal_eventbits_t;


void* SPP_OSAL_GetEventGroupsBuffer();

void* SPP_OSAL_EventGroupCreate(void* event_group_buffer);

retval_t OSAL_EventGroupSetBitsFromISR(void* event_group,
                                      osal_eventbits_t bits_to_set,
                                      osal_eventbits_t* previous_bits,
                                      spp_uint8_t* higher_priority_task_woken);

retval_t OSAL_EventGroupWaitBits(void* event_group,
                                 osal_eventbits_t bits_to_wait,
                                 spp_uint8_t clear_on_exit,
                                 spp_uint8_t wait_for_all_bits,
                                 spp_uint32_t timeout_ms,
                                 osal_eventbits_t* actual_bits);
                                 
// /**
//  * @brief Set bits in an event group
//  * 
//  * @param event_group Event group handle
//  * @param bits_to_set Bits to set
//  * @param[out] previous_bits Pointer to store the previous bits value (optional)
//  * @return retval_t SPP_OK on success, error code otherwise
//  */
// retval_t OSAL_EventGroupSetBits(osal_eventgroup_handle_t event_group, 
//                                osal_eventbits_t bits_to_set,
//                                osal_eventbits_t* previous_bits);

// /**
//  * @brief Set bits in an event group from ISR
//  * 
//  * @param event_group Event group handle
//  * @param bits_to_set Bits to set
//  * @param[out] previous_bits Pointer to store the previous bits value (optional)
//  * @param higher_priority_task_woken Pointer to flag that indicates if a context switch should be performed
//  * @return retval_t SPP_OK on success, error code otherwise
//  */
// retval_t OSAL_EventGroupSetBitsFromISR(osal_eventgroup_handle_t event_group,
//                                       osal_eventbits_t bits_to_set,
//                                       osal_eventbits_t* previous_bits,
//                                       spp_uint8_t* higher_priority_task_woken);

// /**
//  * @brief Wait for bits to be set in an event group
//  * 
//  * @param event_group Event group handle
//  * @param bits_to_wait Bits to wait for
//  * @param clear_on_exit Clear the bits on exit if true
//  * @param wait_for_all_bits Wait for all bits to be set if true, any bit if false
//  * @param timeout_ms Timeout in milliseconds
//  * @param[out] actual_bits Pointer to store the bits that were set
//  * @return retval_t SPP_OK on success, SPP_ERROR on timeout, other error codes otherwise
//  */
// retval_t OSAL_EventGroupWaitBits(osal_eventgroup_handle_t event_group,
//                                 osal_eventbits_t bits_to_wait,
//                                 spp_uint8_t clear_on_exit,
//                                 spp_uint8_t wait_for_all_bits,
//                                 spp_uint32_t timeout_ms,
//                                 osal_eventbits_t* actual_bits);

// /**
//  * @brief Synchronize tasks using event group
//  * 
//  * @param event_group Event group handle
//  * @param bits_to_set Bits to set
//  * @param bits_to_wait Bits to wait for
//  * @param timeout_ms Timeout in milliseconds
//  * @param[out] achieved_bits Pointer to store the bits that were achieved
//  * @return retval_t SPP_OK on success, SPP_ERROR on timeout, other error codes otherwise
//  */
// retval_t OSAL_EventGroupSync(osal_eventgroup_handle_t event_group,
//                             osal_eventbits_t bits_to_set,
//                             osal_eventbits_t bits_to_wait,
//                             spp_uint32_t timeout_ms,
//                             osal_eventbits_t* achieved_bits);

// /**
//  * @brief Clear bits in an event group
//  * 
//  * @param event_group Event group handle
//  * @param bits_to_clear Bits to clear
//  * @param[out] previous_bits Pointer to store the previous bits value (optional)
//  * @return retval_t SPP_OK on success, error code otherwise
//  */
// retval_t OSAL_EventGroupClearBits(osal_eventgroup_handle_t event_group,
//                                  osal_eventbits_t bits_to_clear,
//                                  osal_eventbits_t* previous_bits);

// /**
//  * @brief Get the current bits of an event group
//  * 
//  * @param event_group Event group handle
//  * @param[out] current_bits Pointer to store the current bits
//  * @return retval_t SPP_OK on success, error code otherwise
//  */
// retval_t OSAL_EventGroupGetBits(osal_eventgroup_handle_t event_group,
//                                osal_eventbits_t* current_bits);

// /**
//  * @brief Get the current bits of an event group from ISR
//  * 
//  * @param event_group Event group handle
//  * @param[out] current_bits Pointer to store the current bits
//  * @return retval_t SPP_OK on success, error code otherwise
//  */
// retval_t OSAL_EventGroupGetBitsFromISR(osal_eventgroup_handle_t event_group,
//                                       osal_eventbits_t* current_bits);

// /**
//  * @brief Delete an event group
//  * 
//  * @param event_group Event group handle to delete
//  * @return retval_t SPP_OK on success, error code otherwise
//  */
// retval_t OSAL_EventGroupDelete(osal_eventgroup_handle_t event_group);

#ifdef __cplusplus
}
#endif

#endif /* SPP_OSAL_EVENTGROUPS_H */