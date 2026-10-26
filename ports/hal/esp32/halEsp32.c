/**
 * @file halEsp32.c
 * @brief ESP32-S3 HAL port assembly.
 *
 * The per-driver implementations live in spi/, gpio/, storage/, time/ and
 * uart/. This file only builds the SPP_HalPort_t descriptor consumed by the
 * SPP core.
 */

/* ----------------------------------------------------------------
 * INCLUDES
 * ---------------------------------------------------------------- */
#include "spp/hal/hal.h"
#include "halEsp32.h"

#include "spi/spiPort.h"
#include "gpio/gpioPort.h"
#include "storage/storagePort.h"
#include "time/timePort.h"
#include "uart/uartPort.h"

/* ----------------------------------------------------------------
 * PORT DESCRIPTORS
 * ---------------------------------------------------------------- */
static const SPP_HALSpi_t s_esp32HalSpi = {
    .spiBusInit = SPP_PORTS_HAL_ESP32_spiBusInit,
    .spiGetHandle = SPP_PORTS_HAL_ESP32_spiGetHandle,
    .spiDeviceInit = SPP_PORTS_HAL_ESP32_spiDeviceInit,
    .spiTransmit = SPP_PORTS_HAL_ESP32_spiTransmit,
    .spiDeviceSetSpeed = SPP_PORTS_HAL_ESP32_spiDeviceSetSpeed,
    .spiTransmitRaw = SPP_PORTS_HAL_ESP32_spiTransmitRaw,
};

static const SPP_HALGpio_t s_esp32HalGpio = {
    .gpioConfigInterrupt = SPP_PORTS_HAL_ESP32_gpioConfigInterrupt,
    .gpioRegisterIsr = SPP_PORTS_HAL_ESP32_gpioRegisterIsr,
    .gpioRead = SPP_PORTS_HAL_ESP32_gpioRead,
    .gpioWrite = SPP_PORTS_HAL_ESP32_gpioWrite,
};

static const SPP_HALStorage_t s_esp32HalStorage = {
    .storageInit = SPP_PORTS_HAL_ESP32_storageInit,
    .storageWrite = SPP_PORTS_HAL_ESP32_storageWrite,
};

static const SPP_HALTime_t s_esp32HalTime = {
    .getTimeMs = SPP_PORTS_HAL_ESP32_getTimeMs,
    .getTimeUs = SPP_PORTS_HAL_ESP32_getTimeUs,
    .delayMs = SPP_PORTS_HAL_ESP32_delayMs,
};

static const SPP_HALUart_t s_esp32HalUart = {
    .uartPortInit = SPP_PORTS_HAL_ESP32_uartPortInit,
    .uartTransmit = SPP_PORTS_HAL_ESP32_uartTransmit,
    .uartRead = SPP_PORTS_HAL_ESP32_uartRead,
};

static const SPP_HalPort_t s_esp32HalPorts = {
    .spi = s_esp32HalSpi,
    .gpio = s_esp32HalGpio,
    .storage = s_esp32HalStorage,
    .time = s_esp32HalTime,
    .uart = s_esp32HalUart,
};

/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */

/**
 * @brief  Returns a pointer to the ESP32-S3 HAL port descriptor.
 * @return Pointer to the static @ref SPP_HalPort_t.
 */
const SPP_HalPort_t *SPP_PORTS_ESP32S3_getHalPorts(void)
{
    return &s_esp32HalPorts;
}
