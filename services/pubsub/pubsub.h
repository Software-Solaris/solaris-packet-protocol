/**
 * @file pubsub.h
 * @brief Publish-subscribe packet router.
 */

#ifndef SPP_PUBSUB_H
#define SPP_PUBSUB_H

#include "spp/core/packet.h"
#include "spp/core/types.h"
#include "spp/core/returnTypes.h"
#include "spp/services/service.h"
#include "spp/util/macros.h"


/* ----------------------------------------------------------------
 * CONSTANTS    
 * ---------------------------------------------------------------- */
#define K_SPP_SERVICES_PUBSUB_MAX_PRODUCERS    (5U)
#define K_SPP_SERVICES_PUBSUB_MAX_CONSUMERS    (5U)
#define K_SPP_SERVICES_PUBSUB_BUFFER_SIZE      (8U) /**< Internal packet buffer depth. */
#define K_SPP_SERVICES_PUBSUB_PREEMPT_PRIORITY (1U)

/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */

SPP_RetVal_t SPP_SERVICES_PUBSUB_registerProducer(
    const SPP_SERVICE_ProducerContract_t *p_producerData);

SPP_RetVal_t SPP_SERVICES_PUBSUB_registerConsumer(
    const SPP_SERVICE_ConsumerContract_t *p_consumerData);

SPP_RetVal_t SPP_SERVICES_PUBSUB_init(void);

/**
 * @brief  Copies a packet into the internal buffer and returns it to the databank.
 *
 * Called by producers after filling a packet.  The packet is copied by value
 * into the next free slot of the internal circular buffer; the original is
 * returned to the databank pool immediately.  Returns K_SPP_ERROR if the
 * buffer is full (packet is still returned to the pool).
 *
 * @param  p_pkt  Packet to publish (ownership transferred — do not use after this call).
 * @return K_SPP_OK on success, K_SPP_ERROR_NULL_POINTER if p_pkt is NULL,
 *         K_SPP_ERROR if the buffer is full.
 */
SPP_RetVal_t SPP_SERVICES_PUBSUB_publish(SPP_Packet_t *p_pkt);

/**
 * @brief  Calls acquireData on every registered producer.
 */
void SPP_SERVICES_PUBSUB_callProducers(void);

/**
 * @brief  Routes all buffered packets to matching consumer mailboxes, then clears the buffer.
 */

void SPP_SERVICES_PUBSUB_callConsumers(void);

/**
 * @brief  Returns the number of packets currently held in the internal FIFO buffer.
 */
spp_uint8_t SPP_SERVICES_PUBSUB_queueDepth(void);

/**
 * @brief  Returns the cumulative number of packets dropped for a specific APID due to a full
 */
spp_uint32_t SPP_SERVICES_PUBSUB_overflowCount(spp_uint16_t apid);

/**
 * @brief  Signals that a producer has new data pending.
 *
 * Must be called from within acquireData() as soon as a DRDY flag is detected,
 * so that callConsumers() can preempt consumers with priority > K_SPP_SERVICES_PUBSUB_PREEMPT_PRIORITY.
 */
void SPP_SERVICES_PUBSUB_signalProducerReady(void);

#endif /* SPP_PUBSUB_H */
