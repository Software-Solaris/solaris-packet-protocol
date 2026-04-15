/**
 * @file storage.h
 * @brief SPP storage HAL API — dispatches through the registered HAL port.
 *
 * Naming conventions used in this file:
 * - Public functions: SPP_Hal_storage*()
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
 * @return SPP_OK on success, SPP_ERROR on failure.
 */
retval_t SPP_Hal_storageMount(void *p_cfg);

/**
 * @brief Unmount the storage filesystem.
 *
 * Safe to call when not mounted.
 *
 * @param[in] p_cfg  Pointer to @ref SPP_StorageInitCfg_t.
 *
 * @return SPP_OK on success.
 */
retval_t SPP_Hal_storageUnmount(void *p_cfg);

#endif /* SPP_HAL_STORAGE_H */
