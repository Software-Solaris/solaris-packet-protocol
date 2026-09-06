/**
 * @file spi.c
 * @brief HAL SPI functions implementation. Calls the function pointer and returns the result.
 */


/* ----------------------------------------------------------------
 * INCLUDES
 * ---------------------------------------------------------------- */
#include "spp/hal/hal.h"
#include "spp/hal/time/time.h"


/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */
spp_uint32_t SPP_HAL_TIME_getTimeMs(void)
{
    const SPP_HalPort_t *p_port = SPP_HAL_getPort();
    if ((p_port == NULL) || (p_port->time.getTimeMs == NULL))
    {
        return 0U;
    }
    return p_port->time.getTimeMs();
}

spp_uint32_t SPP_HAL_TIME_getTimeUs(void)
{
    const SPP_HalPort_t *p_port = SPP_HAL_getPort();
    if ((p_port == NULL) || (p_port->time.getTimeUs == NULL))
    {
        return 0U;
    }
    return p_port->time.getTimeUs();
}

void SPP_HAL_TIME_delayMs(spp_uint32_t ms)
{
    const SPP_HalPort_t *p_port = SPP_HAL_getPort();
    if ((p_port != NULL) && (p_port->time.delayMs != NULL))
    {
        p_port->time.delayMs(ms);
    }
}