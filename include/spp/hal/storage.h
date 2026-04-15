/**
 * @file storage.h
 * @brief SPP storage HAL API — dispatches through the registered HAL port.
 *
 * Naming conventions used in this file:
 * - Public functions: SPP_HAL_storage*()
 * - Pointer parameters: p_*
 */

#ifndef SPP_HAL_STORAGE_H
#define SPP_HAL_STORAGE_H

#include "spp/core/types.h"
#include "spp/core/returntypes.h"

/* ----------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------- */

/**
 * @brief Mount the storage filesystem.
 *
 * Safe to call multiple times when already mounted.
 *
 * @param[in] p_cfg  Pointer to @ref SPP_StorageInitCfg_t with mount parameters.
 *
 * @return K_SPP_OK on success, K_SPP_ERROR on failure.
 */
SPP_RetVal_t SPP_HAL_storageMount(void *p_cfg);

/**
 * @brief Unmount the storage filesystem.
 *
 * Safe to call when not mounted.
 *
 * @param[in] p_cfg  Pointer to @ref SPP_StorageInitCfg_t.
 *
 * @return K_SPP_OK on success.
 */
SPP_RetVal_t SPP_HAL_storageUnmount(void *p_cfg);

#endif /* SPP_HAL_STORAGE_H */
