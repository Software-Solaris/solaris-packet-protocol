/**
 * @file halStub.c
 * @brief Stub HAL port for host-side unit testing.
 *
 * All SPI, GPIO, UART, and storage functions return K_SPP_OK without doing any
 * real hardware access.  This allows the full SPP service layer to be
 * exercised on a development machine without an attached MCU.
 */

#include "spp/hal/hal.h"
#include "spp/core/returnTypes.h"
#include "spp/core/types.h"

#include <stdint.h>
#include <sys/time.h>

/* ----------------------------------------------------------------
 * Stub implementations
 * ---------------------------------------------------------------- */

static SPP_RetVal_t SPP_PORTS_HAL_STUB_spiBusInit(void)                                      { return K_SPP_OK; }
static void    *SPP_PORTS_HAL_STUB_spiGetHandle(spp_uint8_t deviceIdx)                   { (void)deviceIdx; return (void*)0x1; }
static SPP_RetVal_t SPP_PORTS_HAL_STUB_spiDeviceInit(void *p_handle)                         { (void)p_handle; return K_SPP_OK; }
static SPP_RetVal_t SPP_PORTS_HAL_STUB_spiTransmit(void *p_handle, spp_uint8_t *p_data, spp_uint8_t len)
{
    (void)p_handle; (void)p_data; (void)len;
    return K_SPP_OK;
}
static SPP_RetVal_t SPP_PORTS_HAL_STUB_spiDeviceSetSpeed(void *p_handle, spp_uint32_t speedHz)
{
    (void)p_handle; (void)speedHz;
    return K_SPP_OK;
}

static SPP_RetVal_t SPP_PORTS_HAL_STUB_gpioConfigInterrupt(spp_uint32_t pin, spp_uint32_t t, spp_uint32_t pull)
{
    (void)pin; (void)t; (void)pull;
    return K_SPP_OK;
}
static SPP_RetVal_t SPP_PORTS_HAL_STUB_gpioRegisterIsr(spp_uint32_t pin, void *p_ctx)        { (void)pin; (void)p_ctx; return K_SPP_OK; }

static SPP_RetVal_t SPP_PORTS_HAL_STUB_storageInit(void)                                     { return K_SPP_OK; }
static SPP_RetVal_t SPP_PORTS_HAL_STUB_storageWrite(const void *p_buffer, spp_uint32_t first_block, spp_uint16_t count)
{
    (void)p_buffer; (void)first_block; (void)count;
    return K_SPP_OK;
}

static spp_uint32_t SPP_PORTS_HAL_STUB_getTimeMs(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (spp_uint32_t)((tv.tv_sec * 1000UL) + (tv.tv_usec / 1000UL));
}

static void SPP_PORTS_HAL_STUB_delayMs(spp_uint32_t ms) { (void)ms; }

static SPP_RetVal_t SPP_PORTS_HAL_STUB_uartPortInit(void)                                    { return K_SPP_OK; }
static SPP_RetVal_t SPP_PORTS_HAL_STUB_uartTransmit(const void *p_data, spp_uint32_t len)     { (void)p_data; (void)len; return K_SPP_OK; }
static SPP_RetVal_t SPP_PORTS_HAL_STUB_uartRead(void *p_data, spp_uint32_t len, spp_uint32_t *p_readBytes)
{
    (void)p_data; (void)len;
    if (p_readBytes != NULL)
    {
        *p_readBytes = 0U;
    }
    return K_SPP_OK;
}

/* ----------------------------------------------------------------
 * Port descriptor
 * ---------------------------------------------------------------- */

const SPP_HalPort_t g_stubHalPort = {
    .spi = {
        .spiBusInit         = SPP_PORTS_HAL_STUB_spiBusInit,
        .spiGetHandle       = SPP_PORTS_HAL_STUB_spiGetHandle,
        .spiDeviceInit      = SPP_PORTS_HAL_STUB_spiDeviceInit,
        .spiTransmit        = SPP_PORTS_HAL_STUB_spiTransmit,
        .spiDeviceSetSpeed  = SPP_PORTS_HAL_STUB_spiDeviceSetSpeed,
    },
    .gpio = {
        .gpioConfigInterrupt = SPP_PORTS_HAL_STUB_gpioConfigInterrupt,
        .gpioRegisterIsr     = SPP_PORTS_HAL_STUB_gpioRegisterIsr,
    },
    .storage = {
        .storageInit  = SPP_PORTS_HAL_STUB_storageInit,
        .storageWrite = SPP_PORTS_HAL_STUB_storageWrite,
    },
    .time = {
        .getTimeMs = SPP_PORTS_HAL_STUB_getTimeMs,
        .delayMs   = SPP_PORTS_HAL_STUB_delayMs,
    },
    .uart = {
        .uartPortInit = SPP_PORTS_HAL_STUB_uartPortInit,
        .uartTransmit = SPP_PORTS_HAL_STUB_uartTransmit,
        .uartRead     = SPP_PORTS_HAL_STUB_uartRead,
    },
};
