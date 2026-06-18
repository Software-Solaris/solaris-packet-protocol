/**
 * @file datalogger.h
 * @brief SD card packet logger service.
 *
 * Provides a consumer service that receives published packets through the
 * pub/sub router and writes them to a file on the SD card.
 *
 * The service follows a two-step workflow:
 * 1. Initialise the SD card and open the log file (SPP_SERVICES_DATALOGGER_init).
 * 2. Receive packets via the consumer contract (deliverToMailbox / consumeData).
 *
 * To use the SPP service registry, obtain the consumer contract via
 * SPP_SERVICES_DATALOGGER_getConsumerContract() and pass it to
 * SPP_SERVICES_PUBSUB_registerConsumer().
 *
 * Naming conventions used in this file:
 * - Constants/macros:  K_DATALOGGER_*
 * - Types:             Datalogger_*_t
 * - Public functions:  SPP_SERVICES_DATALOGGER_*()
 * - Pointer params:    p_*
 */

#ifndef SPP_DATALOGGER_H
#define SPP_DATALOGGER_H

#include "spp/core/returnTypes.h"
#include "spp/core/packet.h"
#include "spp/services/service.h"

/* ----------------------------------------------------------------
 * DEFINES
 * ---------------------------------------------------------------- */

#define K_DATALOGGER_CONSUMER_ID       (0x01U)   /**< Unique consumer ID for the datalogger. */
#define K_DATALOGGER_CONSUMER_PRIO     (0)        /**< Consumer priority (0 = lowest). */
#define K_DATALOGGER_TIMEOUT_MS        (1000U)    /**< Mailbox receive timeout in ms. */
#define K_DATALOGGER_FLUSH_EVERY       (20U)      /**< Flush to SD every N packets. */
#define K_DATALOGGER_SPI_DEV_IDX       (2U)       /**< SPI device index for the SD card. */

/* ----------------------------------------------------------------
 * STRUCTS AND ENUMS
 * ---------------------------------------------------------------- */

/**
 * @brief Datalogger instance.
 *
 * Declare one static instance with the storage config fields filled in, then
 * pass SPP_SERVICES_DATALOGGER_getConsumerContract() to
 * SPP_SERVICES_PUBSUB_registerConsumer(). All other fields are zero-initialised
 * by the compiler and filled in by the init callback.
 */
typedef struct
{
    void       *p_storageCfg;    /**< Pointer to SPP_StorageInitCfg_t.       */
    const char *p_filePath;      /**< Absolute path of the file to write.    */
    void       *p_spiHandler;    /**< SPI device handle for the SD card.     */
    void       *p_file;          /**< Open file handle, NULL if not open.    */
    spp_bool_t  isOpen;          /**< true once mounted and file is open.    */
    spp_uint32_t loggedPackets;  /**< Number of packets written so far.      */
} Datalogger_t;

/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */

const SPP_SERVICE_ConsumerContract_t *SPP_SERVICES_DATALOGGER_getConsumerContract(void);

#endif /* SPP_DATALOGGER_H */
