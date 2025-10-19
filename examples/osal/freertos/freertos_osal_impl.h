/**
 * @file freertos_osal_impl.h
 * @brief FreeRTOS OSAL Implementation for SPP
 * @version 1.0.0
 * @date 2024
 * 
 * This file provides FreeRTOS specific OSAL implementation for the SPP layer.
 */

#ifndef SPP_FREERTOS_OSAL_IMPL_H
#define SPP_FREERTOS_OSAL_IMPL_H

#include "osal/osal.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize FreeRTOS OSAL implementation
 * 
 * This function should be called before using any OSAL functions
 * to ensure FreeRTOS-specific initialization is complete.
 * 
 * @return retval_t SPP_OK on success, error code otherwise
 */
retval_t FreeRTOS_OSAL_Init(void);

/**
 * @brief Deinitialize FreeRTOS OSAL implementation
 * 
 * @return retval_t SPP_OK on success, error code otherwise
 */
retval_t FreeRTOS_OSAL_Deinit(void);

/**
 * @brief Check if FreeRTOS is available
 * 
 * @return bool true if FreeRTOS is available, false otherwise
 */
bool FreeRTOS_OSAL_IsAvailable(void);

/**
 * @brief Get FreeRTOS version information
 * 
 * @param version_string Buffer to store version string
 * @param buffer_size Size of the buffer
 * @return retval_t SPP_OK on success, error code otherwise
 */
retval_t FreeRTOS_OSAL_GetVersion(char* version_string, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif /* SPP_FREERTOS_OSAL_IMPL_H */ 