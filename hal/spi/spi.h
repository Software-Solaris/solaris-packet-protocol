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
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize SPI bus configuration
 * 
 * @return retval_t SPP_OK on success, error code otherwise
 */
retval_t SPP_HAL_SPI_BusInit(void);


/**
 * @brief Sets a handler for every pheripheral ([0] reserved for BMP and [1] reserved for ICM)
 * 
 * @return a void pointer to an device_id array position, NULL otherwise
 */
void* SPP_HAL_SPI_GetHandler(void);


/**
 * @brief Initialize SPI peripheral and defines transaction details
 * 
 * @param p_handler Void pointer to pheripheral device (device_id[0] on BMP or device_id[1] on ICM)
 * @return retval_t SPP_OK on success, error code otherwise
 */
retval_t SPP_HAL_SPI_DeviceInit(void* p_handler);

/**
 * @brief Transmit data over SPI
 * 
 * @param handle Pointer to SPI handle
 * @param data_to_send Pointer to the data to transmit
 * @param data_to_recieve Pointer to where the data recieved will be stored
 * @param length Size of the buffer to transmit data (recieve buffer should be half of the size)
 * @return retval_t SPP_OK on success, error code otherwise
 */
retval_t SPP_HAL_SPI_Transmit(void* handler, void* data_to_send, void* data_to_recieve, spp_uint8_t length);


#ifdef __cplusplus
}
#endif

#endif /* SPP_HAL_SPI_H */ 