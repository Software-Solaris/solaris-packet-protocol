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

/* ---------------------------------------------------------------- */
/*  Includes                                                        */
/* ---------------------------------------------------------------- */

#include "spp/core/returntypes.h"
#include "spp/core/types.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /* ---------------------------------------------------------------- */
    /*  Public function declarations                                    */
    /* ---------------------------------------------------------------- */

    /**
     * @brief  Initialize the SPI bus configuration.
     *
     * @return retval_t  SPP_OK on success, error code otherwise.
     */
    retval_t SPP_HAL_SPI_BusInit(void);

    /**
     * @brief  Get a handler for the next available SPI peripheral.
     *
     * Returns a void pointer to a device_id array position.
     * Index 0 is reserved for BMP and index 1 for ICM.
     *
     * @return void*  Pointer to the device handle, NULL if none available.
     */
    void *SPP_HAL_SPI_GetHandler(void);

    /**
     * @brief  Initialize an SPI peripheral and define transaction details.
     *
     * @param[in] p_handler  Void pointer to the peripheral device
     *                       (device_id[0] for BMP or device_id[1] for ICM).
     *
     * @return retval_t  SPP_OK on success, error code otherwise.
     */
    retval_t SPP_HAL_SPI_DeviceInit(void *p_handler);

    /**
     * @brief  Transmit data over SPI (full-duplex).
     *
     * @param[in]     p_handler  Pointer to the SPI device handle.
     * @param[in,out] p_data     Pointer to the TX/RX buffer.
     * @param[in]     length     Number of bytes to transmit.
     *
     * @return retval_t  SPP_OK on success, error code otherwise.
     */
    retval_t SPP_HAL_SPI_Transmit(void *p_handler, spp_uint8_t *p_data, spp_uint8_t length);

#ifdef __cplusplus
}
#endif

#endif /* SPP_HAL_SPI_H */
