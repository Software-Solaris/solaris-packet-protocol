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
#define K_SPP_SERVICES_PUBSUB_BUFFER_SIZE      (128U) /**< Internal packet buffer depth. */
#define K_SPP_SERVICES_PUBSUB_PREEMPT_PRIORITY (1U)

#define K_SPP_SERVICES_PUBSUB_STEP             (1U) /**< Step to increment circular buffer index. */
#define K_SPP_SERVICES_PUBSUB_SORT_START_INDEX (1U) /**< Starting index for insertion sort. */
#define K_SPP_SERVICES_PUBSUB_OFFSET           (1U) /**< Array index offset. */
#define K_SPP_SERVICES_PUBSUB_ZERO_INDEX       (0)  /**< Zero index/value for comparisons. */

/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */

/**
 * @brief Registers a producer and assigns it a unique KPID.
 * @param p_producerData Pointer to the producer contract.
 * @param p_assignedKpid Pointer where the assigned KPID is stored.
 * @return K_SPP_OK, K_SPP_ERROR_NULL_POINTER or K_SPP_ERROR_REGISTRY_FULL.
 */
SPP_RetVal_t SPP_SERVICES_PUBSUB_registerProducer(const SPP_SERVICE_ProducerContract_t *p_producerData,

                                                  /**
 * @brief Registers a consumer with its KPID subscription bitmask.
 * @param p_consumerData Pointer to the consumer contract.
 * @param subscription KPID bitmask identifying subscribed producers.
 * @return K_SPP_OK, K_SPP_ERROR_NULL_POINTER or K_SPP_ERROR_REGISTRY_FULL.
 */
                                                  SPP_Kpid_t *p_assignedKpid);
SPP_RetVal_t SPP_SERVICES_PUBSUB_registerConsumer(const SPP_SERVICE_ConsumerContract_t *p_consumerData,
                                                  SPP_Kpid_t subscription);

/**
 * @brief Sorts consumers by priority and initializes registered modules.
 *
 * Consumers are initialized before producers. Individual initialization
 * results are stored for later inspection.
 *
 * @return K_SPP_OK. Individual module failures do not change this return value.
 */
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
  * @brief Calls acquireData on every successfully initialized producer.
 */
void SPP_SERVICES_PUBSUB_callProducers(void);

/**
 * @brief Dispatches one buffered packet and executes initialized consumers.
 *
 * Consumers execute in ascending priority order. Lower-priority consumers
 * may be skipped when a producer signals that new data is ready.
 */
void SPP_SERVICES_PUBSUB_callConsumers(void);

/**
 * @brief  Returns the number of packets currently held in the internal FIFO buffer.
 */
spp_uint8_t SPP_SERVICES_PUBSUB_queueDepth(void);

/**
 * @brief  Signals that a producer has new data pending.
 *
 * Must be called from within acquireData() as soon as a DRDY flag is detected,
 * so that callConsumers() can preempt consumers with priority > K_SPP_SERVICES_PUBSUB_PREEMPT_PRIORITY.
 */
void SPP_SERVICES_PUBSUB_signalProducerReady(void);

/**
 * @brief Retrieves the initialization result of a registered producer.
 * @param p_producerData Pointer to the producer contract.
 * @param p_initResult Output pointer for the initialization result.
 * @return K_SPP_OK if found, K_SPP_ERROR if not registered,
 *         or K_SPP_ERROR_NULL_POINTER for invalid pointers.
 */
SPP_RetVal_t SPP_SERVICES_PUBSUB_getProducerInitResult(const SPP_SERVICE_ProducerContract_t *p_producerData,
                                                       SPP_RetVal_t *p_initResult);

/**
 * @brief Retrieves the initialization result of a registered consumer.
 * @param p_consumerData Pointer to the consumer contract.
 * @param p_initResult Output pointer for the initialization result.
 * @return K_SPP_OK if found, K_SPP_ERROR if not registered,
 *         or K_SPP_ERROR_NULL_POINTER for invalid pointers.
 */
SPP_RetVal_t SPP_SERVICES_PUBSUB_getConsumerInitResult(const SPP_SERVICE_ConsumerContract_t *p_consumerData,
                                                       SPP_RetVal_t *p_initResult);
#endif /* SPP_PUBSUB_H */
