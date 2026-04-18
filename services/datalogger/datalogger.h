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

#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ----------------------------------------------------------------
 * Data types
 * ---------------------------------------------------------------- */

/**
 * @brief Datalogger instance context.
 *
 * Caller allocates this on the stack or as a static variable and passes a
 * pointer to all DATALOGGER_* calls.
 */
typedef struct
{
    void    *p_storage_cfg; /**< Storage config pointer (owned by caller). */
    FILE    *p_file;        /**< Open file handle, or NULL if not open.    */
    uint8_t  is_initialized; /**< 1 when mounted and file is open.         */
    uint32_t logged_packets; /**< Number of packets written so far.        */
} Datalogger_t;

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
SPP_RetVal_t SPP_SERVICES_DATALOGGER_init(Datalogger_t *p_logger, void *p_storage_cfg,
                         const char *p_file_path);

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
