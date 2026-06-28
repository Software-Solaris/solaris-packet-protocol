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

/* ----------------------------------------------------------------
 * STATIC FUNCTIONS DECLARATIONS
 * ---------------------------------------------------------------- */
static SPP_RetVal_t SPP_SERVICES_DATALOGGER_init(void);
static SPP_RetVal_t SPP_SERVICES_DATALOGGER_deliverToMailbox(const SPP_Packet_t *p_packet);
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

static Datalogger_t s_datalogger;
//static const char *const k_tag = "DATALOGGER";

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
    s_datalogger.currentSector = K_DATALOGGER_FIRST_SECTOR;
    s_datalogger.pendingHalf = K_DATALOGGER_NO_PENDING;

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
static SPP_RetVal_t SPP_SERVICES_DATALOGGER_deliverToMailbox(const SPP_Packet_t *p_packet)
{
    if (p_packet == NULL)
    {
        return K_SPP_ERROR_NULL_POINTER;
    }

    if ((K_DATALOGGER_SECTOR_SIZE - s_datalogger.writeIndex) < sizeof(SPP_Packet_t))
    {
        s_datalogger.pendingHalf = s_datalogger.activeHalf;
        if (s_datalogger.activeHalf == 0U)
        {
            s_datalogger.activeHalf = 1U;
        }
        else
        {
            s_datalogger.activeHalf = 0U;
        }

        s_datalogger.writeIndex = 0U;
    }

    memcpy(&s_datalogger.buffer[s_datalogger.activeHalf * K_DATALOGGER_SECTOR_SIZE] + s_datalogger.writeIndex, p_packet,
           sizeof(SPP_Packet_t));

    s_datalogger.writeIndex += (spp_uint16_t)sizeof(SPP_Packet_t);
    return K_SPP_OK;
}

/**
 * @brief Consume a packet from the mailbox and write it to the SD card log file.
 * @param  p_data  Unused (context pointer reserved for future use).
 * @return K_SPP_OK on success, K_SPP_ERROR if the file is not open or write fails.
 */
static SPP_RetVal_t SPP_SERVICES_DATALOGGER_consumeData(void *p_data)
{
    if (s_datalogger.pendingHalf == K_DATALOGGER_NO_PENDING)
    {
        return K_SPP_OK;
    }

    SPP_RetVal_t ret = SPP_HAL_STORAGE_write(&s_datalogger.buffer[s_datalogger.pendingHalf * K_DATALOGGER_SECTOR_SIZE],
                                             s_datalogger.currentSector, 1U);

    if (ret != K_SPP_OK)
    {
        return K_SPP_ERROR;
    }

    s_datalogger.currentSector++;
    s_datalogger.pendingHalf = K_DATALOGGER_NO_PENDING;

    return K_SPP_OK;
}
