/**
 * @file pubsub.h
 * @brief Synchronous publish-subscribe packet router.
 *
 * Producers call @ref SPP_SERVICES_PUBSUB_publish() after filling a packet.  The
 * publish function dispatches the packet to every registered subscriber whose
 * APID matches, then returns the packet to the databank.
 *
 * Subscribers are registered from outside the producing module (e.g. in
 * main()) so that sensor services remain agnostic to who consumes their data.
 *
 * Naming conventions used in this file:
 * - Constants/macros:  K_SPP_*
 * - Types:             SPP_PubSub*_t
 * - Public functions:  SPP_PubSub_*()
 * - Pointer params:    p_*
 */

#ifndef SPP_PUBSUB_H
#define SPP_PUBSUB_H

#include "spp/core/packet.h"
#include "spp/core/types.h"
#include "spp/core/returnTypes.h"

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
 * Called synchronously inside @ref SPP_SERVICES_PUBSUB_publish().  The packet pointer
 * is valid only for the duration of the call — do not store it.
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
 * Clears all subscriptions.  Safe to call multiple times.
 */
void SPP_SERVICES_PUBSUB_init(void);

/**
 * @brief Register a subscriber for a given APID.
 *
 * @param[in] apid     APID to subscribe to, or @ref K_SPP_APID_ALL for all.
 * @param[in] handler  Callback invoked on each matching publish.
 * @param[in] p_ctx    Context pointer forwarded unchanged to the callback.
 *
 * @return K_SPP_OK on success, or an error code otherwise.
 */
SPP_RetVal_t SPP_SERVICES_PUBSUB_subscribe(spp_uint16_t apid, SPP_PubSub_Handler_t handler,
                                   void *p_ctx);

/**
 * @brief Publish a filled packet to all matching subscribers.
 *
 * Dispatches @p p_packet synchronously to every subscriber whose APID equals
 * @c p_packet->primaryHeader.apid or is @ref K_SPP_APID_ALL, then returns
 * the packet to the databank via @ref SPP_SERVICES_DATABANK_returnPacket().
 *
 * The packet pointer becomes invalid after this call returns — subscribers
 * must not retain it.
 *
 * @param[in] p_packet  Filled packet obtained from @ref SPP_SERVICES_DATABANK_getPacket().
 *
 * @return K_SPP_OK on success, K_SPP_ERROR_NULL_POINTER if @p p_packet is NULL.
 */
SPP_RetVal_t SPP_SERVICES_PUBSUB_publish(SPP_Packet_t *p_packet);

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
