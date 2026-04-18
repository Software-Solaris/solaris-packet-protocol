/**
 * @file core.c
 * @brief SPP core initialisation and port registry implementation.
 */

#include "spp/core/core.h"
#include "spp/core/returnTypes.h"
#include "spp/core/error.h"
#include "spp/core/version.h"
#include "spp/services/databank/databank.h"
#include "spp/services/pubsub/pubsub.h"
#include "spp/services/log/log.h"

/* ----------------------------------------------------------------
 * Private state
 * ---------------------------------------------------------------- */

static const SPP_HalPort_t *s_p_halPort = NULL;

/* ----------------------------------------------------------------
 * Port registration
 * ---------------------------------------------------------------- */

SPP_RetVal_t SPP_CORE_setHalPort(const SPP_HalPort_t *p_port)
{
    if (p_port == NULL)
    {
        SPP_ERR_RETURN(K_SPP_ERROR_NULL_POINTER);
    }
    s_p_halPort = p_port;
    return K_SPP_OK;
}

const SPP_HalPort_t *SPP_CORE_getHalPort(void)
{
    return s_p_halPort;
}

/* ----------------------------------------------------------------
 * Core initialisation
 * ---------------------------------------------------------------- */

SPP_RetVal_t SPP_CORE_init(void)
{
    if (s_p_halPort == NULL)
    {
        SPP_ERR_RETURN(K_SPP_ERROR_NOT_INITIALIZED);
    }

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

    SPP_SERVICES_PUBSUB_init();

    SPP_LOGI("SPP_CORE", "SPP core initialised (v%u.%u.%u)", K_SPP_VERSION_MAJOR,
             K_SPP_VERSION_MINOR, K_SPP_VERSION_PATCH);

    return K_SPP_OK;
}
