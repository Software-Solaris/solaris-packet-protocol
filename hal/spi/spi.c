/**
 * @file spi.c
 * @brief SPI Hardware Abstraction Layer Implementation
 * @version 1.0.0
 * @date 2024
 * 
 * This file provides the default (weak) implementation of SPI HAL functions
 * that can be overridden by platform-specific implementations.
 */

#include "spi.h"
#include "core/returntypes.h"
#include <string.h>

/**
 * @brief Default (weak) SPI initialization
 */
__attribute__((weak)) retval_t SPP_HAL_SPI_Init(spi_handle_t* handle, const spi_config_t* config)
{   
    return SPP_ERROR;
}

/**
 * @brief Default (weak) SPI deinitialization
 */
__attribute__((weak)) retval_t SPI_Deinit(spi_handle_t* handle)
{
    if (handle == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
    handle->initialized = false;
    return SPP_OK;
}

/**
 * @brief Default (weak) SPI transmit function
 */
__attribute__((weak)) retval_t SPP_HAL_SPI_Transmit(void* handler, void* data_to_send, void* data_to_recieve, spp_uint8_t length)

{    
    return SPP_ERROR;
}

/**
 * @brief Default (weak) SPI receive function
 */
__attribute__((weak)) retval_t SPP_SPI_Receive(spi_handle_t* handle, uint8_t* rx_data, 
                                          uint16_t length, uint32_t timeout_ms)
{
    if (handle == NULL || rx_data == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
    if (!handle->initialized) {
        return SPP_ERROR_NOT_INITIALIZED;
    }
    
    // Default implementation - fill with zeros
    memset(rx_data, 0, length);
    (void)timeout_ms;
    
    return SPP_OK;
}

/**
 * @brief Default (weak) SPI transmit and receive function
 */
__attribute__((weak)) retval_t SPI_TransmitReceive(spi_handle_t* handle, const uint8_t* tx_data, 
                                                  uint8_t* rx_data, uint16_t length, uint32_t timeout_ms)
{
    if (handle == NULL || tx_data == NULL || rx_data == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
    if (!handle->initialized) {
        return SPP_ERROR_NOT_INITIALIZED;
    }
    
    // Default implementation - echo tx_data to rx_data
    memcpy(rx_data, tx_data, length);
    (void)timeout_ms;
    
    return SPP_OK;
}

/**
 * @brief Default (weak) SPI busy check function
 */
__attribute__((weak)) bool SPI_IsBusy(spi_handle_t* handle)
{
    if (handle == NULL) {
        return false;
    }
    
    // Default implementation - never busy
    return false;
}

/**
 * @brief Default (weak) SPI chip select function
 */
__attribute__((weak)) retval_t SPI_SetChipSelect(spi_handle_t* handle, uint8_t cs_pin, bool active)
{
    if (handle == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
    if (!handle->initialized) {
        return SPP_ERROR_NOT_INITIALIZED;
    }
    
    // Default implementation - just return OK
    (void)cs_pin;
    (void)active;
    
    return SPP_OK;
}

/**
 * @brief Get SPI data (simplified interface)
 */
__attribute__((weak)) retval_t spi_get_data(uint8_t spi_id, uint8_t* data, uint16_t length)
{
    if (data == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
    // Default implementation - fill with pattern
    for (uint16_t i = 0; i < length; i++) {
        data[i] = (uint8_t)(spi_id + i);
    }
    
    return SPP_OK;
}

/**
 * @brief Write SPI data (simplified interface)
 */
__attribute__((weak)) retval_t spi_write_data(uint8_t spi_id, const uint8_t* data, uint16_t length)
{
    if (data == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
    // Default implementation - just return OK
    (void)spi_id;
    (void)length;
    
    return SPP_OK;
} 
