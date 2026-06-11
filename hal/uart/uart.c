/**
 * @file uart.c
 * @brief HAL UART functions implementation. Calls the function pointer and returns the result.
 */

/* ----------------------------------------------------------------
 * INCLUDES
 * ---------------------------------------------------------------- */
#include "spp/core/returnTypes.h"
#include "spp/hal/hal.h"
#include "spp/hal/uart/uart.h"

/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */

SPP_RetVal_t SPP_HAL_SPI_portInit(void *p_cfg)
{
    SPP_RetVal_t ret = K_SPP_OK;
    const SPP_HalPort_t *p_port = SPP_HAL_getPort();

    if (p_port == NULL)
    {
        ret = K_SPP_ERROR_NULL_POINTER;
        return ret;
    }

    if (p_port->uart.uartPortInit == NULL)
    {
        ret = K_SPP_ERROR;
        return ret;
    }

    if (p_cfg == NULL)
    {
        ret = K_SPP_ERROR;
        return ret;
    }

    ret = p_port->uart.uartPortInit(p_cfg);

    return ret;
}
