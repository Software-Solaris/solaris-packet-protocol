/**
 * @file uartPort.h
 * @brief ESP32-S3 UART port implementation.
 */

#ifndef SPP_PORTS_HAL_ESP32_UART_PORT_H
#define SPP_PORTS_HAL_ESP32_UART_PORT_H

#include "spp/core/types.h"
#include "spp/core/returnTypes.h"

/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */

SPP_RetVal_t SPP_PORTS_HAL_ESP32_uartPortInit(void);
SPP_RetVal_t SPP_PORTS_HAL_ESP32_uartTransmit(const void *p_data, spp_uint32_t len);
SPP_RetVal_t SPP_PORTS_HAL_ESP32_uartRead(void *p_data, spp_uint32_t len, spp_uint32_t *p_readBytes);

#endif /* SPP_PORTS_HAL_ESP32_UART_PORT_H */
