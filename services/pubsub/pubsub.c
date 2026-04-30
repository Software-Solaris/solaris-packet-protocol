/**
 * @file pubsub.c
 * @brief Priority-aware publish-subscribe packet router implementation.
 */

#include "spp/services/pubsub/pubsub.h"
#include "spp/services/databank/databank.h"
#include "spp/util/macros.h"
#include "spp/services/log/log.h"
#include "spp/core/error.h"

/* ----------------------------------------------------------------
 * Private types
 * ---------------------------------------------------------------- */

typedef struct
{
    spp_uint16_t         apid;
    spp_uint8_t          prio;
    SPP_PubSub_Handler_t handler;
    void                *p_ctx;
} SubEntry_t;

typedef struct
{
    SPP_Packet_t *p_pkt;
    spp_uint8_t   nextSubIdx;
} QueueEntry_t;

/* ----------------------------------------------------------------
 * Private state
 * ---------------------------------------------------------------- */

static const char *const k_tag = "SPP_PUBSUB";

static SubEntry_t  s_subs[K_SPP_PUBSUB_MAX_SUBSCRIBERS];
static spp_uint8_t s_count       = 0U;
static spp_bool_t  s_initialized = false;

static QueueEntry_t s_queue[K_SPP_PUBSUB_QUEUE_SIZE];
static spp_uint8_t  s_head   = 0U;
static spp_uint8_t  s_tail   = 0U;
static spp_uint8_t  s_qCount = 0U;

/* One counter per bit position of the 16-bit APID field. */
static spp_uint16_t s_overflowCount[16U];

/* ----------------------------------------------------------------
 * Private helpers
 * ---------------------------------------------------------------- */

#define K_QUEUE_MASK ((spp_uint8_t)(K_SPP_PUBSUB_QUEUE_SIZE - 1U))

static spp_bool_t apidMatches(spp_uint16_t subApid, spp_uint16_t pktApid)
{
    if (subApid == K_SPP_APID_NONE) return false;
    if (subApid == K_SPP_APID_ALL)  return true;
    return (spp_bool_t)((subApid & pktApid) != 0U);
}

static void overflowIncrement(spp_uint16_t apid)
{
    spp_uint8_t bit;
    for (bit = 0U; bit < 16U; bit++)
    {
        if ((apid & (spp_uint16_t)(1U << bit)) != 0U)
        {
            if (s_overflowCount[bit] < 0xFFFFU)
            {
                s_overflowCount[bit]++;
            }
        }
    }
}

/* ----------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------- */

void SPP_SERVICES_PUBSUB_init(void)
{
    spp_uint8_t i;

    for (i = 0U; i < K_SPP_PUBSUB_MAX_SUBSCRIBERS; i++)
    {
        s_subs[i].apid    = 0U;
        s_subs[i].prio    = 0U;
        s_subs[i].handler = NULL;
        s_subs[i].p_ctx   = NULL;
    }
    for (i = 0U; i < K_SPP_PUBSUB_QUEUE_SIZE; i++)
    {
        s_queue[i].p_pkt      = NULL;
        s_queue[i].nextSubIdx = 0U;
    }
    for (i = 0U; i < 16U; i++)
    {
        s_overflowCount[i] = 0U;
    }

    s_count       = 0U;
    s_head        = 0U;
    s_tail        = 0U;
    s_qCount      = 0U;
    s_initialized = true;
}

SPP_RetVal_t SPP_SERVICES_PUBSUB_subscribe(spp_uint16_t apid, spp_uint8_t prio,
                                            SPP_PubSub_Handler_t handler, void *p_ctx)
{
    spp_uint8_t ins;
    spp_uint8_t i;

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

    /* Find insertion point — keep array sorted by prio ascending. */
    ins = 0U;
    while ((ins < s_count) && (s_subs[ins].prio <= prio))
    {
        ins++;
    }

    /* Shift existing entries right to make room. */
    for (i = s_count; i > ins; i--)
    {
        s_subs[i] = s_subs[i - 1U];
    }

    s_subs[ins].apid    = apid;
    s_subs[ins].prio    = prio;
    s_subs[ins].handler = handler;
    s_subs[ins].p_ctx   = p_ctx;
    s_count++;
    return K_SPP_OK;
}

SPP_RetVal_t SPP_SERVICES_PUBSUB_publish(SPP_Packet_t *p_packet)
{
    spp_uint8_t  i;
    spp_bool_t   hasDeferred = false;

    if (p_packet == NULL)
    {
        SPP_ERR_RETURN(K_SPP_ERROR_NULL_POINTER);
    }

    /* 1. Dispatch CRITICAL subscribers synchronously.  Since the array is
     *    sorted by prio, all CRITICAL entries appear first; break on the
     *    first non-CRITICAL entry. */
    for (i = 0U; i < s_count; i++)
    {
        if (s_subs[i].prio != K_SPP_PUBSUB_PRIO_CRITICAL) break;
        if (apidMatches(s_subs[i].apid, p_packet->primaryHeader.apid))
        {
            s_subs[i].handler(p_packet, s_subs[i].p_ctx);
        }
    }

    /* 2. Check whether any deferred (non-CRITICAL) subscriber matches. */
    for (i = 0U; i < s_count; i++)
    {
        if (s_subs[i].prio == K_SPP_PUBSUB_PRIO_CRITICAL) continue;
        if (apidMatches(s_subs[i].apid, p_packet->primaryHeader.apid))
        {
            hasDeferred = true;
            break;
        }
    }

    if (!hasDeferred)
    {
        (void)SPP_SERVICES_DATABANK_returnPacket(p_packet);
        return K_SPP_OK;
    }

    /* 3. Enqueue packet for deferred dispatch. */
    if (s_qCount >= K_SPP_PUBSUB_QUEUE_SIZE)
    {
        /* Queue full — drop newest, record overflow. */
        SPP_LOGW(k_tag, "Queue full — dropping apid=0x%04X", (unsigned)p_packet->primaryHeader.apid);
        overflowIncrement(p_packet->primaryHeader.apid);
        (void)SPP_SERVICES_DATABANK_returnPacket(p_packet);
        return K_SPP_OK;
    }

    s_queue[s_tail].p_pkt      = p_packet;
    s_queue[s_tail].nextSubIdx = 0U;
    s_tail                     = (s_tail + 1U) & K_QUEUE_MASK;
    s_qCount++;
    return K_SPP_OK;
}

void SPP_SERVICES_PUBSUB_tick(void)
{
    QueueEntry_t *p_entry;
    spp_uint8_t   i;
    spp_uint16_t  pktApid;

    if (s_qCount == 0U) return;

    p_entry = &s_queue[s_head];
    pktApid = p_entry->p_pkt->primaryHeader.apid;

    /* Find the next non-CRITICAL subscriber (starting from nextSubIdx) that
     * matches this packet's APID, call it, and save progress. */
    for (i = p_entry->nextSubIdx; i < s_count; i++)
    {
        if (s_subs[i].prio == K_SPP_PUBSUB_PRIO_CRITICAL) continue;
        if (apidMatches(s_subs[i].apid, pktApid))
        {
            s_subs[i].handler(p_entry->p_pkt, s_subs[i].p_ctx);
            p_entry->nextSubIdx = i + 1U;
            return;
        }
    }

    /* No more matching deferred subscribers — return packet to databank. */
    (void)SPP_SERVICES_DATABANK_returnPacket(p_entry->p_pkt);
    p_entry->p_pkt = NULL;
    s_head         = (s_head + 1U) & K_QUEUE_MASK;
    s_qCount--;
}

spp_uint16_t SPP_SERVICES_PUBSUB_overflowCount(spp_uint16_t apid)
{
    spp_uint16_t count = 0U;
    spp_uint8_t  bit;

    for (bit = 0U; bit < 16U; bit++)
    {
        if ((apid & (spp_uint16_t)(1U << bit)) != 0U)
        {
            count += s_overflowCount[bit];
        }
    }
    return count;
}

spp_uint8_t SPP_SERVICES_PUBSUB_subscriberCount(void)
{
    return s_count;
}
