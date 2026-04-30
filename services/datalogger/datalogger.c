/**
 * @file datalogger.c
 * @brief SD card packet logger — writes every published packet to a text file.
 *
 * This module is a pure consumer: it has no produce() function and never reads
 * hardware directly.  It receives packets through pub/sub at PRIO_LOW, meaning
 * callConsumers() dispatches it one call at a time so SD card writes never
 * delay sensor reads.
 *
 * Log format:
 *   Log messages:   "[I] TAG: message text"
 *   Sensor packets: "ts=12345 apid=0x0004 seq=7 len=12 payload_hex=44 9A ..."
 *
 * Flush strategy: fflush() is called every K_FLUSH_EVERY packets.  Buffering
 * the writes in the C library reduces the number of physical SD card sectors
 * written per packet, which is the main bottleneck on a microSD card.
 */

#include "spp/services/datalogger/datalogger.h"

#include "spp/hal/storage.h"
#include "spp/core/packet.h"
#include "spp/services/log/log.h"
#include "spp/core/types.h"

#define K_FLUSH_EVERY (20U)

static const char *const k_tag = "DATALOGGER";

/* ----------------------------------------------------------------
 * Mount / open / close
 * ---------------------------------------------------------------- */

SPP_RetVal_t SPP_SERVICES_DATALOGGER_init(Datalogger_t *p_logger)
{
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

    p_logger->is_open       = true;
    p_logger->logged_packets = 0U;
    SPP_LOGI(k_tag, "Ready — logging to %s", p_logger->p_filePath);
    return K_SPP_OK;
}

SPP_RetVal_t SPP_SERVICES_DATALOGGER_flush(Datalogger_t *p_logger)
{
    if (!p_logger->is_open) return K_SPP_ERROR;

    if (fflush(p_logger->p_file) != 0)
    {
        SPP_LOGE(k_tag, "fflush failed");
        return K_SPP_ERROR;
    }
    return K_SPP_OK;
}

SPP_RetVal_t SPP_SERVICES_DATALOGGER_deinit(Datalogger_t *p_logger)
{
    if (p_logger == NULL) return K_SPP_ERROR_NULL_POINTER;

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

SPP_RetVal_t SPP_SERVICES_DATALOGGER_logPacket(Datalogger_t *p_logger,
                                                const SPP_Packet_t *p_packet)
{
    if (!p_logger->is_open) return K_SPP_ERROR;

    int n;

    if (p_packet->primaryHeader.apid == K_SPP_APID_LOG)
    {
        /* Log message — payload is a null-terminated string, write as-is. */
        n = fprintf(p_logger->p_file, "%.*s\n",
                    (int)p_packet->primaryHeader.payloadLen,
                    (const char *)p_packet->payload);
    }
    else
    {
        /* Sensor packet — write header fields then payload bytes as hex. */
        n = fprintf(p_logger->p_file,
                    "ts=%lu apid=0x%04X seq=%u len=%u payload_hex=",
                    (unsigned long)p_packet->secondaryHeader.timestampMs,
                    (unsigned)p_packet->primaryHeader.apid,
                    (unsigned)p_packet->primaryHeader.seq,
                    (unsigned)p_packet->primaryHeader.payloadLen);
        if (n < 0) return K_SPP_ERROR;

        for (spp_uint16_t i = 0U; i < p_packet->primaryHeader.payloadLen; i++)
        {
            (void)fprintf(p_logger->p_file, "%s%02X",
                          (i > 0U) ? " " : "",
                          (unsigned)p_packet->payload[i]);
        }
        n = fprintf(p_logger->p_file, "\n");
    }

    if (n < 0) return K_SPP_ERROR;

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
    .p_name       = "sd_logger",
    .apid         = K_SPP_APID_NONE,       /* produces nothing          */
    .ctxSize      = sizeof(Datalogger_t),
    .init         = dataloggerInit,
    .start        = NULL,
    .stop         = dataloggerStop,         /* flush on stop             */
    .deinit       = dataloggerDeinit,
    .produce      = NULL,                   /* consumer only — no sensor */
    .consumesApid = K_SPP_APID_ALL,        /* receives every packet     */
    .onPacket     = dataloggerOnPacket,
    .onPacketPrio = K_SPP_PUBSUB_PRIO_LOW, /* deferred — never blocks sensors */
};
