/**
 * @file service.c
 * @brief SPP service registry implementation.
 */

#include "spp/services/service.h"
#include "spp/services/log.h"

#include <string.h>

/* ----------------------------------------------------------------
 * Private state
 * ---------------------------------------------------------------- */

static const char *const k_tag = "SPP_SVC";

typedef struct
{
    const SPP_ServiceDesc_t *p_desc;
    void                    *p_ctx;
    const void              *p_cfg;
} ServiceEntry_t;

static ServiceEntry_t s_registry[K_SPP_MAX_SERVICES];
static spp_uint32_t   s_count = 0U;

/* ----------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------- */

retval_t SPP_Service_register(const SPP_ServiceDesc_t *p_desc,
                               void *p_ctx, const void *p_cfg)
{
    if ((p_desc == NULL) || (p_ctx == NULL))
    {
        return SPP_ERROR_NULL_POINTER;
    }

    if (s_count >= K_SPP_MAX_SERVICES)
    {
        SPP_LOGE(k_tag, "Registry full — cannot register '%s'", p_desc->p_name);
        return SPP_ERROR_REGISTRY_FULL;
    }

    s_registry[s_count].p_desc = p_desc;
    s_registry[s_count].p_ctx  = p_ctx;
    s_registry[s_count].p_cfg  = p_cfg;
    s_count++;

    SPP_LOGI(k_tag, "Registered service '%s' (apid=0x%04X)", p_desc->p_name, p_desc->apid);
    return SPP_OK;
}

retval_t SPP_Service_initAll(void)
{
    retval_t result = SPP_OK;

    for (spp_uint32_t i = 0U; i < s_count; i++)
    {
        const ServiceEntry_t *p_entry = &s_registry[i];
        if (p_entry->p_desc->init == NULL)
        {
            continue;
        }

        retval_t ret = p_entry->p_desc->init(p_entry->p_ctx, p_entry->p_cfg);
        if (ret != SPP_OK)
        {
            SPP_LOGE(k_tag, "Service '%s' init failed (%d)",
                     p_entry->p_desc->p_name, (int)ret);
            result = ret;
        }
    }
    return result;
}

retval_t SPP_Service_startAll(void)
{
    retval_t result = SPP_OK;

    for (spp_uint32_t i = 0U; i < s_count; i++)
    {
        const ServiceEntry_t *p_entry = &s_registry[i];
        if (p_entry->p_desc->start == NULL)
        {
            continue;
        }

        retval_t ret = p_entry->p_desc->start(p_entry->p_ctx);
        if (ret != SPP_OK)
        {
            SPP_LOGE(k_tag, "Service '%s' start failed (%d)",
                     p_entry->p_desc->p_name, (int)ret);
            result = ret;
        }
    }
    return result;
}

retval_t SPP_Service_stopAll(void)
{
    retval_t result = SPP_OK;

    /* Stop in reverse registration order. */
    for (spp_uint32_t i = s_count; i > 0U; i--)
    {
        const ServiceEntry_t *p_entry = &s_registry[i - 1U];
        if (p_entry->p_desc->stop == NULL)
        {
            continue;
        }

        retval_t ret = p_entry->p_desc->stop(p_entry->p_ctx);
        if (ret != SPP_OK)
        {
            SPP_LOGE(k_tag, "Service '%s' stop failed (%d)",
                     p_entry->p_desc->p_name, (int)ret);
            result = ret;
        }
    }
    return result;
}

spp_uint32_t SPP_Service_count(void)
{
    return s_count;
}
