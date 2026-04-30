/**
 * @file datalogger.h
 * @brief SD card packet logger service.
 *
 * Provides a thin wrapper around the SPP storage HAL that opens a text file
 * on the SD card and writes human-readable packet records to it.
 *
 * Naming conventions used in this file:
 * - Constants/macros: K_SPP_*
 * - Types: Datalogger_t
 * - Public functions: SPP_SERVICES_DATALOGGER_*()
 * - Pointer parameters: p_*
 */

#ifndef SPP_DATALOGGER_H
#define SPP_DATALOGGER_H

#include "spp/core/returnTypes.h"
#include "spp/core/packet.h"
#include "spp/services/service.h"
#include "spp/util/macros.h"

#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ----------------------------------------------------------------
 * Data types
 * ---------------------------------------------------------------- */

/**
 * @brief SD logger instance.
 *
 * Declare one static instance with the storage config fields filled in, then
 * pass its address to SPP_SERVICES_register().
 *
 * @code
 * static Datalogger_t s_logger = {
 *     .p_storageCfg = &s_storageCfg,
 *     .p_filePath   = "/sdcard/log.txt",
 * };
 * @endcode
 */
typedef struct
{
    /* Configuration — set at declaration */
    void       *p_storageCfg; /**< Pointer to SPP_StorageInitCfg_t.         */
    const char *p_filePath;   /**< Absolute path of the file to write.      */

    /* Runtime state — filled in by init, do not set manually */
    FILE    *p_file;          /**< Open file handle, or NULL if not open.   */
    uint8_t  is_initialized;  /**< 1 when mounted and file is open.         */
    uint32_t logged_packets;  /**< Number of packets written so far.        */
} Datalogger_t;

/**
 * @brief SD card logger module descriptor — pass to SPP_SERVICES_register().
 *
 * Subscribes to K_SPP_APID_ALL at K_SPP_PUBSUB_PRIO_LOW; every published
 * packet is appended to the log file.
 */
extern const SPP_Module_t g_sdLoggerModule;

/* ----------------------------------------------------------------
 * API
 * ---------------------------------------------------------------- */

/**
 * @brief Mount the SD card and open the log file for writing.
 *
 * @param[out] p_logger       Pointer to the datalogger context to initialise.
 * @param[in]  p_storage_cfg  Pointer to a @ref SPP_StorageInitCfg_t (or NULL
 *                            if the filesystem is already mounted).
 * @param[in]  p_file_path    Absolute path of the file to create/overwrite.
 *
 * @return K_SPP_OK on success, or an error code otherwise.
 */
SPP_RetVal_t SPP_SERVICES_DATALOGGER_init(Datalogger_t *p_logger);

/**
 * @brief Write a formatted record for @p p_packet to the log file.
 *
 * @param[in,out] p_logger  Datalogger context.
 * @param[in]     p_packet  Packet to log.
 *
 * @return K_SPP_OK on success, K_SPP_ERROR on write failure.
 */
SPP_RetVal_t SPP_SERVICES_DATALOGGER_logPacket(Datalogger_t *p_logger, const SPP_Packet_t *p_packet);

/**
 * @brief Flush buffered data to the SD card.
 *
 * @param[in,out] p_logger  Datalogger context.
 *
 * @return K_SPP_OK on success, K_SPP_ERROR on flush failure.
 */
SPP_RetVal_t SPP_SERVICES_DATALOGGER_flush(Datalogger_t *p_logger);

/**
 * @brief Close the log file and unmount the SD card.
 *
 * @param[in,out] p_logger  Datalogger context.
 *
 * @return K_SPP_OK on success, or an error code otherwise.
 */
SPP_RetVal_t SPP_SERVICES_DATALOGGER_deinit(Datalogger_t *p_logger);

#ifdef __cplusplus
}
#endif

#endif /* SPP_DATALOGGER_H */
