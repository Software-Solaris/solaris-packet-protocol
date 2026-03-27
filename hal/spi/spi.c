/**
 * @file spi.c
 * @brief SPI Hardware Abstraction Layer — default (weak) implementation.
 * @version 1.0.0
 * @date 2024
 *
 * This file provides the default (weak) implementation of SPI HAL functions
 * that can be overridden by platform-specific implementations.
 */

/* ---------------------------------------------------------------- */
/*  Includes                                                        */
/* ---------------------------------------------------------------- */

#include "spp/hal/spi/spi.h"
#include "spp/core/returntypes.h"
#include <string.h>

/* ---------------------------------------------------------------- */
/*  Public functions (weak defaults)                                */
/* ---------------------------------------------------------------- */

/**
 * @brief  Default (weak) SPI bus initialisation.
 *
 * @return retval_t  Always returns SPP_ERROR.
 */
__attribute__((weak)) retval_t SPP_HAL_SPI_BusInit()
{
    return SPP_ERROR;
}

/**
 * @brief  Default (weak) SPI handler allocator.
 *
 * @return void*  Always returns NULL.
 */
__attribute__((weak)) void *SPP_HAL_SPI_GetHandler()
{
    return NULL;
}

/**
 * @brief  Default (weak) SPI device initialisation.
 *
 * @param[in] p_handler  Pointer to the peripheral device handle.
 *
 * @return retval_t  Always returns SPP_ERROR.
 */
__attribute__((weak)) retval_t SPP_HAL_SPI_DeviceInit(void *p_handler)
{
    return SPP_ERROR;
}

/**
 * @brief  Default (weak) SPI transmit function.
 *
 * @param[in]     p_handler  Pointer to the SPI device handle.
 * @param[in,out] p_data     Pointer to the TX/RX buffer.
 * @param[in]     length     Number of bytes to transmit.
 *
 * @return retval_t  Always returns SPP_ERROR.
 */
__attribute__((weak)) retval_t SPP_HAL_SPI_Transmit(void *p_handler, spp_uint8_t *p_data,
                                                    spp_uint8_t length)
{
    return SPP_ERROR;
}
