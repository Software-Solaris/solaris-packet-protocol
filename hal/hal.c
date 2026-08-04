/**
 * @file hal.c
 * @brief HAL port. All the necessary functions needed to setup the HAL ports and
 *        to get the pointers to the functions that are used by the SPP library.    
 */


/* ----------------------------------------------------------------
 * INCLUDES
 * ---------------------------------------------------------------- */
#include "spp/core/returnTypes.h"
#include "spp/hal/hal.h"
#include "spp/hal/spi/spi.h"
#include "spp/hal/uart/uart.h"

/* ----------------------------------------------------------------
 * VARIABLES
 * ---------------------------------------------------------------- */

static const SPP_HalPort_t *s_p_halPort = {NULL}; /**< Pointer to the HAL ports struct. */

/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */

SPP_RetVal_t SPP_HAL_init(void *p_halPorts)
{
    const SPP_HalPort_t *p_port = (SPP_HalPort_t *)p_halPorts;
    SPP_RetVal_t ret = K_SPP_ERROR;
    if (p_port == NULL)
    {
        return K_SPP_ERROR_NULL_POINTER;
    }
    s_p_halPort = p_port;

    /* Init the UART port */
    ret = SPP_HAL_UART_portInit();
    if (ret != K_SPP_OK)
    {
        return K_SPP_ERROR;
    }

    /* Init the SPI bus */
    ret = SPP_HAL_SPI_busInit();
    if (ret != K_SPP_OK)
    {
        return ret;
    }
    return K_SPP_OK;
}

const SPP_HalPort_t *SPP_HAL_getPort(void)
{
    return (SPP_HalPort_t *)s_p_halPort;
}
