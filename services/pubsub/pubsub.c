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

static SPP_Packet_t s_packetBuffer[K_SPP_SERVICES_PUBSUB_BUFFER_SIZE];
static spp_uint8_t s_bufferHead = 0U;
static spp_uint8_t s_bufferTail = 0U;
static spp_uint8_t s_bufferCount = 0U;

static volatile spp_bool_t s_producerReady = false;

static int K_SPP_APID_ALL = 1;
/* ----------------------------------------------------------------
 * STATIC FUNCTIONS DECLARATIONS
 * ---------------------------------------------------------------- */
static void SPP_SERVICES_PUBSUB_sendToMailbox(void);
static void SPP_SERVICES_PUBSUB_sortConsumers(void);

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

void SPP_SERVICES_PUBSUB_callProducers(void)
{
    s_producerReady = false;
    for (spp_uint8_t i = 0U; i < s_registeredProducers; i++)
    {
        (void)s_producers[i]->acquireData();
    }
}

SPP_RetVal_t SPP_SERVICES_PUBSUB_publish(SPP_Packet_t *p_pkt)
{
    SPP_RetVal_t ret = K_SPP_OK;

    if (p_pkt == NULL)
    {
        return K_SPP_ERROR_NULL_POINTER;
    }

    if (s_bufferCount >= K_SPP_SERVICES_PUBSUB_BUFFER_SIZE)
    {
        ret = K_SPP_ERROR;
    }
    else
    {
        s_packetBuffer[s_bufferTail] = *p_pkt;
        s_bufferTail = (spp_uint8_t)((s_bufferTail + 1U) % K_SPP_SERVICES_PUBSUB_BUFFER_SIZE);
        s_bufferCount++;
    }

    (void)SPP_SERVICES_DATABANK_returnPacket(p_pkt);
    return ret;
}

void SPP_SERVICES_PUBSUB_callConsumers(void)
{
    // Sends the received data via de publish to the consumer mailbox
    SPP_SERVICES_PUBSUB_sendToMailbox();
    for (spp_uint8_t j = 0U; j < s_registeredConsumers; j++)
    {
        if (s_consumers[j]->consumeData == NULL)
        {
            continue;
        }
        if (!s_consumers[j]->isMailboxFull)
        {
            continue;
        }
        /* Preempt lower-priority consumers if a producer has new data pending. */
        if (s_producerReady && (s_consumers[j]->priority > K_SPP_SERVICES_PUBSUB_PREEMPT_PRIORITY))
        {
            break;
        }
        (void)s_consumers[j]->consumeData(NULL);
    }
}

spp_uint8_t SPP_SERVICES_PUBSUB_queueDepth(void)
{
    return s_bufferCount;
}
spp_uint32_t SPP_SERVICES_PUBSUB_overflowCount(spp_uint16_t apid)
{
    spp_uint32_t total = 0U;
    if (apid == K_SPP_APID_ALL)
    {
        for (spp_uint8_t i = 0U; i < s_registeredConsumers; i++)
        {
            total += s_consumers[i]->overflowCount;
        }
        return total;
    }
    for (spp_uint8_t i = 0U; i < s_registeredConsumers; i++)
    {
        if ((s_consumers[i]->suscribeToApid & apid) != 0U)
        {
            total += s_consumers[i]->overflowCount;
        }
    }
    return total;
}

void SPP_SERVICES_PUBSUB_signalProducerReady(void)
{
    s_producerReady = true;
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

/* ----------------------------------------------------------------
 * STATIC FUNCTIONS
 * ---------------------------------------------------------------- */
/**
 * @brief  Copies each buffered packet to every consumer whose APID subscription matches,
 *         then resets the internal FIFO (head, tail and count back to zero).
 *
 * Iterates the circular packet buffer from head to head+count.  For each packet
 * it walks the registered consumer list and performs a bitmask check on
 * @c suscribeToApid vs @c primaryHeader.apid.  On a match the full
 * @ref SPP_Packet_t is copied by value into the consumer's @c p_mailBox.
 * When all consumers have been served the buffer is cleared.
 */
static void SPP_SERVICES_PUBSUB_sendToMailbox(void)
{
    if (s_bufferCount == 0U)
    {
        return;
    }
    const SPP_Packet_t *p_pkt = &s_packetBuffer[s_bufferHead];
    for (spp_uint8_t j = 0U; j < s_registeredConsumers; j++)
    {
        if (s_consumers[j]->deliverToMailbox == NULL)
        {
            continue;
        }
        if ((s_consumers[j]->suscribeToApid & p_pkt->primaryHeader.apid) == 0U)
        {
            continue;
        }
        (void)s_consumers[j]->deliverToMailbox(p_pkt);
    }
    s_bufferHead = (spp_uint8_t)((s_bufferHead + 1U) % K_SPP_SERVICES_PUBSUB_BUFFER_SIZE);
    s_bufferCount--;
}

/**
 * @brief  Sorts the registered consumer array in ascending priority order using
 *         insertion sort.  Stable — consumers with equal priority retain their
 *         registration order.  Called once during init before any dispatch.
 */
static void SPP_SERVICES_PUBSUB_sortConsumers(void)
{
    for (spp_uint8_t i = 1U; i < s_registeredConsumers; i++)
    {
        const SPP_SERVICE_ConsumerContract_t *key = s_consumers[i];
        spp_int8_t j = (spp_int8_t)i - 1;
        while ((j >= 0) && (s_consumers[(spp_uint8_t)j]->priority > key->priority))
        {
            s_consumers[(spp_uint8_t)(j + 1U)] = s_consumers[(spp_uint8_t)j];
            j--;
        }
        s_consumers[(spp_uint8_t)(j + 1)] = key;
    }
}