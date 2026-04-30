/**
 * @file service.c
 * @brief SPP module registry implementation.
 */

#include "spp/services/service.h"
#include "spp/services/pubsub/pubsub.h"
#include "spp/core/error.h"
#include "spp/services/log/log.h"

#include <string.h>

/* ----------------------------------------------------------------
 * Private state
 * ---------------------------------------------------------------- */

static const char *const k_tag = "SPP_SVC";

typedef struct
{
    const SPP_Module_t *p_module;
    void               *p_ctx;
    const void         *p_cfg;
} ServiceEntry_t;

static ServiceEntry_t s_registry[K_SPP_MAX_SERVICES];
static spp_uint32_t   s_count = 0U;

/* ----------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------- */

SPP_RetVal_t SPP_SERVICES_register(const SPP_Module_t *p_module,
                                    void *p_ctx, const void *p_cfg)
{
    if ((p_module == NULL) || (p_ctx == NULL))
    {
        SPP_ERR_RETURN(K_SPP_ERROR_NULL_POINTER);
    }

    if (s_count >= K_SPP_MAX_SERVICES)
    {
        SPP_LOGE(k_tag, "Registry full — cannot register '%s'", p_module->p_name);
        SPP_ERR_RETURN(K_SPP_ERROR_REGISTRY_FULL);
    }

    s_registry[s_count].p_module = p_module;
    s_registry[s_count].p_ctx    = p_ctx;
    s_registry[s_count].p_cfg    = p_cfg;
    s_count++;

    SPP_LOGI(k_tag, "Registered module '%s' (apid=0x%04X)", p_module->p_name, p_module->apid);

    /* Auto-subscribe if this module declares a packet handler. */
    if (p_module->onPacket != NULL)
    {
        (void)SPP_SERVICES_PUBSUB_subscribe(p_module->consumesApid, p_module->onPacketPrio,
                                             p_module->onPacket, p_ctx);
        SPP_LOGI(k_tag, "  subscribed '%s' to apid=0x%04X prio=%u",
                 p_module->p_name, p_module->consumesApid, (unsigned)p_module->onPacketPrio);
    }

    return K_SPP_OK;
}

SPP_RetVal_t SPP_SERVICES_initAll(void)
{
    SPP_RetVal_t result = K_SPP_OK;

    for (spp_uint32_t i = 0U; i < s_count; i++)
    {
        const ServiceEntry_t *p_entry = &s_registry[i];
        if (p_entry->p_module->init == NULL)
        {
            continue;
        }

        SPP_RetVal_t ret = p_entry->p_module->init(p_entry->p_ctx, p_entry->p_cfg);
        if (ret != K_SPP_OK)
        {
            SPP_LOGE(k_tag, "Module '%s' init failed (%d)",
                     p_entry->p_module->p_name, (int)ret);
            result = ret;
        }
    }
    return result;
}

SPP_RetVal_t SPP_SERVICES_startAll(void)
{
    SPP_RetVal_t result = K_SPP_OK;

    for (spp_uint32_t i = 0U; i < s_count; i++)
    {
        const ServiceEntry_t *p_entry = &s_registry[i];
        if (p_entry->p_module->start == NULL)
        {
            continue;
        }

        SPP_RetVal_t ret = p_entry->p_module->start(p_entry->p_ctx);
        if (ret != K_SPP_OK)
        {
            SPP_LOGE(k_tag, "Module '%s' start failed (%d)",
                     p_entry->p_module->p_name, (int)ret);
            result = ret;
        }
    }
    return result;
}

SPP_RetVal_t SPP_SERVICES_stopAll(void)
{
    SPP_RetVal_t result = K_SPP_OK;

    /* Stop in reverse registration order. */
    for (spp_uint32_t i = s_count; i > 0U; i--)
    {
        const ServiceEntry_t *p_entry = &s_registry[i - 1U];
        if (p_entry->p_module->stop == NULL)
        {
            continue;
        }

        SPP_RetVal_t ret = p_entry->p_module->stop(p_entry->p_ctx);
        if (ret != K_SPP_OK)
        {
            SPP_LOGE(k_tag, "Module '%s' stop failed (%d)",
                     p_entry->p_module->p_name, (int)ret);
            result = ret;
        }
    }
    return result;
}

SPP_RetVal_t SPP_SERVICES_pollAll(void)
{
    for (spp_uint32_t i = 0U; i < s_count; i++)
    {
        const ServiceEntry_t *p_entry = &s_registry[i];
        if (p_entry->p_module->serviceTask != NULL)
        {
            p_entry->p_module->serviceTask(p_entry->p_ctx);
        }
    }
    return K_SPP_OK;
}

spp_uint32_t SPP_SERVICES_count(void)
{
    return s_count;
}
