/**
 * @file initEvents.c
 * @brief Init info driver + SPP service implementation.
 */

#include "spp/services/events/initEvents.h"

#include "spp/hal/spi/spi.h"
#include "spp/hal/time/time.h"
#include "spp/services/log/log.h"
#include "spp/services/databank/databank.h"
#include "spp/core/packet.h"
#include "spp/core/pubsub/pubsub.h"
#include "spp/services/datalogger/datalogger.h"

#include <string.h>

/* ----------------------------------------------------------------
 * CONSTANTS
 * ---------------------------------------------------------------- */
#define K_EVENTS_TASK_TIMEOUT_MS 10000U // Only executed once, np with stops
#define K_EVENTS_LOG_TAG         "INIT EVENTS"

/* ----------------------------------------------------------------
 * STATIC FUNCTIONS DECLARATIONS
 * ---------------------------------------------------------------- */
static SPP_RetVal_t SPP_SERVICES_INITEVENTS_init(void);
static SPP_RetVal_t SPP_SERVICES_INITEVENTS_acquireData(SPP_Kpid_t kpid);

/* ----------------------------------------------------------------
 * VARIABLES
 * ---------------------------------------------------------------- */
static const SPP_SERVICE_ProducerContract_t g_initEventsProducerContract = {
    .p_nameProducer = "initEvents",
    .tiemoutMs = K_EVENTS_TASK_TIMEOUT_MS,
    .init = SPP_SERVICES_INITEVENTS_init,
    .acquireData = SPP_SERVICES_INITEVENTS_acquireData,
};

static initEvents_t s_initEventsData;
static const char *const k_svcTag = "INITEVENTS_SVC";

/* ----------------------------------------------------------------
* PUBLIC FUNCTIONS
* ---------------------------------------------------------------- */
const SPP_SERVICE_ProducerContract_t *SPP_SERVICES_INITEVENTS_getProducerContract(void)
{
    return &g_initEventsProducerContract;
}

/* ----------------------------------------------------------------
* STATIC FUNCTIONS
* ---------------------------------------------------------------- */
// Just reset the counter, not necessary (just to maintain consistency)
static SPP_RetVal_t SPP_SERVICES_INITEVENTS_init(void)
{
    s_initEventsData.seq = 0U;

    return K_SPP_OK;
}

static SPP_RetVal_t SPP_SERVICES_INITEVENTS_acquireData(SPP_Kpid_t kpid)
{
    SPP_RetVal_t sdInitStatus = SPP_SERVICES_DATALOGGER_getInitStatus();

    SPP_Packet_t *p_packet = SPP_SERVICES_DATABANK_getPacket();
    if (p_packet == NULL)
    {
        return K_SPP_ERROR;
    }

    SPP_RetVal_t ret = SPP_SERVICES_DATABANK_packetData(p_packet, kpid.value, s_initEventsData.seq++, &sdInitStatus,
                                                        (spp_uint16_t)sizeof(sdInitStatus));

    if (ret != K_SPP_OK)
    {
        SPP_LOGE(k_svcTag, "packetData failed ret=%d", (int)ret);
        (void)SPP_SERVICES_DATABANK_returnPacket(p_packet);
        return ret;
    }

    (void)SPP_SERVICES_PUBSUB_publish(p_packet);

    return K_SPP_OK;
}