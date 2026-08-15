/**
 * @file MAX-M10M-20B_service.c
 * @brief Ublox GNSS (MAX-M10M-20B) driver + SPP service implementation.
 */

#include "spp/services/MAX-M10M-20B/MAX-M10M-20B.h"

#include "spp/hal/uart/uart.h"
#include "spp/hal/time/time.h"
#include "spp/services/log/log.h"
#include "spp/services/databank/databank.h"
#include "spp/core/packet.h"
#include "spp/core/pubsub/pubsub.h"
#include "spp/services/fsm/fsm.h"

/* ----------------------------------------------------------------
 * CONSTANTS
 * ---------------------------------------------------------------- */
#define K_M10M_TAG "MAX-M10M-20B"


/* ----------------------------------------------------------------
 * STATIC FUNCTIONS DECLARATIONS
 * ---------------------------------------------------------------- */
static SPP_RetVal_t SPP_SERVICES_M10M_init(void);
static SPP_RetVal_t SPP_SERVICES_M10M_acquireData(SPP_Kpid_t kpid);
/* ----------------------------------------------------------------
* VARIABLES
* ---------------------------------------------------------------- */
static const SPP_SERVICE_ProducerContract_t g_m10mProducerContract = {.p_nameProducer = "bmp390",
                                                                      .tiemoutMs = K_M10M_TIMEOUT_MS,
                                                                      .init = SPP_SERVICES_M10M_init,
                                                                      .acquireData = SPP_SERVICES_M10M_acquireData};

/* ----------------------------------------------------------------
* PUBLIC FUNCTIONS
* ---------------------------------------------------------------- */
const SPP_SERVICE_ProducerContract_t *SPP_SERVICES_M10M_getProducerContract(void)
{
    return &g_m10mProducerContract;
}