/**
 * @file datalogger_service.c
 * @brief SD card packet logger implementation.
 */

#include "spp/services/datalogger/datalogger.h"

#include "spp/hal/storage.h"
#include "spp/core/packet.h"
#include "spp/services/log/log.h"
#include "spp/core/types.h"

#include <string.h>

/* ----------------------------------------------------------------
 * Private state
 * ---------------------------------------------------------------- */

static const char *const k_tag = "DATALOGGER";

/* ----------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------- */

SPP_RetVal_t SPP_SERVICES_DATALOGGER_init(Datalogger_t *p_logger)
{
    SPP_RetVal_t ret = SPP_HAL_storageMount(p_logger->p_storageCfg);
    if (ret != K_SPP_OK)
    {
        SPP_LOGE(k_tag, "Storage mount failed");
        return ret;
    }

    p_logger->p_file = fopen(p_logger->p_filePath, "w");
    if (p_logger->p_file == NULL)
    {
        SPP_LOGE(k_tag, "Failed to open file: %s", p_logger->p_filePath);
        (void)SPP_HAL_storageUnmount(p_logger->p_storageCfg);
        return K_SPP_ERROR;
    }

    p_logger->is_initialized = 1U;
    p_logger->logged_packets = 0U;

    SPP_LOGI(k_tag, "Init OK file=%s", p_logger->p_filePath);
    return K_SPP_OK;
}

SPP_RetVal_t SPP_SERVICES_DATALOGGER_logPacket(Datalogger_t *p_logger, const SPP_Packet_t *p_packet)
{
    if ((p_logger->is_initialized == 0U) || (p_logger->p_file == NULL))
    {
        return K_SPP_ERROR;
    }

    int n;

    if (p_packet->primaryHeader.apid == K_SPP_APID_LOG)
    {
        /* Log packets carry a null-terminated string — write it directly. */
        n = fprintf(p_logger->p_file, "%.*s\n",
                    (int)p_packet->primaryHeader.payloadLen,
                    (const char *)p_packet->payload);
    }
    else
    {
        /* Sensor/telemetry packet: one line with hex payload. */
        n = fprintf(p_logger->p_file,
                    "ts=%lu apid=0x%04X seq=%u len=%u payload_hex=",
                    (unsigned long)p_packet->secondaryHeader.timestampMs,
                    (unsigned)p_packet->primaryHeader.apid,
                    (unsigned)p_packet->primaryHeader.seq,
                    (unsigned)p_packet->primaryHeader.payloadLen);
        if (n < 0)
        {
            SPP_LOGE(k_tag, "fprintf header failed");
            return K_SPP_ERROR;
        }

        for (spp_uint16_t i = 0U; i < p_packet->primaryHeader.payloadLen; i++)
        {
            n = fprintf(p_logger->p_file, "%02X", (unsigned)p_packet->payload[i]);
            if (n < 0)
            {
                SPP_LOGE(k_tag, "fprintf payload failed");
                return K_SPP_ERROR;
            }
            if (i + 1U < p_packet->primaryHeader.payloadLen)
            {
                (void)fprintf(p_logger->p_file, " ");
            }
        }

        n = fprintf(p_logger->p_file, "\n");
    }

    if (n < 0)
    {
        SPP_LOGE(k_tag, "fprintf failed");
        return K_SPP_ERROR;
    }

    p_logger->logged_packets++;
    return K_SPP_OK;
}

SPP_RetVal_t SPP_SERVICES_DATALOGGER_flush(Datalogger_t *p_logger)
{
    if ((p_logger->is_initialized == 0U) || (p_logger->p_file == NULL))
    {
        return K_SPP_ERROR;
    }

    if (fflush(p_logger->p_file) != 0)
    {
        SPP_LOGE(k_tag, "fflush failed");
        return K_SPP_ERROR;
    }

    SPP_LOGI(k_tag, "fflush OK");
    return K_SPP_OK;
}

SPP_RetVal_t SPP_SERVICES_DATALOGGER_deinit(Datalogger_t *p_logger)
{
    SPP_RetVal_t ret;

    if (p_logger == NULL)
    {
        return K_SPP_ERROR_NULL_POINTER;
    }

    if (p_logger->p_file != NULL)
    {
        if (fflush(p_logger->p_file) != 0)
        {
            SPP_LOGE(k_tag, "fflush failed during deinit");
        }

        fclose(p_logger->p_file);
        p_logger->p_file = NULL;
    }

    if (p_logger->p_storageCfg != NULL)
    {
        ret = SPP_HAL_storageUnmount(p_logger->p_storageCfg);
        if (ret != K_SPP_OK)
        {
            SPP_LOGE(k_tag, "Storage unmount failed");
            p_logger->is_initialized = 0U;
            p_logger->p_storageCfg = NULL;
            return ret;
        }
    }

    p_logger->is_initialized = 0U;
    p_logger->p_storageCfg = NULL;

    SPP_LOGI(k_tag, "Deinit OK");
    return K_SPP_OK;
}

/* ----------------------------------------------------------------
 * Module callbacks and descriptor
 * ---------------------------------------------------------------- */

#define K_DATALOGGER_FLUSH_EVERY (20U)

static void sdLoggerOnPacket(const SPP_Packet_t *p_packet, void *p_ctx)
{
    Datalogger_t *p_logger = (Datalogger_t *)p_ctx;
    (void)SPP_SERVICES_DATALOGGER_logPacket(p_logger, p_packet);

    if ((p_logger->logged_packets % K_DATALOGGER_FLUSH_EVERY) == 0U)
    {
        (void)SPP_SERVICES_DATALOGGER_flush(p_logger);
    }
}

static SPP_RetVal_t sdLoggerInit(void *p_ctx)
{
    return SPP_SERVICES_DATALOGGER_init((Datalogger_t *)p_ctx);
}

static SPP_RetVal_t sdLoggerStart(void *p_ctx)
{
    (void)p_ctx;
    SPP_LOGI(k_tag, "Service ready — logging all packets");
    return K_SPP_OK;
}

static SPP_RetVal_t sdLoggerStop(void *p_ctx)
{
    return SPP_SERVICES_DATALOGGER_flush((Datalogger_t *)p_ctx);
}

static SPP_RetVal_t sdLoggerDeinit(void *p_ctx)
{
    return SPP_SERVICES_DATALOGGER_deinit((Datalogger_t *)p_ctx);
}

const SPP_Module_t g_sdLoggerModule = {
    .p_name       = "sd_logger",
    .apid         = K_SPP_APID_NONE,
    .ctxSize      = sizeof(Datalogger_t),
    .init         = sdLoggerInit,
    .start        = sdLoggerStart,
    .stop         = sdLoggerStop,
    .deinit       = sdLoggerDeinit,
    .produce      = NULL,
    .consumesApid = K_SPP_APID_ALL,
    .onPacket     = sdLoggerOnPacket,
    .onPacketPrio = K_SPP_PUBSUB_PRIO_LOW,
};
