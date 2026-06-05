/**
 * @file spi.c
 * @brief HAL SPI functions implementation. Calls the function pointer and returns the result.
 */


/* ----------------------------------------------------------------
 * INCLUDES
 * ---------------------------------------------------------------- */
#include "spp/core/returnTypes.h"
#include "spp/hal/hal.h"
#include "spp/hal/gpio/gpio.h"


/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */

SPP_RetVal_t SPP_HAL_GPIO_configInterrupt(spp_uint32_t pin, spp_uint32_t intrType,
                                          spp_uint32_t pull)
{
    SPP_RetVal_t ret = K_SPP_ERROR;
    const SPP_HalPort_t *p_port = SPP_HAL_getPort();
    if ((p_port == NULL) || (p_port->gpio.gpioConfigInterrupt == NULL))
    {
        ret = K_SPP_ERROR_NULL_POINTER;
    }
    else
    {
        ret = p_port->gpio.gpioConfigInterrupt(pin, intrType, pull);
        if (ret != K_SPP_OK)
        {
            ret = K_SPP_ERROR;
        }
    }
    return ret;
}

SPP_RetVal_t SPP_HAL_GPIO_registerIsr(spp_uint32_t pin, void *p_isrCtx)
{
    SPP_RetVal_t ret = K_SPP_ERROR;
    const SPP_HalPort_t *p_port = SPP_HAL_getPort();
    if ((p_port == NULL) || (p_port->gpio.gpioRegisterIsr == NULL))
    {
        ret = K_SPP_ERROR_NULL_POINTER;
    }
    else
    {
        ret = p_port->gpio.gpioRegisterIsr(pin, p_isrCtx);
        if (ret != K_SPP_OK)
        {
            ret = K_SPP_ERROR;
        }
    }

    return ret;
}
