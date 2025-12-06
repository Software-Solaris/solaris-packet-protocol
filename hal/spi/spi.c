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
 * @brief Default (weak) SPI bus configuration
 */
__attribute__((weak)) retval_t SPP_HAL_SPI_BusInit() {   
    return SPP_ERROR;
}


/**
 * @brief Default (weak) SPI handler asssigner (void pointers)
 */
__attribute__((weak)) void* SPP_HAL_SPI_GetHandler() {
    return NULL; 
}

/**
 * @brief Default (weak) SPI device initialization
 */
__attribute__((weak)) retval_t SPP_HAL_SPI_DeviceInit(void* handler) {
    return SPP_ERROR; 
}

/**
 * @brief Default (weak) SPI transmit function
 */
__attribute__((weak)) retval_t SPP_HAL_SPI_Transmit(void* handler, spp_uint8_t* data, spp_uint8_t length) {    
    return SPP_ERROR;
}

