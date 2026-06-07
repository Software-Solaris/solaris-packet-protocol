/**
 * @file pubsub.c
 * @brief Priority-aware publish-subscribe packet router implementation.
 */

#include "spp/services/pubsub/pubsub.h"
#include "spp/services/databank/databank.h"
#include "spp/util/macros.h"
#include "spp/services/log/log.h"
#include "spp/core/error.h"


//TODO: Tener una forma de guardar los data packets que te llegan
//      Una forma de distribuir ese data packet por consumidores prioritarios
//      Devolver el datapacketal bank
/* ----------------------------------------------------------------
 * CONSTANTS
 * ---------------------------------------------------------------- */
static const SPP_SERVICE_ProducerContract_t *s_producers[K_SPP_SERVICES_PUBSUB_MAX_PRODUCERS] = {NULL};
static spp_uint8_t s_registeredProducers = 0U;
static const SPP_SERVICE_ConsumerContract_t *s_consumers[K_SPP_SERVICES_PUBSUB_MAX_CONSUMERS] = {NULL};
static spp_uint8_t s_registeredConsumers = 0U;

/* ----------------------------------------------------------------
* PUBLIC FUNCTIONS
* ---------------------------------------------------------------- */

SPP_RetVal_t SPP_SERVICES_PUBSUB_registerProducer(const SPP_SERVICE_ProducerContract_t *p_producerData)
{
    SPP_RetVal_t ret = K_SPP_ERROR;
    if (p_producerData == NULL)
    {
        ret = K_SPP_ERROR_NULL_POINTER;
    }
    else if (s_registeredProducers >= K_SPP_SERVICES_PUBSUB_MAX_PRODUCERS)
    {
        ret = K_SPP_ERROR_REGISTRY_FULL;
    }
    else
    {
        s_producers[s_registeredProducers] = p_producerData;
        s_registeredProducers++;
        ret = K_SPP_OK;
    }
    return ret;
}

SPP_RetVal_t SPP_SERVICES_PUBSUB_registerConsumer(const SPP_SERVICE_ConsumerContract_t *p_consumerData)
{
    SPP_RetVal_t ret = K_SPP_ERROR;
    if (p_consumerData == NULL)
    {
        ret = K_SPP_ERROR_NULL_POINTER;
    }
    else if (s_registeredConsumers >= K_SPP_SERVICES_PUBSUB_MAX_CONSUMERS)
    {
        ret = K_SPP_ERROR_REGISTRY_FULL;
    }
    else
    {
        s_consumers[s_registeredConsumers] = p_consumerData;
        s_registeredConsumers++;
        ret = K_SPP_OK;
    }
    return ret;
}

SPP_RetVal_t SPP_SERVICES_PUBSUB_init(void)
{
    SPP_RetVal_t ret = K_SPP_OK;

    //TODO: Call a function to order the consumer array based on priority


    if (s_registeredProducers > 0U)
    {
        //Call the init function for each of the producers registered
        for (spp_uint8_t i = 0U; i < s_registeredProducers; i++)
        {
            ret = s_producers[i]->init();
        }
    }
    else if (s_registeredConsumers > 0U)
    {
        //Call the init function for each of the consumers registered
        for (spp_uint8_t i = 0U; i < s_registeredConsumers; i++)
        {
            ret = s_consumers[i]->init();
        }
    }
    else
    {
        //No producers or consumers registered
    }
    return ret;
}
