/**
 * @file pubsub.h
 * @brief Publish-subscribe packet router.
 *
 * How it works:
 *   1. A producer calls publish(packet).
 *   2. SYNC subscribers (prio = K_SPP_PUBSUB_PRIO_SYNC) run immediately inside
 *      publish() before it returns — use this only for very fast operations.
 *   3. All other subscribers are queued and dispatched one-per-call by
 *      SPP_SERVICES_PUBSUB_callConsumers() from the superloop.
 *
 * A subscriber receives a packet when (subscriber.apid & packet.apid) != 0,
 * or when subscriber.apid == K_SPP_APID_ALL (receives everything).
 */

#ifndef SPP_PUBSUB_H
#define SPP_PUBSUB_H

#include "spp/core/packet.h"
#include "spp/core/types.h"
#include "spp/core/returnTypes.h"
#include "spp/util/macros.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ----------------------------------------------------------------
 * Subscriber dispatch priorities
 * ---------------------------------------------------------------- */

/** @brief Run inside publish() — blocks the producer until the handler returns.
 *  Only use for very fast operations (e.g. copying a value to a buffer). */
#define K_SPP_PUBSUB_PRIO_SYNC   (0U)

/** @brief Deferred — dispatched by callConsumers(), before NORMAL. */
#define K_SPP_PUBSUB_PRIO_HIGH   (1U)

/** @brief Deferred — dispatched by callConsumers(). */
#define K_SPP_PUBSUB_PRIO_NORMAL (2U)

/** @brief Deferred — dispatched by callConsumers(), last. Good for slow operations
 *  like SD card writes that must not delay sensor reads. */
#define K_SPP_PUBSUB_PRIO_LOW    (3U)

/* ----------------------------------------------------------------
 * Constants
 * ---------------------------------------------------------------- */

/** @brief Wildcard APID — subscriber receives packets for every APID. */
#define K_SPP_APID_ALL  (0xFFFFU)

/* ----------------------------------------------------------------
 * Types
 * ---------------------------------------------------------------- */

/**
 * @brief Subscriber callback signature.
 *
 * For CRITICAL subscribers, called synchronously inside
 * @ref SPP_SERVICES_PUBSUB_publish().  For all other priorities, called from
 * @ref SPP_SERVICES_PUBSUB_callConsumers().  The packet pointer is valid only for the
 * duration of the call — do not store it.
 *
 * @param[in] p_packet  Published packet (read-only).
 * @param[in] p_ctx     Caller-supplied context pointer.
 */
typedef void (*SPP_PubSub_Handler_t)(const SPP_Packet_t *p_packet, void *p_ctx);

/* ----------------------------------------------------------------
 * API
 * ---------------------------------------------------------------- */

/**
 * @brief Initialise the pub/sub registry.
 *
 * Clears all subscriptions and resets the deferred queue.  Safe to call
 * multiple times.
 */
void SPP_SERVICES_PUBSUB_init(void);

/**
 * @brief Register a subscriber for a given APID bitmask.
 *
 * Subscribers are stored sorted by @p prio (ascending), so CRITICAL
 * subscribers are always dispatched first.
 *
 * @param[in] apid     Bitmask of APIDs to subscribe to.  A packet matches
 *                     when (apid & packet.apid) != 0.  Use @ref K_SPP_APID_ALL
 *                     to receive every packet, or @ref K_SPP_APID_NONE for none.
 * @param[in] prio     Dispatch priority (@ref K_SPP_PUBSUB_PRIO_SYNC …
 *                     @ref K_SPP_PUBSUB_PRIO_LOW).
 * @param[in] handler  Callback invoked on each matching publish.
 * @param[in] p_ctx    Context pointer forwarded unchanged to the callback.
 *
 * @return K_SPP_OK on success, or an error code otherwise.
 */
SPP_RetVal_t SPP_SERVICES_PUBSUB_subscribe(spp_uint16_t apid, spp_uint8_t prio,
                                            SPP_PubSub_Handler_t handler, void *p_ctx);

/**
 * @brief Publish a filled packet to all matching subscribers.
 *
 * CRITICAL subscribers are called synchronously before this function returns.
 * All other matching subscribers are enqueued; call @ref SPP_SERVICES_PUBSUB_callConsumers()
 * repeatedly to drain them.  The packet is returned to the databank only when
 * all deferred subscribers have been dispatched.
 *
 * On queue overflow the packet is discarded immediately and the per-APID
 * overflow counter is incremented.
 *
 * @param[in] p_packet  Filled packet from @ref SPP_SERVICES_DATABANK_getPacket().
 *
 * @return K_SPP_OK on success, K_SPP_ERROR_NULL_POINTER if @p p_packet is NULL.
 */
SPP_RetVal_t SPP_SERVICES_PUBSUB_publish(SPP_Packet_t *p_packet);

/**
 * @brief Dispatch the next pending deferred subscriber.
 *
 * Processes exactly one subscriber from the deferred queue per call.
 * Call this once per superloop iteration — it returns immediately when
 * the queue is empty.
 *
 * One-per-call is intentional: slow consumers (SD card writes) are spread
 * across loop iterations so they never block sensor reads.
 */
void SPP_SERVICES_PUBSUB_callConsumers(void);

/**
 * @brief Return the accumulated overflow count for a given APID bitmask.
 *
 * Counts how many packets matching @p apid were dropped because the deferred
 * queue was full at the time of publish.
 *
 * @param[in] apid  APID bitmask (same format as in subscribe).
 *
 * @return Cumulative number of dropped packets for the matching APIDs.
 */
spp_uint16_t SPP_SERVICES_PUBSUB_overflowCount(spp_uint16_t apid);

/**
 * @brief Return the number of currently registered subscribers.
 *
 * @return Subscriber count.
 */
spp_uint8_t SPP_SERVICES_PUBSUB_subscriberCount(void);

#ifdef __cplusplus
}
#endif

#endif /* SPP_PUBSUB_H */
