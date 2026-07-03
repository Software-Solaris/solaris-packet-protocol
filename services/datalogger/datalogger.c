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
<<<<<<< HEAD
    SPP_RetVal_t ret = SPP_HAL_storageMount(p_logger->p_storageCfg);

    if (ret != K_SPP_OK)
    {
        SPP_LOGE(k_tag, "Mount failed");
        return ret;
    }

    p_logger->p_file = fopen(p_logger->p_filePath, "w");

    if (p_logger->p_file == NULL)
    {
        SPP_LOGE(k_tag, "Cannot open %s", p_logger->p_filePath);
        (void)SPP_HAL_storageUnmount(p_logger->p_storageCfg);
        return K_SPP_ERROR;
    }

    fprintf(p_logger->p_file, "DATALOGGER START\n");
    fflush(p_logger->p_file);

    p_logger->is_open = true;
    p_logger->logged_packets = 0U;

    SPP_LOGI(k_tag, "Ready — logging to %s", p_logger->p_filePath);

    return K_SPP_OK;
}

SPP_RetVal_t SPP_SERVICES_DATALOGGER_flush(Datalogger_t *p_logger)
{
    if (!p_logger->is_open)
        return K_SPP_ERROR;

    if (fflush(p_logger->p_file) != 0)
    {
        SPP_LOGE(k_tag, "fflush failed");
=======
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
>>>>>>> dev/spp-refactor-code
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
<<<<<<< HEAD
    if (p_logger == NULL)
        return K_SPP_ERROR_NULL_POINTER;

    if (p_logger->is_open)
    {
        (void)fflush(p_logger->p_file);
        fclose(p_logger->p_file);
        p_logger->p_file = NULL;
        p_logger->is_open = false;
    }

    if (p_logger->p_storageCfg != NULL)
    {
        SPP_RetVal_t ret = SPP_HAL_storageUnmount(p_logger->p_storageCfg);
        if (ret != K_SPP_OK)
        {
            SPP_LOGE(k_tag, "Unmount failed");
            return ret;
        }
    }

    SPP_LOGI(k_tag, "Closed");
    return K_SPP_OK;
}

/* ----------------------------------------------------------------
 * Write one packet to the file
 * ---------------------------------------------------------------- */

SPP_RetVal_t SPP_SERVICES_DATALOGGER_logPacket(Datalogger_t *p_logger, const SPP_Packet_t *p_packet)
{
    if (!p_logger->is_open)
        return K_SPP_ERROR;

    int n;

    if (p_packet->primaryHeader.apid == K_SPP_APID_LOG)
    {
        /* Log message — payload is a null-terminated string, write as-is. */
        n = fprintf(p_logger->p_file, "%.*s\n", (int)p_packet->primaryHeader.payloadLen,
                    (const char *)p_packet->payload);
    }
    else
    {
        /* Sensor packet — write header fields then payload bytes as hex. */
        n = fprintf(p_logger->p_file, "ts=%lu apid=0x%04X seq=%u len=%u payload_hex=",
                    (unsigned long)p_packet->secondaryHeader.timestampMs,
                    (unsigned)p_packet->primaryHeader.apid, (unsigned)p_packet->primaryHeader.seq,
                    (unsigned)p_packet->primaryHeader.payloadLen);
        if (n < 0)
            return K_SPP_ERROR;

        for (spp_uint16_t i = 0U; i < p_packet->primaryHeader.payloadLen; i++)
        {
            (void)fprintf(p_logger->p_file, "%s%02X", (i > 0U) ? " " : "",
                          (unsigned)p_packet->payload[i]);
=======
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
>>>>>>> dev/spp-refactor-code
        }
    }

<<<<<<< HEAD
    if (n < 0)
        return K_SPP_ERROR;

    p_logger->logged_packets++;
    return K_SPP_OK;
}

/* ----------------------------------------------------------------
 * Module descriptor — called by register(), never by main.c directly
 * ---------------------------------------------------------------- */

static void dataloggerOnPacket(const SPP_Packet_t *p_packet, void *p_ctx)
{
    Datalogger_t *p_logger = (Datalogger_t *)p_ctx;

    (void)SPP_SERVICES_DATALOGGER_logPacket(p_logger, p_packet);

    /* Flush to SD card every K_FLUSH_EVERY packets.  Infrequent flushing lets
     * the C library buffer multiple writes, reducing microSD sector pressure. */
    if ((p_logger->logged_packets % K_FLUSH_EVERY) == 0U)
    {
        (void)SPP_SERVICES_DATALOGGER_flush(p_logger);
    }
}

static SPP_RetVal_t dataloggerInit(void *p_ctx)
{
    return SPP_SERVICES_DATALOGGER_init((Datalogger_t *)p_ctx);
}

static SPP_RetVal_t dataloggerStop(void *p_ctx)
{
    return SPP_SERVICES_DATALOGGER_flush((Datalogger_t *)p_ctx);
}

static SPP_RetVal_t dataloggerDeinit(void *p_ctx)
{
    return SPP_SERVICES_DATALOGGER_deinit((Datalogger_t *)p_ctx);
}

const SPP_Module_t g_sdLoggerModule = {
    .p_name = "sd_logger",
    .apid = K_SPP_APID_NONE, /* produces nothing          */
    .ctxSize = sizeof(Datalogger_t),
    .init = dataloggerInit,
    .start = NULL,
    .stop = dataloggerStop, /* flush on stop             */
    .deinit = dataloggerDeinit,
    .produce = NULL,                /* consumer only — no sensor */
    .consumesApid = K_SPP_APID_ALL, /* receives every packet     */
    .onPacket = dataloggerOnPacket,
    .onPacketPrio = K_SPP_PUBSUB_PRIO_LOW, /* deferred — never blocks sensors */
};
=======
    return K_SPP_OK;
}
>>>>>>> dev/spp-refactor-code
