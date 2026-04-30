/**
 * @file pubsub.h
 * @brief Priority-aware publish-subscribe packet router with deferred dispatch.
 *
 * Producers call @ref SPP_SERVICES_PUBSUB_publish() after filling a packet.
 * CRITICAL subscribers (@ref K_SPP_PUBSUB_PRIO_CRITICAL) are dispatched
 * synchronously inside publish().  All other subscribers are enqueued and
 * dispatched one-at-a-time by calls to @ref SPP_SERVICES_PUBSUB_tick() from
 * the superloop.
 *
 * APID matching uses a bitmask: a subscriber receives a packet when
 * (subscriber.apid & packet.apid) != 0, or when subscriber.apid ==
 * @ref K_SPP_APID_ALL.  Use @ref K_SPP_APID_NONE to indicate no subscription.
 *
 * Naming conventions used in this file:
 * - Constants/macros:  K_SPP_*
 * - Types:             SPP_PubSub*_t
 * - Public functions:  SPP_SERVICES_PUBSUB_*()
 * - Pointer params:    p_*
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
 * @ref SPP_SERVICES_PUBSUB_tick().  The packet pointer is valid only for the
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
 * @param[in] prio     Dispatch priority (@ref K_SPP_PUBSUB_PRIO_CRITICAL …
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
 * All other matching subscribers are enqueued; call @ref SPP_SERVICES_PUBSUB_tick()
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
 * Processes exactly one subscriber from the deferred queue per call.  Call
 * this once per superloop iteration to drain the queue without monopolising
 * the loop.  Returns immediately when the queue is empty.
 */
void SPP_SERVICES_PUBSUB_tick(void);

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
