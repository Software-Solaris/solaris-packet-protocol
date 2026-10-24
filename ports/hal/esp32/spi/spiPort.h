/**
 * @file spiPort.h
 * @brief ESP32-S3 SPI port implementation.
 */

#ifndef SPP_PORTS_HAL_ESP32_SPI_PORT_H
#define SPP_PORTS_HAL_ESP32_SPI_PORT_H

#include "spp/core/types.h"
#include "spp/core/returnTypes.h"

/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */

SPP_RetVal_t SPP_PORTS_HAL_ESP32_spiBusInit(void);
void *SPP_PORTS_HAL_ESP32_spiGetHandle(spp_uint8_t deviceIdx);
SPP_RetVal_t SPP_PORTS_HAL_ESP32_spiDeviceInit(void *p_handle);
SPP_RetVal_t SPP_PORTS_HAL_ESP32_spiTransmit(void *p_handle, spp_uint8_t *p_data, spp_uint8_t length);
SPP_RetVal_t SPP_PORTS_HAL_ESP32_spiDeviceSetSpeed(void *p_handle, spp_uint32_t speedHz);

/**
 * @brief  Check whether the SPI bus has been initialised.
 * @return true if @ref SPP_PORTS_HAL_ESP32_spiBusInit succeeded.
 */
spp_bool_t SPP_PORTS_HAL_ESP32_spiIsBusInitialized(void);

#endif /* SPP_PORTS_HAL_ESP32_SPI_PORT_H */
