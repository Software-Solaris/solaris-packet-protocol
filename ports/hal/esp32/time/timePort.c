/**
 * @file timePort.c
 * @brief ESP32-S3 time port implementation.
 */

/* ----------------------------------------------------------------
 * INCLUDES
 * ---------------------------------------------------------------- */
#include "timePort.h"

#include "esp_timer.h"

/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */

spp_uint32_t SPP_PORTS_HAL_ESP32_getTimeMs(void)
{
    return (spp_uint32_t)(esp_timer_get_time() / 1000LL);
}

spp_uint32_t SPP_PORTS_HAL_ESP32_getTimeUs(void)
{
    return (spp_uint32_t)(esp_timer_get_time());
}

void SPP_PORTS_HAL_ESP32_delayMs(spp_uint32_t ms)
{
    spp_uint32_t start = SPP_PORTS_HAL_ESP32_getTimeMs();
    while ((SPP_PORTS_HAL_ESP32_getTimeMs() - start) < ms)
    { /* busy-wait */
    }
}
