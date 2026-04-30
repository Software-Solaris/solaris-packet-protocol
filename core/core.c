/**
 * @file core.c
 * @brief SPP core initialisation and port registry implementation.
 */

#include "spp/core/core.h"
#include "spp/core/returnTypes.h"
#include "spp/core/error.h"
#include "spp/core/version.h"
#include "spp/core/packet.h"
#include "spp/core/types.h"
#include "spp/services/databank/databank.h"
#include "spp/services/pubsub/pubsub.h"
#include "spp/services/log/log.h"

#include <stdio.h>

/* ----------------------------------------------------------------
 * Private state
 * ---------------------------------------------------------------- */

static const SPP_HalPort_t *s_p_halPort = NULL;

/* ----------------------------------------------------------------
 * Port registration
 * ---------------------------------------------------------------- */

SPP_RetVal_t SPP_CORE_setHalPort(const SPP_HalPort_t *p_port)
{
    if (p_port == NULL)
    {
        SPP_ERR_RETURN(K_SPP_ERROR_NULL_POINTER);
    }
    s_p_halPort = p_port;
    return K_SPP_OK;
}

const SPP_HalPort_t *SPP_CORE_getHalPort(void)
{
    return s_p_halPort;
}

/* ----------------------------------------------------------------
 * Core initialisation
 * ---------------------------------------------------------------- */

SPP_RetVal_t SPP_CORE_init(void)
{
    if (s_p_halPort == NULL)
    {
        SPP_ERR_RETURN(K_SPP_ERROR_NOT_INITIALIZED);
    }

    SPP_RetVal_t ret = SPP_SERVICES_LOG_init();
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_DATABANK_init();
    if ((ret != K_SPP_OK) && (ret != K_SPP_ERROR_ALREADY_INITIALIZED))
    {
        return ret;
    }

    SPP_SERVICES_PUBSUB_init();

    SPP_LOGI("SPP_CORE", "SPP core initialised (v%u.%u.%u)", K_SPP_VERSION_MAJOR,
             K_SPP_VERSION_MINOR, K_SPP_VERSION_PATCH);

    return K_SPP_OK;
}

/* ----------------------------------------------------------------
 * Boot
 * ---------------------------------------------------------------- */

static spp_uint16_t s_logSeq = 0U;
static spp_bool_t   s_logBusy = false;

static void coreLogOutput(const char *p_tag, SPP_LogLevel_t level, const char *p_message)
{
    static const char k_lvl[] = "?EWID V";
    char lvlChar = k_lvl[(unsigned)level < sizeof(k_lvl) ? (unsigned)level : 0U];

    printf("[%c] %s: %s\n", lvlChar, p_tag, p_message);

    if (s_logBusy)
    {
        return;
    }
    s_logBusy = true;

    SPP_Packet_t *p_pkt = SPP_SERVICES_DATABANK_getPacket();
    if (p_pkt != NULL)
    {
        char buf[K_SPP_PKT_PAYLOAD_MAX];
        int n = snprintf(buf, sizeof(buf), "[%c] %s: %s", lvlChar, p_tag, p_message);
        spp_uint16_t len =
            (n > 0 && n < (int)sizeof(buf)) ? (spp_uint16_t)(n + 1U) : (spp_uint16_t)sizeof(buf);

        (void)SPP_SERVICES_DATABANK_packetData(p_pkt, K_SPP_APID_LOG, s_logSeq++, buf, len);
        (void)SPP_SERVICES_PUBSUB_publish(p_pkt);
    }

    s_logBusy = false;
}

SPP_RetVal_t SPP_CORE_boot(const SPP_HalPort_t *p_port)
{
    SPP_RetVal_t ret = SPP_CORE_setHalPort(p_port);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_CORE_init();
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    SPP_SERVICES_LOG_setOutput(coreLogOutput);
    return K_SPP_OK;
}
