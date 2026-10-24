/**
 * @file gpioPort.h
 * @brief ESP32-S3 GPIO port implementation.
 */

#ifndef SPP_PORTS_HAL_ESP32_GPIO_PORT_H
#define SPP_PORTS_HAL_ESP32_GPIO_PORT_H

#include "spp/core/types.h"
#include "spp/core/returnTypes.h"

/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */

SPP_RetVal_t SPP_PORTS_HAL_ESP32_gpioConfigInterrupt(spp_uint32_t pin, spp_uint32_t intrType, spp_uint32_t pull);
SPP_RetVal_t SPP_PORTS_HAL_ESP32_gpioRegisterIsr(spp_uint32_t pin, void *p_isrCtx);
spp_uint16_t SPP_PORTS_HAL_ESP32_gpioRead(spp_uint32_t pin);
SPP_RetVal_t SPP_PORTS_HAL_ESP32_gpioWrite(spp_uint32_t pin, spp_uint8_t level);

#endif /* SPP_PORTS_HAL_ESP32_GPIO_PORT_H */
