// Idea: no gestionar desde aquí nada que tenga que ver con los paquetes como tal (padding, crc, etc)
#include "datapool.h"
#include "osal/queue.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>

retval_t SPP_Datapool_Init(SPP_DatapoolHandle* p_handle, const SPP_DatapoolCfg* p_cfg)
{
    retval_t ret = SPP_OK;

    p_handle->cfg = p_cfg;

    // Colas 
    p_handle->q_free_handle = SPP_OSAL_QueueCreateStatic(
        (uint32_t)p_cfg->elem_count,
        (uint32_t)sizeof(void*),
        (uint8_t*)p_cfg->p_free_q_storage,
        p_cfg->p_free_q_ctrl
    );

    p_handle->q_ready_handle = SPP_OSAL_QueueCreateStatic(
        (uint32_t)p_cfg->elem_count,
        (uint32_t)sizeof(void*),
        (uint8_t*)p_cfg->p_ready_q_storage,
        p_cfg->p_ready_q_ctrl
    );

    // Crear FREE con los N punteros: base + i*elem_size (dirección de inicio del paquete i: x0+ix)
    {
        spp_uint8_t* base = (spp_uint8_t*)p_cfg->p_mem_base;

        for (spp_uint16_t i = 0; i < p_cfg->elem_count; i++)
        {
            void* p_slot = (void*)(base + ((spp_uint32_t)i * (spp_uint32_t)p_cfg->elem_size));

            ret = SPP_OSAL_QueueSend(p_handle->q_free_handle, &p_slot, 0);
            if (ret != SPP_OK)
            {
                return ret;
            }
        }
    }

    return ret;
}

retval_t SPP_Datapool_Acquire(SPP_DatapoolHandle* p_handle, const spp_uint32_t* p_timeout_ms, void* p_out_elem)
{
    retval_t ret;
    void* p_slot;
    uint32_t timeout_ms;

    if (p_timeout_ms == NULL)
    {
        timeout_ms = 0xFFFFFFFF; // infinito 
    }
    else
    {
        timeout_ms = (uint32_t)(*p_timeout_ms);
    }

    ret = SPP_OSAL_QueueReceive(p_handle->q_free_handle, &p_slot, timeout_ms);
    if (ret != SPP_OK)
    {
        return ret;
    }

    memcpy(p_out_elem, &p_slot, sizeof(void*)); // sizeof(void*): tamaño de un puntero (depende del micro)

    return ret;
}

retval_t SPP_Datapool_Publish(SPP_DatapoolHandle* p_handle, void* p_elem)
{
    retval_t ret;
    uint32_t timeout_ms;

    if (p_handle->cfg->overflow_policy == SPP_POOL_OVERFLOW_BLOCK)
    {
        timeout_ms = 0xFFFFFFFF;
    }
    else
    {
        timeout_ms = 0;      
    }

    // Intentar meter en READY
    ret = SPP_OSAL_QueueSend(p_handle->q_ready_handle, &p_elem, timeout_ms);
    if (ret != SPP_OK)
    {
        // No podemos "perder" el paquete: lo devolvemos a FREE
        if (p_handle->cfg->overflow_policy == SPP_POOL_OVERFLOW_DROP_NEW)
        {
            (void)SPP_OSAL_QueueSend(p_handle->q_free_handle, &p_elem, 0u);
        }

        return ret;
    }

    return SPP_OK;
}

retval_t SPP_Datapool_ReceiveReady(SPP_DatapoolHandle* p_handle, const spp_uint32_t* p_timeout_ms, void* p_out_elem)
{
    retval_t ret;
    void* p_slot;
    uint32_t timeout_ms;

    if (p_timeout_ms == NULL)
    {
        timeout_ms = 0xFFFFFFFF; // infinito 
    }

    else
    {
        timeout_ms = (uint32_t)(*p_timeout_ms);
    }

    ret = SPP_OSAL_QueueReceive(p_handle->q_ready_handle, &p_slot, timeout_ms);
    if (ret != SPP_OK)
    {
        return ret;
    }

    memcpy(p_out_elem, &p_slot, sizeof(void*)); // sizeof(void*): tamaño de un puntero (depende del micro)

    return ret;
}

retval_t SPP_Datapool_Release(SPP_DatapoolHandle* p_handle, void* p_elem)
{
    retval_t ret;

    ret = SPP_OSAL_QueueSend(p_handle->q_free_handle, &p_elem, 0u);
    if (ret != SPP_OK)
    {
        return ret;
    }

    return ret;
}

retval_t SPP_Datapool_Reset(SPP_DatapoolHandle* p_handle)
{
    retval_t ret;

    // Vaciar las colas
    ret = SPP_OSAL_QueueReset(p_handle->q_free_handle);
    if (ret != SPP_OK)
    {
        return ret;
    }

    ret = SPP_OSAL_QueueReset(p_handle->q_ready_handle);
    if (ret != SPP_OK)
    {
        return ret;
    }

    // Reinsertar todos los punteros en FREE
    {
        spp_uint8_t* base = (spp_uint8_t*)p_handle->cfg->p_mem_base;

        for (spp_uint16_t i = 0; i < p_handle->cfg->elem_count; i++)
        {
            void* p_slot = (void*)(base + ((spp_uint32_t)i * (spp_uint32_t)p_handle->cfg->elem_size));

            ret = SPP_OSAL_QueueSend(p_handle->q_free_handle, &p_slot, 0);
            if (ret != SPP_OK)
            {
                return ret;
            }
        }
    }

    return ret;
}