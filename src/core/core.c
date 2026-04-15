/**
 * @file core.c
 * @brief SPP core initialisation and port registry implementation.
 */

#include "spp/core/core.h"
#include "spp/core/returntypes.h"
#include "spp/core/error.h"
#include "spp/core/version.h"
#include "spp/services/databank.h"
#include "spp/services/db_flow.h"
#include "spp/services/log.h"

/* ----------------------------------------------------------------
 * Private state
 * ---------------------------------------------------------------- */

static const SPP_OsalPort_t *s_p_osalPort = NULL;
static const SPP_HalPort_t  *s_p_halPort  = NULL;

/* ----------------------------------------------------------------
 * Port registration
 * ---------------------------------------------------------------- */

SPP_RetVal_t SPP_Core_setOsalPort(const SPP_OsalPort_t *p_port)
{
    if (p_port == NULL)
    {
        SPP_ERR_RETURN(K_SPP_ERROR_NULL_POINTER);
    }
    s_p_osalPort = p_port;
    return K_SPP_OK;
}

SPP_RetVal_t SPP_Core_setHalPort(const SPP_HalPort_t *p_port)
{
    if (p_port == NULL)
    {
        SPP_ERR_RETURN(K_SPP_ERROR_NULL_POINTER);
    }
    s_p_halPort = p_port;
    return K_SPP_OK;
}

const SPP_OsalPort_t *SPP_Core_getOsalPort(void)
{
    return s_p_osalPort;
}

const SPP_HalPort_t *SPP_Core_getHalPort(void)
{
    return s_p_halPort;
}

/* ----------------------------------------------------------------
 * Core initialisation
 * ---------------------------------------------------------------- */

SPP_RetVal_t SPP_Core_init(void)
{
    if ((s_p_osalPort == NULL) || (s_p_halPort == NULL))
    {
        SPP_ERR_RETURN(K_SPP_ERROR_NOT_INITIALIZED);
    }

    SPP_RetVal_t ret = SPP_Log_init();
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    ret = SPP_Databank_init();
    if ((ret != K_SPP_OK) && (ret != K_SPP_ERROR_ALREADY_INITIALIZED))
    {
        return ret;
    }

    ret = SPP_DbFlow_init();
    if ((ret != K_SPP_OK) && (ret != K_SPP_ERROR_ALREADY_INITIALIZED))
    {
        return ret;
    }

    SPP_LOGI("SPP_CORE", "SPP core initialised (v%u.%u.%u)",
             K_SPP_VERSION_MAJOR, K_SPP_VERSION_MINOR, K_SPP_VERSION_PATCH);

    return K_SPP_OK;
}
