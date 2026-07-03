/**
 * @file datalogger.c
 * @brief SD card packet logger — SPP consumer service implementation.
 */

#include "spp/services/datalogger/datalogger.h"

#include "spp/hal/storage/storage.h"
#include "spp/hal/spi/spi.h"
#include "spp/core/packet.h"
#include "spp/services/log/log.h"
#include "spp/core/types.h"

#include <string.h>

/* ----------------------------------------------------------------
* CONSTANTS
* ---------------------------------------------------------------- */
static SPP_Packet_t s_packetBuffer[K_DATALOGGER_BUFFER_SIZE];
static spp_uint8_t s_packetIndex = 0;        /**< Index of the next packet to be written to the buffer. */
static spp_uint32_t s_currentDataSector = 0; /**< Current sector of the data log. */
static spp_uint16_t s_blocksWritten = 0;     /**< Number of blocks written to the current sector. */


/* ----------------------------------------------------------------
 * STATIC FUNCTIONS DECLARATIONS
 * ---------------------------------------------------------------- */
static SPP_RetVal_t SPP_SERVICES_DATALOGGER_init(void);
static SPP_RetVal_t SPP_SERVICES_DATALOGGER_deliverToMailbox(const SPP_Packet_t packet);
static SPP_RetVal_t SPP_SERVICES_DATALOGGER_consumeData(void *p_data);

/* ----------------------------------------------------------------
 * VARIABLES
 * ---------------------------------------------------------------- */
static const SPP_SERVICE_ConsumerContract_t g_dataloggerConsumerContract = {
    .consumerID = K_DATALOGGER_CONSUMER_ID,
    .priority = K_DATALOGGER_CONSUMER_PRIO,
    .p_nameConsumer = "datalogger",
    .tiemoutMs = K_DATALOGGER_TIMEOUT_MS,
    .suscribeToApid = 0U,
    .init = SPP_SERVICES_DATALOGGER_init,
    .deliverToMailbox = SPP_SERVICES_DATALOGGER_deliverToMailbox,
    .consumeData = SPP_SERVICES_DATALOGGER_consumeData,
};

/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */
const SPP_SERVICE_ConsumerContract_t *SPP_SERVICES_DATALOGGER_getConsumerContract(void)
{
    return &g_dataloggerConsumerContract;
}

/* ----------------------------------------------------------------
 * STATIC FUNCTIONS
 * ---------------------------------------------------------------- */
/**
 * @brief Initialise the SD card and open the log file for writing.
 * @return K_SPP_OK on success, K_SPP_ERROR on mount or file-open failure.
 */
static SPP_RetVal_t SPP_SERVICES_DATALOGGER_init(void)
{
    /* Init the variables */
    s_currentDataSector = K_DATALOGGER_FIRST_SECTOR;
    s_packetIndex = 0;
    s_blocksWritten = 1;

    /* Init SPI for SD card */
    SPP_RetVal_t ret = SPP_HAL_STORAGE_init();
    if (ret != K_SPP_OK)
    {
        return K_SPP_ERROR;
    }
    return K_SPP_OK;
}

/**
 * @brief Receive a packet from the pub/sub router into the consumer mailbox.
 * @param  p_pkt  Packet dispatched by the router.
 * @return K_SPP_OK on success, K_SPP_ERROR_NULL_POINTER if p_pkt is NULL.
 */
static SPP_RetVal_t SPP_SERVICES_DATALOGGER_deliverToMailbox(const SPP_Packet_t packet)
{
    /* Copy the packets into the buffer*/
    s_packetBuffer[s_packetIndex] = packet;
    s_packetIndex++;
    return K_SPP_OK;
}

/**
 * @brief Consume a packet from the mailbox and write it to the SD card log file.
 * @param  p_data  Unused (context pointer reserved for future use).
 * @return K_SPP_OK on success, K_SPP_ERROR if the file is not open or write fails.
 */
static SPP_RetVal_t SPP_SERVICES_DATALOGGER_consumeData(void *p_data)
{
    if ((s_packetIndex * K_SPP_TOTAL_PACKET_SIZE) >= K_DATALOGGER_BUFFER_SIZE)
    {
        SPP_RetVal_t ret = SPP_HAL_STORAGE_write(s_packetBuffer, s_currentDataSector, s_blocksWritten);
        if (ret == K_SPP_OK)
        {
            s_currentDataSector++;
            s_blocksWritten++;
            s_packetIndex = 0;
        }
    }

    return K_SPP_OK;
}
