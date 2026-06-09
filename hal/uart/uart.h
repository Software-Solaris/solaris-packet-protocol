/**
 * @file uart.h
 * @brief SPP UART HAL API — dispatches through the registered HAL port.
 */

#include "spp/core/types.h"
#include "spp/core/returnTypes.h"

/* ----------------------------------------------------------------
 * UART configuration types
 * ---------------------------------------------------------------- */

/**
 * @brief UART bus and device initialisation configuration.
 *
 * Passed to the HAL port's @c spiDeviceInit callback to describe one
 * physical SPI device.
 */

typedef struct
{
    spp_uint8_t portId;
    spp_uint32_t baudRate;

    spp_int32_t txPin;
    spp_int32_t rxPin;

    spp_int32_t rtsPin;
    spp_int32_t ctsPin;

    spp_uint8_t dataBits;
    spp_uint8_t stopBits;
    spp_uint8_t parity;

    spp_uint32_t rxBufferSize;
    spp_uint32_t txBufferSize;

    spp_uint32_t queueSize;
    spp_uint32_t flags;

} SPP_UartInitCfg_t;

/* ----------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------- */

/**
  * @brief Initialise the UART port.
  * 
  * @return K_SPP_OK on success.
  */
