/**
 * @file service.c
 * @brief SPP module registry implementation.
 */

#include "spp/services/service.h"
#include "spp/services/pubsub/pubsub.h"
#include "spp/core/error.h"
#include "spp/services/log/log.h"

/* ----------------------------------------------------------------
 * Private state
 * ---------------------------------------------------------------- */

static const char *const k_tag = "SPP_SVC";

typedef struct
{
    const SPP_Module_t *p_module;
    void               *p_ctx;
} ServiceEntry_t;

static ServiceEntry_t s_registry[K_SPP_MAX_SERVICES];
static spp_uint32_t   s_count = 0U;

/* ----------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------- */

SPP_RetVal_t SPP_SERVICES_register(const SPP_Module_t *p_module, void *p_ctx)
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
    s_count++;

    if (p_module->init != NULL)
    {
        SPP_RetVal_t ret = p_module->init(p_ctx);
        if (ret != K_SPP_OK)
        {
            SPP_LOGE(k_tag, "Module '%s' init failed (%d)", p_module->p_name, (int)ret);
            s_count--;
            SPP_ERR_RETURN(ret);
        }
    }

    if (p_module->start != NULL)
    {
        SPP_RetVal_t ret = p_module->start(p_ctx);
        if (ret != K_SPP_OK)
        {
            SPP_LOGE(k_tag, "Module '%s' start failed (%d)", p_module->p_name, (int)ret);
            s_count--;
            SPP_ERR_RETURN(ret);
        }
    }

    if (p_module->onPacket != NULL)
    {
        (void)SPP_SERVICES_PUBSUB_subscribe(p_module->consumesApid, p_module->onPacketPrio,
                                             p_module->onPacket, p_ctx);
    }

    SPP_LOGI(k_tag, "Registered '%s' (apid=0x%04X)", p_module->p_name, p_module->apid);
    return K_SPP_OK;
}

SPP_RetVal_t SPP_SERVICES_callProducers(void)
{
    for (spp_uint32_t i = 0U; i < s_count; i++)
    {
        const ServiceEntry_t *p_entry = &s_registry[i];
        if (p_entry->p_module->produce != NULL)
        {
            p_entry->p_module->produce(p_entry->p_ctx);
        }
    }
    return K_SPP_OK;
}

void SPP_SERVICES_callConsumers(void)
{
    SPP_SERVICES_PUBSUB_callConsumers();
}

spp_uint32_t SPP_SERVICES_count(void)
{
    return s_count;
}
