/**
 * @file hal_stub.c
 * @brief Stub HAL port for host-side unit testing.
 *
 * All SPI, GPIO, and storage functions return K_SPP_OK without doing any real
 * hardware access.  This allows the full SPP service layer to be exercised
 * on a development machine without an attached MCU.
 */

#include "spp/hal/port.h"
#include "spp/core/returntypes.h"
#include "spp/core/types.h"

#include <stdint.h>
#include <sys/time.h>

/* ----------------------------------------------------------------
 * Stub implementations
 * ---------------------------------------------------------------- */

static SPP_RetVal_t stubSpiBusInit(void)                                      { return K_SPP_OK; }
static void    *stubSpiGetHandle(spp_uint8_t deviceIdx)                   { (void)deviceIdx; return (void*)0x1; }
static SPP_RetVal_t stubSpiDeviceInit(void *p_handle)                         { (void)p_handle; return K_SPP_OK; }
static SPP_RetVal_t stubSpiTransmit(void *p_handle, spp_uint8_t *p_data, spp_uint8_t len)
{
    (void)p_handle; (void)p_data; (void)len;
    return K_SPP_OK;
}
static SPP_RetVal_t stubGpioConfigInterrupt(spp_uint32_t pin, spp_uint32_t t, spp_uint32_t pull)
{
    (void)pin; (void)t; (void)pull;
    return K_SPP_OK;
}
static SPP_RetVal_t stubGpioRegisterIsr(spp_uint32_t pin, void *p_ctx)        { (void)pin; (void)p_ctx; return K_SPP_OK; }
static SPP_RetVal_t stubStorageMount(void *p_cfg)                             { (void)p_cfg; return K_SPP_OK; }
static SPP_RetVal_t stubStorageUnmount(void *p_cfg)                           { (void)p_cfg; return K_SPP_OK; }

static spp_uint32_t stubGetTimeMs(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (spp_uint32_t)((tv.tv_sec * 1000UL) + (tv.tv_usec / 1000UL));
}

static void stubDelayMs(spp_uint32_t ms) { (void)ms; }

/* ----------------------------------------------------------------
 * Port descriptor
 * ---------------------------------------------------------------- */

const SPP_HalPort_t g_stubHalPort = {
    .spiBusInit          = stubSpiBusInit,
    .spiGetHandle        = stubSpiGetHandle,
    .spiDeviceInit       = stubSpiDeviceInit,
    .spiTransmit         = stubSpiTransmit,
    .gpioConfigInterrupt = stubGpioConfigInterrupt,
    .gpioRegisterIsr     = stubGpioRegisterIsr,
    .storageMount        = stubStorageMount,
    .storageUnmount      = stubStorageUnmount,
    .getTimeMs           = stubGetTimeMs,
    .delayMs             = stubDelayMs,
};
