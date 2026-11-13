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

/**
 * @brief Returns the storage location of an SPI device handle.
 * @param deviceIdx Index of the SPI device.
 * @return Pointer to the static handle, or NULL if the index is invalid.
 */
void *SPP_PORTS_HAL_ESP32_spiGetHandle(spp_uint8_t deviceIdx);
SPP_RetVal_t SPP_PORTS_HAL_ESP32_spiDeviceInit(void *p_handle);

/**
 * @brief Performs register-based SPI transactions.
 *
 * Uses bit 7 of each command byte to distinguish reads (3 bytes)
 * from writes (2 bytes). Read responses overwrite the input buffer.
 *
 * @param p_handle Pointer to the SPI device handle.
 * @param p_data Transaction buffer.
 * @param length Total buffer length in bytes.
 * @return K_SPP_OK on success, error code otherwise.
 */
SPP_RetVal_t SPP_PORTS_HAL_ESP32_spiTransmit(void *p_handle, spp_uint8_t *p_data, spp_uint8_t length);

/**
 * @brief Reconfigures an SPI device with a new clock frequency.
 *
 * Removes and re-adds the device using the SD card CS pin.
 *
 * @param p_handle Pointer to the SPI device handle.
 * @param speedHz New SPI clock frequency in Hz.
 * @return K_SPP_OK on success, error code otherwise.
 */
SPP_RetVal_t SPP_PORTS_HAL_ESP32_spiDeviceSetSpeed(void *p_handle, spp_uint32_t speedHz);

/**
 * @brief Performs a raw full-duplex SPI transaction.
 *
 * Transmitted data is overwritten by the received data.
 *
 * @param p_handle Pointer to the SPI device handle.
 * @param p_data Transmission and reception buffer.
 * @param length Number of bytes to transfer.
 * @return K_SPP_OK on success, error code otherwise.
 */
SPP_RetVal_t SPP_PORTS_HAL_ESP32_spiTransmitRaw(void *p_handle, spp_uint8_t *p_data, spp_uint8_t length);

/**
 * @brief  Check whether the SPI bus has been initialised.
 * @return true if @ref SPP_PORTS_HAL_ESP32_spiBusInit succeeded.
 */
spp_bool_t SPP_PORTS_HAL_ESP32_spiIsBusInitialized(void);

#endif /* SPP_PORTS_HAL_ESP32_SPI_PORT_H */
