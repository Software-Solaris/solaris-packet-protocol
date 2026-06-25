/**
 * @file storage.c
 * @brief HAL storage implementation that dispatches operations to the registered port.


/* ----------------------------------------------------------------
 * INCLUDES
 * ---------------------------------------------------------------- */
#include "spp/core/returnTypes.h"
#include "spp/hal/hal.h"
#include "spp/hal/storage/storage.h"


/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */

SPP_RetVal_t SPP_HAL_STORAGE_init(void *p_handle)
{
    SPP_RetVal_t ret = K_SPP_OK;

    const SPP_HalPort_t *p_port = SPP_HAL_getPort();
    if (p_port == NULL)
    {
        ret = K_SPP_ERROR_NULL_POINTER;
    }
    else
    {
        ret = p_port->storage.storageInit(p_handle);
        if (ret != K_SPP_OK)
        {
            ret = K_SPP_ERROR;
        }
    }
    return ret;
}

SPP_RetVal_t SPP_HAL_STORAGE_write(void *p_handle, const void *p_buffer, spp_uint32_t first_block, spp_uint16_t count)
{
    SPP_RetVal_t ret = K_SPP_OK;

    const SPP_HalPort_t *p_port = SPP_HAL_getPort();
    if (p_port == NULL || p_buffer == NULL)
    {
        ret = K_SPP_ERROR_NULL_POINTER;
    }
    else
    {
        ret = p_port->storage.storageWrite(p_handle, p_buffer, first_block, count);
        if (ret != K_SPP_OK)
        {
            ret = K_SPP_ERROR;
        }
    }

    return ret;
}