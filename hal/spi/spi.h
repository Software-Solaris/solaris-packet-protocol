/**
 * @file spi.h
 * @brief SPI Hardware Abstraction Layer
 * @version 1.0.0
 * @date 2024
 * 
 * This file provides the SPI Hardware Abstraction Layer (HAL) interface
 * for the Solaris Packet Protocol library.
 */

#ifndef SPP_HAL_SPI_H
#define SPP_HAL_SPI_H

#include <stdint.h>
#include <stdbool.h>
#include "core/returntypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief SPI configuration structure
 */
typedef struct {
    uint32_t frequency;         /**< SPI frequency in Hz */
    uint8_t mode;              /**< SPI mode (0-3) */
    uint8_t bits_per_word;     /**< Bits per word (8, 16, 32) */
    bool msb_first;            /**< MSB first transmission */
} spi_config_t;

/**
 * @brief SPI handle structure
 */
typedef struct {
    uint8_t spi_id;            /**< SPI peripheral ID */
    spi_config_t config;       /**< SPI configuration */
    bool initialized;          /**< Initialization status */
} spi_handle_t;

/**
 * @brief Initialize SPI peripheral
 * 
 * @param handle Pointer to SPI handle
 * @param config Pointer to SPI configuration
 * @return retval_t SPP_OK on success, error code otherwise
 */
retval_t SPI_Init(spi_handle_t* handle, const spi_config_t* config);

/**
 * @brief Deinitialize SPI peripheral
 * 
 * @param handle Pointer to SPI handle
 * @return retval_t SPP_OK on success, error code otherwise
 */
retval_t SPI_Deinit(spi_handle_t* handle);

/**
 * @brief Transmit data over SPI
 * 
 * @param handle Pointer to SPI handle
 * @param tx_data Pointer to transmission data
 * @param length Number of bytes to transmit
 * @param timeout_ms Timeout in milliseconds
 * @return retval_t SPP_OK on success, error code otherwise
 */
retval_t SPI_Transmit(spi_handle_t* handle, const uint8_t* tx_data, 
                      uint16_t length, uint32_t timeout_ms);

/**
 * @brief Receive data over SPI
 * 
 * @param handle Pointer to SPI handle
 * @param rx_data Pointer to reception buffer
 * @param length Number of bytes to receive
 * @param timeout_ms Timeout in milliseconds
 * @return retval_t SPP_OK on success, error code otherwise
 */
retval_t SPI_Receive(spi_handle_t* handle, uint8_t* rx_data, 
                     uint16_t length, uint32_t timeout_ms);

/**
 * @brief Transmit and receive data over SPI
 * 
 * @param handle Pointer to SPI handle
 * @param tx_data Pointer to transmission data
 * @param rx_data Pointer to reception buffer
 * @param length Number of bytes to transfer
 * @param timeout_ms Timeout in milliseconds
 * @return retval_t SPP_OK on success, error code otherwise
 */
retval_t SPI_TransmitReceive(spi_handle_t* handle, const uint8_t* tx_data, 
                             uint8_t* rx_data, uint16_t length, uint32_t timeout_ms);

/**
 * @brief Check if SPI is busy
 * 
 * @param handle Pointer to SPI handle
 * @return bool true if busy, false if ready
 */
bool SPI_IsBusy(spi_handle_t* handle);

/**
 * @brief Set SPI chip select
 * 
 * @param handle Pointer to SPI handle
 * @param cs_pin Chip select pin number
 * @param active Active state (true = active, false = inactive)
 * @return retval_t SPP_OK on success, error code otherwise
 */
retval_t SPI_SetChipSelect(spi_handle_t* handle, uint8_t cs_pin, bool active);

/**
 * @brief Get SPI data (simplified interface)
 * 
 * @param spi_id SPI peripheral ID
 * @param data Pointer to data buffer
 * @param length Number of bytes to read
 * @return retval_t SPP_OK on success, error code otherwise
 */
retval_t spi_get_data(uint8_t spi_id, uint8_t* data, uint16_t length);

/**
 * @brief Write SPI data (simplified interface)
 * 
 * @param spi_id SPI peripheral ID
 * @param data Pointer to data to write
 * @param length Number of bytes to write
 * @return retval_t SPP_OK on success, error code otherwise
 */
retval_t spi_write_data(uint8_t spi_id, const uint8_t* data, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* SPP_HAL_SPI_H */ 