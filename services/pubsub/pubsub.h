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
#define K_SPP_SERVICES_PUBSUB_MAX_PRODUCERS (5U)
#define K_SPP_SERVICES_PUBSUB_MAX_CONSUMERS (5U)


/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */

SPP_RetVal_t SPP_SERVICES_PUBSUB_registerProducer(const SPP_SERVICE_ProducerContract_t *p_producerData);

SPP_RetVal_t SPP_SERVICES_PUBSUB_registerConsumer(const SPP_SERVICE_ConsumerContract_t *p_consumerData);

SPP_RetVal_t SPP_SERVICES_PUBSUB_init(void);


#endif /* SPP_PUBSUB_H */
