/**
 * @file spi.c
 * @brief HAL SPI functions implementation. Calls the function pointer and returns the result.
 */


/* ----------------------------------------------------------------
 * INCLUDES
 * ---------------------------------------------------------------- */
#include "spp/core/returnTypes.h"
#include "spp/hal/hal.h"
#include "spp/hal/storage/storage.h"


/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */

SPP_RetVal_t SPP_HAL_STORAGE_mount(void *p_cfg)
{
    SPP_RetVal_t ret = K_SPP_ERROR;
    const SPP_HalPort_t *p_port = SPP_HAL_getPort();
    if (p_port == NULL)
    {
        ret = K_SPP_ERROR_NULL_POINTER;
    }
    else
    {
        ret = p_port->storage.storageMount(p_cfg);
        if (ret != K_SPP_OK)
        {
            ret = K_SPP_ERROR;
        }
    }
    return ret;
}

SPP_RetVal_t SPP_HAL_STORAGE_unmount(void *p_cfg)
{
    SPP_RetVal_t ret = K_SPP_ERROR;
    const SPP_HalPort_t *p_port = SPP_HAL_getPort();
    if (p_port == NULL)
    {
        ret = K_SPP_ERROR_NULL_POINTER;
    }
    else
    {
        ret = p_port->storage.storageUnmount(p_cfg);
        if (ret != K_SPP_OK)
        {
            ret = K_SPP_ERROR;
        }
    }

    return ret;
}