/**
 * @file spi.h
 * @brief SPP SPI HAL API — dispatches through the registered HAL port.
 */

#ifndef SPP_HAL_SPI_H
#define SPP_HAL_SPI_H

#include "spp/core/types.h"
#include "spp/core/returnTypes.h"

/* ----------------------------------------------------------------
 * SPI configuration types
 * ---------------------------------------------------------------- */

/**
 * @brief SPI clock phase/polarity mode.
 */
typedef enum
{
    K_SPP_SPI_MODE0 = 0U, /**< CPOL=0, CPHA=0. */
    K_SPP_SPI_MODE1 = 1U, /**< CPOL=0, CPHA=1. */
    K_SPP_SPI_MODE3 = 3U  /**< CPOL=1, CPHA=1. */
} SPP_SpiMode_t;

/**
 * @brief SPI duplex mode.
 */
typedef enum
{
    K_SPP_SPI_FULL_DUPLEX = 0U, /**< Simultaneous TX and RX. */
    K_SPP_SPI_HALF_DUPLEX = 1U  /**< Shared TX/RX line.      */
} SPP_SpiDuplex_t;

/**
 * @brief SPI bus and device initialisation configuration.
 *
 * Passed to the HAL port's @c spiDeviceInit callback to describe one
 * physical SPI device.
 */
typedef struct
{
    int busId;              /**< SPI bus / host identifier (platform-defined). */
    int pinMiso;            /**< MISO (CIPO) GPIO number.                       */
    int pinMosi;            /**< MOSI (COPI) GPIO number.                       */
    int pinSclk;            /**< Clock GPIO number.                             */
    int pinCs;              /**< Chip-select GPIO number.                       */
    unsigned int maxHz;     /**< Maximum SPI clock frequency in Hz.             */
    SPP_SpiMode_t mode;     /**< SPI clock mode.                                */
    SPP_SpiDuplex_t duplex; /**< Duplex mode.                                   */
    unsigned int queueSize; /**< Transaction queue depth.                       */
} SPP_SpiInitCfg_t;

/* ----------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------- */

/**
 * @brief Initialise the SPI bus.
 *
 * Safe to call multiple times; subsequent calls are no-ops.
 *
 * @return K_SPP_OK on success.
 */
SPP_RetVal_t SPP_HAL_SPI_busInit(void);

/**
 * @brief Obtain the SPI device handle for the given device index.
 *
 * @param[in] deviceIdx  Zero-based device index (0 = first device, etc.).
 *
 * @return Opaque handle on success, NULL if index is out of range.
 */
void *SPP_HAL_SPI_getHandle(spp_uint8_t deviceIdx);

/**
 * @brief Initialise a specific SPI device.
 *
 * @param[in] p_handle  Handle returned by @ref SPP_HAL_SPI_getHandle().
 *
 * @return K_SPP_OK on success, K_SPP_ERROR_NULL_POINTER if handle is NULL.
 */
SPP_RetVal_t SPP_HAL_SPI_deviceInit(void *p_handle);

/**
 * @brief Perform a full-duplex SPI transaction.
 *
 * @param[in,out] p_handle  SPI device handle.
 * @param[in,out] p_data    TX data in, RX data out (in-place).
 * @param[in]     length    Number of bytes in the transaction.
 *
 * @return K_SPP_OK on success, K_SPP_ERROR_ON_SPI_TRANSACTION on failure.
 */
SPP_RetVal_t SPP_HAL_SPI_transmit(void *p_handle, spp_uint8_t *p_data, spp_uint8_t length);

#endif /* SPP_HAL_SPI_H */
