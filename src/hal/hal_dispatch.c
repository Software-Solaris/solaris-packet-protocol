/**
 * @file hal_dispatch.c
 * @brief HAL dispatch — forwards every call through the registered port.
 */

#include "spp/hal/spi.h"
#include "spp/hal/gpio.h"
#include "spp/hal/storage.h"
#include "spp/core/core.h"
#include "spp/core/returntypes.h"

/* ----------------------------------------------------------------
 * Internal helper
 * ---------------------------------------------------------------- */

static inline const SPP_HalPort_t *getPort(void)
{
    return SPP_Core_getHalPort();
}

/* ----------------------------------------------------------------
 * SPI dispatch
 * ---------------------------------------------------------------- */

retval_t SPP_Hal_spiBusInit(void)
{
    const SPP_HalPort_t *p_port = getPort();
    if ((p_port == NULL) || (p_port->spiBusInit == NULL))
    {
        return SPP_ERROR_NO_PORT;
    }
    return p_port->spiBusInit();
}

void *SPP_Hal_spiGetHandle(spp_uint8_t deviceIdx)
{
    const SPP_HalPort_t *p_port = getPort();
    if ((p_port == NULL) || (p_port->spiGetHandle == NULL))
    {
        return NULL;
    }
    return p_port->spiGetHandle(deviceIdx);
}

retval_t SPP_Hal_spiDeviceInit(void *p_handle)
{
    const SPP_HalPort_t *p_port = getPort();
    if ((p_port == NULL) || (p_port->spiDeviceInit == NULL))
    {
        return SPP_ERROR_NO_PORT;
    }
    return p_port->spiDeviceInit(p_handle);
}

retval_t SPP_Hal_spiTransmit(void *p_handle, spp_uint8_t *p_data,
                               spp_uint8_t length)
{
    const SPP_HalPort_t *p_port = getPort();
    if ((p_port == NULL) || (p_port->spiTransmit == NULL))
    {
        return SPP_ERROR_NO_PORT;
    }
    return p_port->spiTransmit(p_handle, p_data, length);
}

/* ----------------------------------------------------------------
 * GPIO dispatch
 * ---------------------------------------------------------------- */

retval_t SPP_Hal_gpioConfigInterrupt(spp_uint32_t pin, spp_uint32_t intrType,
                                      spp_uint32_t pull)
{
    const SPP_HalPort_t *p_port = getPort();
    if ((p_port == NULL) || (p_port->gpioConfigInterrupt == NULL))
    {
        return SPP_ERROR_NO_PORT;
    }
    return p_port->gpioConfigInterrupt(pin, intrType, pull);
}

retval_t SPP_Hal_gpioRegisterIsr(spp_uint32_t pin, void *p_isrCtx)
{
    const SPP_HalPort_t *p_port = getPort();
    if ((p_port == NULL) || (p_port->gpioRegisterIsr == NULL))
    {
        return SPP_ERROR_NO_PORT;
    }
    return p_port->gpioRegisterIsr(pin, p_isrCtx);
}

/* ----------------------------------------------------------------
 * Storage dispatch
 * ---------------------------------------------------------------- */

retval_t SPP_Hal_storageMount(void *p_cfg)
{
    const SPP_HalPort_t *p_port = getPort();
    if (p_port == NULL)
    {
        return SPP_ERROR_NO_PORT;
    }
    if (p_port->storageMount == NULL)
    {
        return SPP_OK; /* Optional — treat as no-op. */
    }
    return p_port->storageMount(p_cfg);
}

retval_t SPP_Hal_storageUnmount(void *p_cfg)
{
    const SPP_HalPort_t *p_port = getPort();
    if (p_port == NULL)
    {
        return SPP_ERROR_NO_PORT;
    }
    if (p_port->storageUnmount == NULL)
    {
        return SPP_OK;
    }
    return p_port->storageUnmount(p_cfg);
}
