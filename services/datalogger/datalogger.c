/**
 * @file datalogger_service.c
 * @brief SD card packet logger implementation.
 */

#include "spp/services/datalogger/datalogger.h"

#include "spp/hal/storage.h"
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

SPP_RetVal_t DATALOGGER_init(Datalogger_t *p_logger, void *p_storage_cfg,
                         const char *p_file_path)
{
    SPP_RetVal_t ret;

    memset(p_logger, 0, sizeof(Datalogger_t));
    p_logger->p_storage_cfg = p_storage_cfg;

    ret = SPP_HAL_storageMount(p_storage_cfg);
    if (ret != K_SPP_OK)
    {
        SPP_LOGE(k_tag, "Storage mount failed");
        return ret;
    }

    p_logger->p_file = fopen(p_file_path, "w");
    if (p_logger->p_file == NULL)
    {
        SPP_LOGE(k_tag, "Failed to open file: %s", p_file_path);
        (void)SPP_HAL_storageUnmount(p_storage_cfg);
        return K_SPP_ERROR;
    }

    p_logger->is_initialized = 1U;
    p_logger->logged_packets = 0U;

    SPP_LOGI(k_tag, "Init OK file=%s", p_file_path);
    return K_SPP_OK;
}

SPP_RetVal_t DATALOGGER_logPacket(Datalogger_t *p_logger, const SPP_Packet_t *p_packet)
{
    if ((p_logger->is_initialized == 0U) || (p_logger->p_file == NULL))
    {
        return K_SPP_ERROR;
    }

    int n;
    n = fprintf(p_logger->p_file,
                "pkt=%lu ver=%u apid=0x%04X seq=%u len=%u ts=%lu drop=%u crc=%u payload_hex=",
                (unsigned long)p_logger->logged_packets,
                (unsigned)p_packet->primaryHeader.version,
                (unsigned)p_packet->primaryHeader.apid,
                (unsigned)p_packet->primaryHeader.seq,
                (unsigned)p_packet->primaryHeader.payloadLen,
                (unsigned long)p_packet->secondaryHeader.timestampMs,
                (unsigned)p_packet->secondaryHeader.dropCounter,
                (unsigned)p_packet->crc);
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

    (void)fprintf(p_logger->p_file, "\n");
    p_logger->logged_packets++;

    return K_SPP_OK;
}

SPP_RetVal_t DATALOGGER_flush(Datalogger_t *p_logger)
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

SPP_RetVal_t DATALOGGER_deinit(Datalogger_t *p_logger)
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

    if (p_logger->p_storage_cfg != NULL)
    {
        ret = SPP_HAL_storageUnmount(p_logger->p_storage_cfg);
        if (ret != K_SPP_OK)
        {
            SPP_LOGE(k_tag, "Storage unmount failed");
            p_logger->is_initialized = 0U;
            p_logger->p_storage_cfg = NULL;
            return ret;
        }
    }

    p_logger->is_initialized = 0U;
    p_logger->p_storage_cfg = NULL;

    SPP_LOGI(k_tag, "Deinit OK");
    return K_SPP_OK;
}
