/**
 * @file esp32_spi.h
 * @brief ESP32-S3 SPI Implementation for SPP HAL
 * @version 1.0.0
 * @date 2024
 * 
 * This file provides ESP32-S3 specific SPI implementation for the SPP HAL layer.
 */

#ifndef SPP_ESP32_SPI_H
#define SPP_ESP32_SPI_H

#include "hal/spi/spi.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ESP32-S3 SPI configuration structure
 */
typedef struct {
    int spi_host;           ///< SPI host (SPI2_HOST, SPI3_HOST)
    int mosi_pin;           ///< MOSI pin number
    int miso_pin;           ///< MISO pin number
    int sclk_pin;           ///< SCLK pin number
    int cs_pin;             ///< CS pin number
    uint32_t clock_speed;   ///< Clock speed in Hz
    uint8_t mode;           ///< SPI mode (0-3)
} esp32_spi_config_t;

/**
 * @brief Initialize ESP32-S3 SPI with specific configuration
 * 
 * @param config ESP32 SPI configuration
 * @return retval_t SPP_OK on success, error code otherwise
 */
retval_t ESP32_SPI_Init(const esp32_spi_config_t* config);

/**
 * @brief Deinitialize ESP32-S3 SPI
 * 
 * @return retval_t SPP_OK on success, error code otherwise
 */
retval_t ESP32_SPI_Deinit(void);

/**
 * @brief Get default ESP32-S3 SPI configuration
 * 
 * @param config Pointer to configuration structure to fill
 */
void ESP32_SPI_GetDefaultConfig(esp32_spi_config_t* config);

#ifdef __cplusplus
}
#endif

#endif /* SPP_ESP32_SPI_H */ 