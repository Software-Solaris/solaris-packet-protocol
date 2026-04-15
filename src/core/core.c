/**
 * @file core.c
 * @brief SPP core initialisation and port registry implementation.
 */

#include "spp/core/core.h"
#include "spp/core/returntypes.h"
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

retval_t SPP_Core_setOsalPort(const SPP_OsalPort_t *p_port)
{
    if (p_port == NULL)
    {
        return SPP_ERROR_NULL_POINTER;
    }
    s_p_osalPort = p_port;
    return SPP_OK;
}

retval_t SPP_Core_setHalPort(const SPP_HalPort_t *p_port)
{
    if (p_port == NULL)
    {
        return SPP_ERROR_NULL_POINTER;
    }
    s_p_halPort = p_port;
    return SPP_OK;
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

retval_t SPP_Core_init(void)
{
    if ((s_p_osalPort == NULL) || (s_p_halPort == NULL))
    {
        return SPP_ERROR_NOT_INITIALIZED;
    }

    retval_t ret = SPP_Log_init();
    if (ret != SPP_OK)
    {
        return ret;
    }

    ret = SPP_Databank_init();
    if ((ret != SPP_OK) && (ret != SPP_ERROR_ALREADY_INITIALIZED))
    {
        return ret;
    }

    ret = SPP_DbFlow_init();
    if ((ret != SPP_OK) && (ret != SPP_ERROR_ALREADY_INITIALIZED))
    {
        return ret;
    }

    SPP_LOGI("SPP_CORE", "SPP core initialised (v%u.%u.%u)",
             K_SPP_VERSION_MAJOR, K_SPP_VERSION_MINOR, K_SPP_VERSION_PATCH);

    return SPP_OK;
}
