/**
 * @file pubsub.c
 * @brief Synchronous publish-subscribe packet router implementation.
 */

#include "spp/services/pubsub/pubsub.h"
#include "spp/services/databank/databank.h"
#include "spp/util/macros.h"
#include "spp/services/log/log.h"
#include "spp/core/error.h"

/* ----------------------------------------------------------------
 * Private state
 * ---------------------------------------------------------------- */

static const char *const k_tag = "SPP_PUBSUB";

typedef struct
{
    spp_uint16_t         apid;
    SPP_PubSub_Handler_t handler;
    void                *p_ctx;
} SubEntry_t;

static SubEntry_t  s_subs[K_SPP_PUBSUB_MAX_SUBSCRIBERS];
static spp_uint8_t s_count       = 0U;
static spp_bool_t  s_initialized = false;

/* ----------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------- */

void SPP_PubSub_init(void)
{
    spp_uint8_t i;
    for (i = 0U; i < K_SPP_PUBSUB_MAX_SUBSCRIBERS; i++)
    {
        s_subs[i].apid    = 0U;
        s_subs[i].handler = NULL;
        s_subs[i].p_ctx   = NULL;
    }
    s_count       = 0U;
    s_initialized = true;
}

SPP_RetVal_t SPP_PubSub_subscribe(spp_uint16_t apid, SPP_PubSub_Handler_t handler,
                                   void *p_ctx)
{
    if (!s_initialized)
    {
        SPP_ERR_RETURN(K_SPP_ERROR_NOT_INITIALIZED);
    }
    if (handler == NULL)
    {
        SPP_ERR_RETURN(K_SPP_ERROR_NULL_POINTER);
    }
    if (s_count >= K_SPP_PUBSUB_MAX_SUBSCRIBERS)
    {
        SPP_LOGE(k_tag, "Subscriber table full (%u)", (unsigned)K_SPP_PUBSUB_MAX_SUBSCRIBERS);
        SPP_ERR_RETURN(K_SPP_ERROR);
    }

    s_subs[s_count].apid    = apid;
    s_subs[s_count].handler = handler;
    s_subs[s_count].p_ctx   = p_ctx;
    s_count++;
    return K_SPP_OK;
}

SPP_RetVal_t SPP_PubSub_publish(SPP_Packet_t *p_packet)
{
    spp_uint8_t i;

    if (p_packet == NULL)
    {
        SPP_ERR_RETURN(K_SPP_ERROR_NULL_POINTER);
    }

    for (i = 0U; i < s_count; i++)
    {
        if ((s_subs[i].apid == K_SPP_APID_ALL) ||
            (s_subs[i].apid == p_packet->primaryHeader.apid))
        {
            s_subs[i].handler(p_packet, s_subs[i].p_ctx);
        }
    }

    (void)SPP_Databank_returnPacket(p_packet);
    return K_SPP_OK;
}

spp_uint8_t SPP_PubSub_subscriberCount(void)
{
    return s_count;
}
