/**
 * @file storage.h
 * @brief Storage Hardware Abstraction Layer
 * @version 1.0.0
 * @date 2024
 *
 * This file provides the storage HAL interface for mounting and
 * unmounting an SD-card-backed filesystem.
 */

#ifndef SPP_HAL_STORAGE_H
#define SPP_HAL_STORAGE_H

/* ---------------------------------------------------------------- */
/*  Includes                                                        */
/* ---------------------------------------------------------------- */

#include "spp/core/returntypes.h"
#include "spp/core/types.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /* ---------------------------------------------------------------- */
    /*  Types                                                           */
    /* ---------------------------------------------------------------- */

    /**
     * @brief  Configuration structure for storage initialisation.
     */
    typedef struct
    {
        const char *p_base_path;           /**< Filesystem mount point.            */
        int spi_host_id;                   /**< SPI host used for the SD card.     */
        int pin_cs;                        /**< Chip-select GPIO pin.              */
        spp_uint32_t max_files;            /**< Maximum number of open files.      */
        spp_uint32_t allocation_unit_size; /**< FAT allocation unit size in bytes. */
        spp_bool_t format_if_mount_failed; /**< Format the card on mount failure. */
    } SPP_Storage_InitCfg;

    /* ---------------------------------------------------------------- */
    /*  Public function declarations                                    */
    /* ---------------------------------------------------------------- */

    /**
     * @brief  Mount the storage filesystem.
     *
     * @param[in] p_cfg  Pointer to an SPP_Storage_InitCfg configuration.
     *
     * @return retval_t  SPP_OK on success, error code otherwise.
     */
    retval_t SPP_HAL_Storage_Mount(void *p_cfg);

    /**
     * @brief  Unmount the storage filesystem.
     *
     * @param[in] p_cfg  Pointer to the same configuration used for mounting.
     *
     * @return retval_t  SPP_OK on success, error code otherwise.
     */
    retval_t SPP_HAL_Storage_Unmount(void *p_cfg);

#ifdef __cplusplus
}
#endif

#endif /* SPP_HAL_STORAGE_H */
