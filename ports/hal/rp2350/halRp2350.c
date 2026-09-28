/**
 * @file halRp2350.c
 * @brief RP2350 HAL port for SPP — baremetal Pico SDK implementation.
 *
 * Register @ref g_rp2350HalPort before calling @ref SPP_CORE_init().
 */

/* ----------------------------------------------------------------
 * INCLUDES
 * ---------------------------------------------------------------- */
#include "halRp2350.h"
#include "macrosRp2350.h"

#include "spp/hal/hal.h"
#include "spp/hal/gpio/gpio.h"
#include "spp/hal/uart/uart.h"
#include "spp/core/returnTypes.h"
#include "spp/core/types.h"

#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "hardware/timer.h"

#include <string.h>

/* ----------------------------------------------------------------
 * TYPES
 * ---------------------------------------------------------------- */

/**
 * @brief Descriptor for an SPI device slot on RP2350.
 */
typedef struct
{
    spp_uint32_t csPin;
    spp_uint32_t baudRate;
    spp_bool_t   isInitialized;
} RP2350_SpiDevice_t;

/* ----------------------------------------------------------------
 * STATIC FUNCTIONS DECLARATIONS
 * ---------------------------------------------------------------- */

/* SPI */
static SPP_RetVal_t SPP_PORTS_HAL_RP2350_spiBusInit(void);
static void *SPP_PORTS_HAL_RP2350_spiGetHandle(spp_uint8_t deviceIdx);
static SPP_RetVal_t SPP_PORTS_HAL_RP2350_spiDeviceInit(void *p_handle);
static SPP_RetVal_t SPP_PORTS_HAL_RP2350_spiTransmit(void *p_handle, spp_uint8_t *p_data, spp_uint8_t length);
static SPP_RetVal_t SPP_PORTS_HAL_RP2350_spiDeviceSetSpeed(void *p_handle, spp_uint32_t speedHz);

/* GPIO */
static SPP_RetVal_t SPP_PORTS_HAL_RP2350_gpioConfigInterrupt(spp_uint32_t pin, spp_uint32_t intrType, spp_uint32_t pull);
static SPP_RetVal_t SPP_PORTS_HAL_RP2350_gpioRegisterIsr(spp_uint32_t pin, void *p_isrCtx);
static void SPP_PORTS_HAL_RP2350_gpioSharedCallback(uint gpio, uint32_t events);

/* Storage */
static SPP_RetVal_t SPP_PORTS_HAL_RP2350_storageInit(void);
static SPP_RetVal_t SPP_PORTS_HAL_RP2350_storageWrite(const void *p_buffer, spp_uint32_t first_block, spp_uint16_t count);

/* Time */
static spp_uint32_t SPP_PORTS_HAL_RP2350_getTimeMs(void);
static spp_uint32_t SPP_PORTS_HAL_RP2350_getTimeUs(void);
static void SPP_PORTS_HAL_RP2350_delayMs(spp_uint32_t ms);

/* UART */
static SPP_RetVal_t SPP_PORTS_HAL_RP2350_uartPortInit(void);
static SPP_RetVal_t SPP_PORTS_HAL_RP2350_uartTransmit(const void *p_data, spp_uint32_t len);
static SPP_RetVal_t SPP_PORTS_HAL_RP2350_uartRead(void *p_data, spp_uint32_t len, spp_uint32_t *p_readBytes);

/* ----------------------------------------------------------------
 * STATIC VARIABLES
 * ---------------------------------------------------------------- */

static spp_bool_t s_busInitialized = false;
static spp_bool_t s_storageInitialized = false;
static spp_bool_t s_uartInitialized = false;

/* SPI Device table */
static RP2350_SpiDevice_t s_spiDevices[K_RP2350_MAX_SPI_DEVICES] = {
    [K_RP2350_SPI_IDX_BMP]  = {.csPin = K_RP2350_PIN_CS_BMP,  .baudRate = 500000U,             .isInitialized = false},
    [K_RP2350_SPI_IDX_ICM]  = {.csPin = K_RP2350_PIN_CS_ICM,  .baudRate = 1000000U,            .isInitialized = false},
    [K_RP2350_SPI_IDX_SDC]  = {.csPin = K_RP2350_PIN_CS_SDC,  .baudRate = K_RP2350_SPI_INIT_BAUD, .isInitialized = false},
    [K_RP2350_SPI_IDX_LORA] = {.csPin = K_RP2350_PIN_CS_LORA, .baudRate = 2000000U,            .isInitialized = false},
};

/* GPIO ISR Context table (RP2350B supports up to 48 GPIOs) */
#define K_RP2350_MAX_GPIOS (48U)
static SPP_GpioIsrCtx_t *s_gpioIsrContexts[K_RP2350_MAX_GPIOS] = {NULL};

/* HAL Port Sub-structures */
const static SPP_HALSpi_t s_rp2350HalSpi = {
    .spiBusInit        = SPP_PORTS_HAL_RP2350_spiBusInit,
    .spiGetHandle      = SPP_PORTS_HAL_RP2350_spiGetHandle,
    .spiDeviceInit     = SPP_PORTS_HAL_RP2350_spiDeviceInit,
    .spiTransmit       = SPP_PORTS_HAL_RP2350_spiTransmit,
    .spiDeviceSetSpeed = SPP_PORTS_HAL_RP2350_spiDeviceSetSpeed,
};

const static SPP_HALGpio_t s_rp2350HalGpio = {
    .gpioConfigInterrupt = SPP_PORTS_HAL_RP2350_gpioConfigInterrupt,
    .gpioRegisterIsr     = SPP_PORTS_HAL_RP2350_gpioRegisterIsr,
};

const static SPP_HALStorage_t s_rp2350HalStorage = {
    .storageInit  = SPP_PORTS_HAL_RP2350_storageInit,
    .storageWrite = SPP_PORTS_HAL_RP2350_storageWrite,
};

const static SPP_HALTime_t s_rp2350HalTime = {
    .getTimeMs = SPP_PORTS_HAL_RP2350_getTimeMs,
    .getTimeUs = SPP_PORTS_HAL_RP2350_getTimeUs,
    .delayMs   = SPP_PORTS_HAL_RP2350_delayMs,
};

const static SPP_HALUart_t s_rp2350HalUart = {
    .uartPortInit = SPP_PORTS_HAL_RP2350_uartPortInit,
    .uartTransmit = SPP_PORTS_HAL_RP2350_uartTransmit,
    .uartRead     = SPP_PORTS_HAL_RP2350_uartRead,
};

static const SPP_HalPort_t s_rp2350HalPorts = {
    .spi     = s_rp2350HalSpi,
    .gpio    = s_rp2350HalGpio,
    .storage = s_rp2350HalStorage,
    .time    = s_rp2350HalTime,
    .uart    = s_rp2350HalUart,
};

/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */

const SPP_HalPort_t *SPP_PORTS_RP2350_getHalPorts(void)
{
    return &s_rp2350HalPorts;
}

/* ----------------------------------------------------------------
 * SPI IMPLEMENTATION
 * ---------------------------------------------------------------- */

static SPP_RetVal_t SPP_PORTS_HAL_RP2350_spiBusInit(void)
{
    if (s_busInitialized)
    {
        return K_SPP_OK;
    }

    /* Initialize SPI0 at default baudrate */
    (void)spi_init(K_RP2350_SPI_INST, K_RP2350_SPI_DEFAULT_BAUD);

    /* Configure GPIO functions for SPI */
    gpio_set_function(K_RP2350_PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(K_RP2350_PIN_CLK,  GPIO_FUNC_SPI);
    gpio_set_function(K_RP2350_PIN_MOSI, GPIO_FUNC_SPI);

    s_busInitialized = true;
    return K_SPP_OK;
}

static void *SPP_PORTS_HAL_RP2350_spiGetHandle(spp_uint8_t deviceIdx)
{
    if (deviceIdx >= K_RP2350_MAX_SPI_DEVICES)
    {
        return NULL;
    }
    return (void *)&s_spiDevices[deviceIdx];
}

static SPP_RetVal_t SPP_PORTS_HAL_RP2350_spiDeviceInit(void *p_handle)
{
    if (p_handle == NULL)
    {
        return K_SPP_ERROR_NULL_POINTER;
    }

    RP2350_SpiDevice_t *p_dev = (RP2350_SpiDevice_t *)p_handle;

    /* Initialize CS pin as GPIO output, set HIGH (deselected) */
    gpio_init(p_dev->csPin);
    gpio_set_dir(p_dev->csPin, GPIO_OUT);
    gpio_put(p_dev->csPin, 1);

    p_dev->isInitialized = true;
    return K_SPP_OK;
}

static SPP_RetVal_t SPP_PORTS_HAL_RP2350_spiTransmit(void *p_handle, spp_uint8_t *p_data, spp_uint8_t length)
{
    if ((p_handle == NULL) || (p_data == NULL) || (length == 0U))
    {
        return K_SPP_ERROR_NULL_POINTER;
    }

    RP2350_SpiDevice_t *p_dev = (RP2350_SpiDevice_t *)p_handle;
    if (!p_dev->isInitialized)
    {
        return K_SPP_ERROR_NOT_INITIALIZED;
    }

    /* Set device specific baudrate */
    (void)spi_set_baudrate(K_RP2350_SPI_INST, p_dev->baudRate);

    spp_uint8_t i = 0U;

    /*
     * SPP register transactions:
     * - Read: bit 7 set -> 3 bytes (cmd, dummy, rx)
     * - Write: bit 7 clear -> 2 bytes (cmd, tx)
     */
    while (i < length)
    {
        spp_uint8_t chunk = ((p_data[i] & 0x80U) != 0U) ? 3U : 2U;
        if ((i + chunk) > length)
        {
            chunk = length - i;
        }

        gpio_put(p_dev->csPin, 0);
        int written = spi_write_read_blocking(K_RP2350_SPI_INST, &p_data[i], &p_data[i], (size_t)chunk);
        gpio_put(p_dev->csPin, 1);

        if (written != (int)chunk)
        {
            return K_SPP_ERROR_ON_SPI_TRANSACTION;
        }

        i += chunk;
    }

    return K_SPP_OK;
}

static SPP_RetVal_t SPP_PORTS_HAL_RP2350_spiDeviceSetSpeed(void *p_handle, spp_uint32_t speedHz)
{
    if (p_handle == NULL)
    {
        return K_SPP_ERROR_NULL_POINTER;
    }

    RP2350_SpiDevice_t *p_dev = (RP2350_SpiDevice_t *)p_handle;
    p_dev->baudRate = speedHz;

    return K_SPP_OK;
}

/* ----------------------------------------------------------------
 * GPIO IMPLEMENTATION
 * ---------------------------------------------------------------- */

static void SPP_PORTS_HAL_RP2350_gpioSharedCallback(uint gpio, uint32_t events)
{
    (void)events;
    if ((gpio < K_RP2350_MAX_GPIOS) && (s_gpioIsrContexts[gpio] != NULL))
    {
        if (s_gpioIsrContexts[gpio]->p_flag != NULL)
        {
            *s_gpioIsrContexts[gpio]->p_flag = true;
        }
    }
}

static SPP_RetVal_t SPP_PORTS_HAL_RP2350_gpioConfigInterrupt(spp_uint32_t pin, spp_uint32_t intrType, spp_uint32_t pull)
{
    if (pin >= K_RP2350_MAX_GPIOS)
    {
        return K_SPP_ERROR_INVALID_PARAMETER;
    }

    (void)intrType;
    gpio_init((uint)pin);
    gpio_set_dir((uint)pin, GPIO_IN);

    if (pull == 1U)
    {
        gpio_pull_up((uint)pin);
    }
    else if (pull == 2U)
    {
        gpio_pull_down((uint)pin);
    }
    else
    {
        gpio_disable_pulls((uint)pin);
    }

    return K_SPP_OK;
}

static SPP_RetVal_t SPP_PORTS_HAL_RP2350_gpioRegisterIsr(spp_uint32_t pin, void *p_isrCtx)
{
    if ((pin >= K_RP2350_MAX_GPIOS) || (p_isrCtx == NULL))
    {
        return K_SPP_ERROR_INVALID_PARAMETER;
    }

    s_gpioIsrContexts[pin] = (SPP_GpioIsrCtx_t *)p_isrCtx;

    /* Enable rising edge interrupt with the shared callback */
    gpio_set_irq_enabled_with_callback((uint)pin, GPIO_IRQ_EDGE_RISE, true, &SPP_PORTS_HAL_RP2350_gpioSharedCallback);

    return K_SPP_OK;
}

/* ----------------------------------------------------------------
 * STORAGE IMPLEMENTATION
 * ---------------------------------------------------------------- */

static SPP_RetVal_t SPP_PORTS_HAL_RP2350_storageInit(void)
{
    /* Storage initialization hook for SD card raw SPI block mode */
    s_storageInitialized = true;
    return K_SPP_OK;
}

static SPP_RetVal_t SPP_PORTS_HAL_RP2350_storageWrite(const void *p_buffer, spp_uint32_t first_block, spp_uint16_t count)
{
    if (p_buffer == NULL)
    {
        return K_SPP_ERROR_NULL_POINTER;
    }
    if (!s_storageInitialized || (count == 0U))
    {
        return K_SPP_ERROR;
    }

    (void)first_block;
    (void)count;

    return K_SPP_OK;
}

/* ----------------------------------------------------------------
 * TIME IMPLEMENTATION
 * ---------------------------------------------------------------- */

static spp_uint32_t SPP_PORTS_HAL_RP2350_getTimeMs(void)
{
    return (spp_uint32_t)to_ms_since_boot(get_absolute_time());
}

static spp_uint32_t SPP_PORTS_HAL_RP2350_getTimeUs(void)
{
    return (spp_uint32_t)time_us_32();
}

static void SPP_PORTS_HAL_RP2350_delayMs(spp_uint32_t ms)
{
    busy_wait_ms(ms);
}

/* ----------------------------------------------------------------
 * UART IMPLEMENTATION
 * ---------------------------------------------------------------- */

static SPP_RetVal_t SPP_PORTS_HAL_RP2350_uartPortInit(void)
{
    if (s_uartInitialized)
    {
        return K_SPP_OK;
    }

    (void)uart_init(K_RP2350_UART_INST, K_RP2350_UART_BAUD_RATE);

    gpio_set_function(K_RP2350_UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(K_RP2350_UART_RX_PIN, GPIO_FUNC_UART);

    uart_set_hw_flow(K_RP2350_UART_INST, false, false);
    uart_set_format(K_RP2350_UART_INST, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(K_RP2350_UART_INST, true);

    s_uartInitialized = true;
    return K_SPP_OK;
}

static SPP_RetVal_t SPP_PORTS_HAL_RP2350_uartTransmit(const void *p_data, spp_uint32_t len)
{
    if ((p_data == NULL) || (len == 0U))
    {
        return K_SPP_ERROR_NULL_POINTER;
    }

    const uint8_t *p_bytes = (const uint8_t *)p_data;
    uart_write_blocking(K_RP2350_UART_INST, p_bytes, (size_t)len);

    return K_SPP_OK;
}

static SPP_RetVal_t SPP_PORTS_HAL_RP2350_uartRead(void *p_data, spp_uint32_t len, spp_uint32_t *p_readBytes)
{
    if ((p_data == NULL) || (len == 0U) || (p_readBytes == NULL))
    {
        return K_SPP_ERROR_NULL_POINTER;
    }

    uint8_t *p_buf = (uint8_t *)p_data;
    spp_uint32_t count = 0U;

    while ((count < len) && uart_is_readable(K_RP2350_UART_INST))
    {
        p_buf[count] = uart_getc(K_RP2350_UART_INST);
        count++;
    }

    *p_readBytes = count;
    return K_SPP_OK;
}
