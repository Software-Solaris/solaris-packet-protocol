/**
 * @file datalogger.h
 * @brief SD card packet logger service.
 */

#ifndef SPP_DATALOGGER_H
#define SPP_DATALOGGER_H

#include "spp/core/returnTypes.h"
#include "spp/core/packet.h"
#include "spp/services/service.h"

/* ----------------------------------------------------------------
 * DEFINES
 * ---------------------------------------------------------------- */

#define K_DATALOGGER_CONSUMER_ID   (0x01U) /**< Unique consumer ID for the datalogger. */
#define K_DATALOGGER_CONSUMER_PRIO (0)     /**< Consumer priority (0 = lowest). */
#define K_DATALOGGER_TIMEOUT_MS    (1000U) /**< Mailbox receive timeout in ms. */
#define K_DATALOGGER_FLUSH_EVERY   (20U)   /**< Flush to SD every N packets. */
#define K_DATALOGGER_SPI_DEV_IDX   (2U)    /**< SPI device index for the SD card. */
#define K_DATALOGGER_QUEUE_SIZE    (16U)
#define K_DATALOGGER_SECTOR_SIZE   (512U)
#define K_DATALOGGER_BUFFER_SIZE   (2U * K_DATALOGGER_SECTOR_SIZE)

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
    void *p_spiHandler;                           /* SD handler */
    spp_uint8_t buffer[K_DATALOGGER_BUFFER_SIZE]; /* Ping-pong buffer for packet accumulation */
    spp_uint16_t writeIndex;                      /* Current write position in buffer */
    spp_uint32_t currentSector;                   /* Next SD sector to write */
    spp_bool_t txInProgress;                      /* SD write in progress flag */
    spp_bool_t halfPending;                       /* Indicates a half-buffer is being flushed */
    spp_uint8_t pendingHalf;                      /* 0 = first half, 1 = second half */
    spp_uint32_t loggedPackets;                   /* Total packets successfully logged */

} Datalogger_t;

/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */

const SPP_SERVICE_ConsumerContract_t *SPP_SERVICES_DATALOGGER_getConsumerContract(void);

#endif /* SPP_DATALOGGER_H */
