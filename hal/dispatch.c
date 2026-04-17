/**
 * @file hal_dispatch.c
 * @brief HAL dispatch — forwards every call through the registered port.
 */

#include "spp/hal/spi.h"
#include "spp/hal/gpio.h"
#include "spp/hal/storage.h"
#include "spp/hal/time.h"
#include "spp/core/core.h"
#include "spp/core/returntypes.h"
#include "spp/core/error.h"

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

SPP_RetVal_t SPP_HAL_spiBusInit(void)
{
    const SPP_HalPort_t *p_port = getPort();
    if ((p_port == NULL) || (p_port->spiBusInit == NULL))
    {
        SPP_ERR_RETURN(K_SPP_ERROR_NO_PORT);
    }
    return p_port->spiBusInit();
}

void *SPP_HAL_spiGetHandle(spp_uint8_t deviceIdx)
{
    const SPP_HalPort_t *p_port = getPort();
    if ((p_port == NULL) || (p_port->spiGetHandle == NULL))
    {
        return NULL;
    }
    return p_port->spiGetHandle(deviceIdx);
}

SPP_RetVal_t SPP_HAL_spiDeviceInit(void *p_handle)
{
    const SPP_HalPort_t *p_port = getPort();
    if ((p_port == NULL) || (p_port->spiDeviceInit == NULL))
    {
        SPP_ERR_RETURN(K_SPP_ERROR_NO_PORT);
    }
    return p_port->spiDeviceInit(p_handle);
}

SPP_RetVal_t SPP_HAL_spiTransmit(void *p_handle, spp_uint8_t *p_data,
                               spp_uint8_t length)
{
    const SPP_HalPort_t *p_port = getPort();
    if ((p_port == NULL) || (p_port->spiTransmit == NULL))
    {
        SPP_ERR_RETURN(K_SPP_ERROR_NO_PORT);
    }
    return p_port->spiTransmit(p_handle, p_data, length);
}

/* ----------------------------------------------------------------
 * GPIO dispatch
 * ---------------------------------------------------------------- */

SPP_RetVal_t SPP_HAL_gpioConfigInterrupt(spp_uint32_t pin, spp_uint32_t intrType,
                                      spp_uint32_t pull)
{
    const SPP_HalPort_t *p_port = getPort();
    if ((p_port == NULL) || (p_port->gpioConfigInterrupt == NULL))
    {
        SPP_ERR_RETURN(K_SPP_ERROR_NO_PORT);
    }
    return p_port->gpioConfigInterrupt(pin, intrType, pull);
}

SPP_RetVal_t SPP_HAL_gpioRegisterIsr(spp_uint32_t pin, void *p_isrCtx)
{
    const SPP_HalPort_t *p_port = getPort();
    if ((p_port == NULL) || (p_port->gpioRegisterIsr == NULL))
    {
        SPP_ERR_RETURN(K_SPP_ERROR_NO_PORT);
    }
    return p_port->gpioRegisterIsr(pin, p_isrCtx);
}

/* ----------------------------------------------------------------
 * Storage dispatch
 * ---------------------------------------------------------------- */

SPP_RetVal_t SPP_HAL_storageMount(void *p_cfg)
{
    const SPP_HalPort_t *p_port = getPort();
    if (p_port == NULL)
    {
        SPP_ERR_RETURN(K_SPP_ERROR_NO_PORT);
    }
    if (p_port->storageMount == NULL)
    {
        return K_SPP_OK; /* Optional — treat as no-op. */
    }
    return p_port->storageMount(p_cfg);
}

SPP_RetVal_t SPP_HAL_storageUnmount(void *p_cfg)
{
    const SPP_HalPort_t *p_port = getPort();
    if (p_port == NULL)
    {
        SPP_ERR_RETURN(K_SPP_ERROR_NO_PORT);
    }
    if (p_port->storageUnmount == NULL)
    {
        return K_SPP_OK;
    }
    return p_port->storageUnmount(p_cfg);
}

/* ----------------------------------------------------------------
 * Time dispatch
 * ---------------------------------------------------------------- */

spp_uint32_t SPP_HAL_getTimeMs(void)
{
    const SPP_HalPort_t *p_port = getPort();
    if ((p_port == NULL) || (p_port->getTimeMs == NULL))
    {
        return 0U;
    }
    return p_port->getTimeMs();
}

void SPP_HAL_delayMs(spp_uint32_t ms)
{
    const SPP_HalPort_t *p_port = getPort();
    if ((p_port != NULL) && (p_port->delayMs != NULL))
    {
        p_port->delayMs(ms);
    }
}
