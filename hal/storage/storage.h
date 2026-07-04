/**
 * @file storage.h
 * @brief SPP storage HAL API — dispatches through the registered HAL port.
 *
 */

#ifndef SPP_HAL_STORAGE_H
#define SPP_HAL_STORAGE_H

#include "spp/core/returnTypes.h"

/* ----------------------------------------------------------------
 * Storage configuration type
 * ---------------------------------------------------------------- */

/**
 * @brief SD card / filesystem mount configuration.
 *
 * Passed to @ref SPP_HAL_STORAGE_mount().
 */
typedef struct
{
    const char *p_basePath;          /**< VFS mount point (e.g. "/sdcard").  */
    int spiHostId;                   /**< SPI host used by the card.         */
    int pinCs;                       /**< Chip-select GPIO for the SD card.  */
    spp_uint32_t maxFiles;           /**< Maximum simultaneously open files. */
    spp_uint32_t allocationUnitSize; /**< FAT allocation unit size in bytes. */
    spp_bool_t formatIfMountFailed;  /**< Auto-format if mount fails.        */
} SPP_StorageInitCfg_t;

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
SPP_RetVal_t SPP_HAL_STORAGE_mount(void *p_cfg);

/**
 * @brief Unmount the storage filesystem.
 *
 * Safe to call when not mounted.
 *
 * @param[in] p_cfg  Pointer to @ref SPP_StorageInitCfg_t.
 *
 * @return K_SPP_OK on success.
 */
SPP_RetVal_t SPP_HAL_STORAGE_unmount(void *p_cfg);

#endif /* SPP_HAL_STORAGE_H */
