/**
 * @file esp32_spi.c
 * @brief ESP32-S3 SPI Implementation for SPP HAL
 * @version 1.0.0
 * @date 2024
 * 
 * This file provides ESP32-S3 specific SPI implementation for the SPP HAL layer.
 */

#include "esp32_spi.h"
#include <string.h>

#ifdef ESP_PLATFORM
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#endif

static const char* TAG = "ESP32_SPI";
static spi_device_handle_t spi_device = NULL;
static esp32_spi_config_t current_config;
static bool is_initialized = false;

/**
 * @brief Get default ESP32-S3 SPI configuration
 */
void ESP32_SPI_GetDefaultConfig(esp32_spi_config_t* config)
{
    if (config == NULL) {
        return;
    }
    
    config->spi_host = SPI2_HOST;
    config->mosi_pin = 23;
    config->miso_pin = 19;
    config->sclk_pin = 18;
    config->cs_pin = 5;
    config->clock_speed = 1000000; // 1 MHz
    config->mode = 0;
}

/**
 * @brief Initialize ESP32-S3 SPI with specific configuration
 */
SppRetVal_t ESP32_SPI_Init(const esp32_spi_config_t* config)
{
    if (config == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
    if (is_initialized) {
        return SPP_ERROR_ALREADY_INITIALIZED;
    }
    
#ifdef ESP_PLATFORM
    esp_err_t ret;
    
    // Configure SPI bus
    spi_bus_config_t buscfg = {
        .mosi_io_num = config->mosi_pin,
        .miso_io_num = config->miso_pin,
        .sclk_io_num = config->sclk_pin,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096
    };
    
    // Configure SPI device
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = config->clock_speed,
        .mode = config->mode,
        .spics_io_num = config->cs_pin,
        .queue_size = 7,
        .flags = 0,
        .pre_cb = NULL,
        .post_cb = NULL
    };
    
    // Initialize SPI bus
    ret = spi_bus_initialize(config->spi_host, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(ret));
        return SPP_ERROR_HARDWARE_FAILURE;
    }
    
    // Add device to SPI bus
    ret = spi_bus_add_device(config->spi_host, &devcfg, &spi_device);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add SPI device: %s", esp_err_to_name(ret));
        spi_bus_free(config->spi_host);
        return SPP_ERROR_HARDWARE_FAILURE;
    }
    
    ESP_LOGI(TAG, "ESP32 SPI initialized successfully");
#endif
    
    // Store configuration
    memcpy(&current_config, config, sizeof(esp32_spi_config_t));
    is_initialized = true;
    
    return SPP_OK;
}

/**
 * @brief Deinitialize ESP32-S3 SPI
 */
SppRetVal_t ESP32_SPI_Deinit(void)
{
    if (!is_initialized) {
        return SPP_ERROR_NOT_INITIALIZED;
    }
    
#ifdef ESP_PLATFORM
    if (spi_device != NULL) {
        spi_bus_remove_device(spi_device);
        spi_device = NULL;
    }
    
    spi_bus_free(current_config.spi_host);
    ESP_LOGI(TAG, "ESP32 SPI deinitialized");
#endif
    
    is_initialized = false;
    return SPP_OK;
}

/**
 * @brief ESP32-S3 SPI Initialization (HAL Override)
 */
SppRetVal_t SPI_Init(void)
{
    esp32_spi_config_t config;
    ESP32_SPI_GetDefaultConfig(&config);
    return ESP32_SPI_Init(&config);
}

/**
 * @brief ESP32-S3 SPI Deinitialization (HAL Override)
 */
SppRetVal_t SPI_Deinit(void)
{
    return ESP32_SPI_Deinit();
}

/**
 * @brief ESP32-S3 SPI Transmit (HAL Override)
 */
SppRetVal_t SPI_Transmit(const uint8_t* data, uint16_t size, uint32_t timeout_ms)
{
    if (data == NULL || size == 0) {
        return SPP_ERROR_INVALID_PARAMETER;
    }
    
    if (!is_initialized || spi_device == NULL) {
        return SPP_ERROR_NOT_INITIALIZED;
    }
    
#ifdef ESP_PLATFORM
    spi_transaction_t trans = {
        .length = size * 8,  // Length in bits
        .tx_buffer = data,
        .rx_buffer = NULL
    };
    
    esp_err_t ret = spi_device_transmit(spi_device, &trans);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI transmit failed: %s", esp_err_to_name(ret));
        return SPP_ERROR_HARDWARE_FAILURE;
    }
#else
    // Simulation for non-ESP platforms
    (void)timeout_ms;
#endif
    
    return SPP_OK;
}

/**
 * @brief ESP32-S3 SPI Receive (HAL Override)
 */
SppRetVal_t SPI_Receive(uint8_t* data, uint16_t size, uint32_t timeout_ms)
{
    if (data == NULL || size == 0) {
        return SPP_ERROR_INVALID_PARAMETER;
    }
    
    if (!is_initialized || spi_device == NULL) {
        return SPP_ERROR_NOT_INITIALIZED;
    }
    
#ifdef ESP_PLATFORM
    spi_transaction_t trans = {
        .length = size * 8,  // Length in bits
        .tx_buffer = NULL,
        .rx_buffer = data
    };
    
    esp_err_t ret = spi_device_transmit(spi_device, &trans);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI receive failed: %s", esp_err_to_name(ret));
        return SPP_ERROR_HARDWARE_FAILURE;
    }
#else
    // Simulation for non-ESP platforms
    memset(data, 0xAA, size);
    (void)timeout_ms;
#endif
    
    return SPP_OK;
}

/**
 * @brief ESP32-S3 SPI Transmit/Receive (HAL Override)
 */
SppRetVal_t SPI_TransmitReceive(const uint8_t* tx_data, uint8_t* rx_data, uint16_t size, uint32_t timeout_ms)
{
    if (tx_data == NULL || rx_data == NULL || size == 0) {
        return SPP_ERROR_INVALID_PARAMETER;
    }
    
    if (!is_initialized || spi_device == NULL) {
        return SPP_ERROR_NOT_INITIALIZED;
    }
    
#ifdef ESP_PLATFORM
    spi_transaction_t trans = {
        .length = size * 8,  // Length in bits
        .tx_buffer = tx_data,
        .rx_buffer = rx_data
    };
    
    esp_err_t ret = spi_device_transmit(spi_device, &trans);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI transmit/receive failed: %s", esp_err_to_name(ret));
        return SPP_ERROR_HARDWARE_FAILURE;
    }
#else
    // Simulation for non-ESP platforms
    memcpy(rx_data, tx_data, size);
    (void)timeout_ms;
#endif
    
    return SPP_OK;
}

/**
 * @brief ESP32-S3 SPI Busy Check (HAL Override)
 */
bool SPI_IsBusy(void)
{
    // ESP32 SPI driver handles this internally
    return false;
}

/**
 * @brief ESP32-S3 SPI Chip Select Control (HAL Override)
 */
SppRetVal_t SPI_SetChipSelect(bool active)
{
    if (!is_initialized) {
        return SPP_ERROR_NOT_INITIALIZED;
    }
    
#ifdef ESP_PLATFORM
    // ESP32 SPI driver handles CS automatically
    // This is for manual CS control if needed
    gpio_set_level(current_config.cs_pin, active ? 0 : 1);
#else
    (void)active;
#endif
    
    return SPP_OK;
}

/**
 * @brief ESP32-S3 SPI Simple Data Read (HAL Override)
 */
SppRetVal_t spi_get_data(uint8_t* data, uint16_t size)
{
    return SPI_Receive(data, size, 1000); // 1 second timeout
}

/**
 * @brief ESP32-S3 SPI Simple Data Write (HAL Override)
 */
SppRetVal_t spi_write_data(const uint8_t* data, uint16_t size)
{
    return SPI_Transmit(data, size, 1000); // 1 second timeout
} 