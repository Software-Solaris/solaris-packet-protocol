/**
 * @file spi.c
 * @brief HAL SPI functions implementation. Calls the function pointer and returns the result.
 */


/* ----------------------------------------------------------------
 * INCLUDES
 * ---------------------------------------------------------------- */
#include "spp/core/returnTypes.h"
#include "spp/hal/hal.h"
#include "spp/hal/spi/spi.h"


/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */

SPP_RetVal_t SPP_HAL_SPI_busInit(void)
{
    SPP_RetVal_t ret = K_SPP_OK;
    const SPP_HalPort_t *p_port = SPP_HAL_getPort();
    if ((p_port == NULL) || (p_port->spi.spiBusInit == NULL))
    {
        ret = K_SPP_ERROR_NULL_POINTER;
    }
    else
    {
        ret = p_port->spi.spiBusInit();
        if (ret != K_SPP_OK)
        {
            ret = K_SPP_ERROR;
        }
    }

    return ret;
}

void *SPP_HAL_SPI_getHandle(spp_uint8_t deviceIdx)
{
    const SPP_HalPort_t *p_port = SPP_HAL_getPort();
    if ((p_port == NULL) || (p_port->spi.spiGetHandle == NULL))
    {
        return NULL;
    }
    return p_port->spi.spiGetHandle(deviceIdx);
}

SPP_RetVal_t SPP_HAL_SPI_deviceInit(void *p_handle)
{
    SPP_RetVal_t ret = K_SPP_ERROR;
    const SPP_HalPort_t *p_port = SPP_HAL_getPort();
    if ((p_port == NULL) || (p_port->spi.spiDeviceInit == NULL))
    {
        ret = K_SPP_ERROR_NULL_POINTER;
    }
    else
    {
        ret = p_port->spi.spiDeviceInit(p_handle);
        if (ret != K_SPP_OK)
        {
            ret = K_SPP_ERROR;
        }
    }

    return ret;
}

SPP_RetVal_t SPP_HAL_SPI_transmit(void *p_handle, spp_uint8_t *p_data, spp_uint8_t length)
{
    SPP_RetVal_t ret = K_SPP_ERROR;
    const SPP_HalPort_t *p_port = SPP_HAL_getPort();
    if ((p_port == NULL) || (p_port->spi.spiTransmit == NULL))
    {
        ret = K_SPP_ERROR_NULL_POINTER;
    }
    else
    {
        ret = p_port->spi.spiTransmit(p_handle, p_data, length);
        if (ret != K_SPP_OK)
        {
            ret = K_SPP_ERROR;
        }
    }
    return ret;
}