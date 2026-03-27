/**
 * @file eventgroups.c
 * @brief OSAL Event Groups Implementation
 * @version 1.0.0
 * @date 2024
 *
 * This file provides the default (weak) implementation of event groups functions.
 */

#include "spp/osal/eventgroups.h"
#include "spp/core/returntypes.h"
#include <stddef.h>

/* ============================================================================
 *  Event Group Storage
 * ========================================================================= */

/**
 * @brief Default (weak) implementation of event group buffer retrieval
 *
 * @return void* Always returns NULL in the default implementation
 */
__attribute__((weak)) void *SPP_OSAL_GetEventGroupsBuffer()
{
    return NULL;
}

/* ============================================================================
 *  Event Group Lifecycle
 * ========================================================================= */

/**
 * @brief Default (weak) implementation of event group creation
 *
 * @param[in] p_event_group        Pointer to store the created event group handle
 * @param[in] p_buffer_event_group Pointer to pre-allocated event group buffer
 *
 * @return retval_t SPP_ERROR_NOT_INITIALIZED in the default implementation
 */
__attribute__((weak)) retval_t OSAL_EventGroupCreate(void *p_event_group,
                                                     void *p_buffer_event_group)
{
    return SPP_ERROR_NOT_INITIALIZED;
}
