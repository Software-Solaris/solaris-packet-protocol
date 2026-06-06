/**
 * @file core.c
 * @brief SPP core initialisation and port registry implementation.
 */

#include "spp/core/core.h"
#include "spp/core/returnTypes.h"
#include "spp/core/error.h"
#include "spp/core/version.h"
#include "spp/core/packet.h"
#include "spp/core/types.h"
#include "spp/services/databank/databank.h"
#include "spp/services/pubsub/pubsub.h"
#include "spp/services/log/log.h"

#include <stdio.h>


/* ----------------------------------------------------------------
* VARIABLES
* ---------------------------------------------------------------- */
static spp_uint16_t s_logSeq = 0U;
static spp_bool_t s_logBusy = false;


/* ----------------------------------------------------------------
* STATIC FUNCTIONS DECLARATIONS
* ---------------------------------------------------------------- */
static void coreLogOutput(const char *p_tag, SPP_LogLevel_t level, const char *p_message);

/* ----------------------------------------------------------------
* PUBLIC FUNCTIONS
* ---------------------------------------------------------------- */

SPP_RetVal_t SPP_CORE_init(void)
{
    SPP_RetVal_t ret = SPP_SERVICES_LOG_init();
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_SERVICES_DATABANK_init();
    if ((ret != K_SPP_OK) && (ret != K_SPP_ERROR_ALREADY_INITIALIZED))
    {
        return ret;
    }

    // Init all the producers and consumer init functions
    ret = SPP_SERVICES_PUBSUB_init();
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    // TODO: This to be pending on how it works in the overall scheme
    // SPP_SERVICES_PUBSUB_init();

    // SPP_SERVICES_LOG_setOutput(coreLogOutput);

    SPP_LOGI("SPP_CORE", "SPP core initialised (v%u.%u.%u)", K_SPP_VERSION_MAJOR, K_SPP_VERSION_MINOR,
             K_SPP_VERSION_PATCH);

    return K_SPP_OK;
}
