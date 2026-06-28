/**
 * @file halEsp32.c
 * @brief ESP32 HAL port for SPP — polling SPI, no FreeRTOS dependency.
 *
 * Register @ref g_esp32HalPort before calling @ref SPP_CORE_init().
 */

#include "spp/hal/hal.h"
#include "spp/hal/gpio/gpio.h"
#include "spp/hal/uart/uart.h"
#include "spp/core/returnTypes.h"
#include "spp/core/types.h"
#include "macrosEsp32.h"
#include "halEsp32.h"

#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/uart.h"

#include <string.h>

/* ----------------------------------------------------------------
 * STATIC FUNCTIONS DEFINITIONS
 * ---------------------------------------------------------------- */

static SPP_RetVal_t SPP_PORTS_HAL_ESP32_spiBusInit(void);
static void *SPP_PORTS_HAL_ESP32_spiGetHandle(spp_uint8_t deviceIdx);
static SPP_RetVal_t SPP_PORTS_HAL_ESP32_spiDeviceInit(void *p_handle);
static SPP_RetVal_t SPP_PORTS_HAL_ESP32_spiTransmit(void *p_handle, spp_uint8_t *p_data, spp_uint8_t length);
static SPP_RetVal_t SPP_PORTS_HAL_ESP32_spiDeviceSetSpeed(void *p_handle, spp_uint32_t speedHz);

static SPP_RetVal_t SPP_PORTS_HAL_ESP32_gpioConfigInterrupt(spp_uint32_t pin, spp_uint32_t intrType, spp_uint32_t pull);
static SPP_RetVal_t SPP_PORTS_HAL_ESP32_gpioRegisterIsr(spp_uint32_t pin, void *p_isrCtx);

static SPP_RetVal_t SPP_PORTS_HAL_ESP32_storageInit(void);
static SPP_RetVal_t SPP_PORTS_HAL_ESP32_storageWrite(const void *p_buffer, spp_uint32_t first_block,
                                                     spp_uint16_t count);

static spp_uint32_t SPP_PORTS_HAL_ESP32_getTimeMs(void);
static void SPP_PORTS_HAL_ESP32_delayMs(spp_uint32_t ms);

static SPP_RetVal_t SPP_PORTS_HAL_ESP32_uartPortInit(void);
static SPP_RetVal_t SPP_PORTS_HAL_ESP32_uartTransmit(const void *p_data, spp_uint32_t len);


/* ----------------------------------------------------------------
 * STATIC VARIABLES
 * ---------------------------------------------------------------- */

const static SPP_HALSpi_t s_esp32HalSpi = {
    .spiBusInit = SPP_PORTS_HAL_ESP32_spiBusInit,
    .spiGetHandle = SPP_PORTS_HAL_ESP32_spiGetHandle,
    .spiDeviceInit = SPP_PORTS_HAL_ESP32_spiDeviceInit,
    .spiTransmit = SPP_PORTS_HAL_ESP32_spiTransmit,
    .spiDeviceSetSpeed = SPP_PORTS_HAL_ESP32_spiDeviceSetSpeed,
};

const static SPP_HALGpio_t s_esp32HalGpio = {
    .gpioConfigInterrupt = SPP_PORTS_HAL_ESP32_gpioConfigInterrupt,
    .gpioRegisterIsr = SPP_PORTS_HAL_ESP32_gpioRegisterIsr,
};

const static SPP_HALStorage_t s_esp32HalStorage = {
    .storageInit = SPP_PORTS_HAL_ESP32_storageInit,
    .storageWrite = SPP_PORTS_HAL_ESP32_storageWrite,
};

const static SPP_HALTime_t s_esp32HalTime = {
    .getTimeMs = SPP_PORTS_HAL_ESP32_getTimeMs,
    .delayMs = SPP_PORTS_HAL_ESP32_delayMs,
};

const static SPP_HALUart_t s_esp32HalUart = {.uartPortInit = SPP_PORTS_HAL_ESP32_uartPortInit,
                                             .uartTransmit = SPP_PORTS_HAL_ESP32_uartTransmit};


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

/* ----------------------------------------------------------------
 * PRIVATE VARIABLES
 * ---------------------------------------------------------------- */

static const char *const k_tag = "SPP_HAL";

static spi_device_handle_t s_spiHandles[K_ESP32_MAX_SPI_DEVICES];
static spp_uint8_t s_spiDevCount = 0U;
static spp_bool_t s_busInitialized = false;

static sdspi_dev_handle_t sdHandle;
static sdmmc_card_t sdCard;
static spp_bool_t s_storageInitialized = false;

/* ----------------------------------------------------------------
 * SPI
 * ---------------------------------------------------------------- */

static SPP_RetVal_t SPP_PORTS_HAL_ESP32_spiBusInit(void)
{
    if (s_busInitialized)
    {
        return K_SPP_OK;
    }

    /* Diagnostic: test MISO pin level before SPI takes control of it */
    gpio_config_t misoCfg = {
        .pin_bit_mask = (1ULL << K_ESP32_PIN_MISO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    // gpio_config(&misoCfg);
    // int misoLevel = gpio_get_level((gpio_num_t)K_ESP32_PIN_MISO);
    // ESP_LOGI(k_tag, "MISO GPIO%d pre-SPI level = %d", K_ESP32_PIN_MISO, misoLevel);

    spi_bus_config_t busCfg = {
        .miso_io_num = K_ESP32_PIN_MISO,
        .mosi_io_num = K_ESP32_PIN_MOSI,
        .sclk_io_num = K_ESP32_PIN_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 0,
    };

    esp_err_t ret = spi_bus_initialize(K_ESP32_SPI_HOST, &busCfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK)
    {
        ESP_LOGE(k_tag, "SPI bus init failed: %s", esp_err_to_name(ret));
        return K_SPP_ERROR;
    }

    s_busInitialized = true;
    return K_SPP_OK;
}

static void *SPP_PORTS_HAL_ESP32_spiGetHandle(spp_uint8_t deviceIdx)
{
    if (deviceIdx >= K_ESP32_MAX_SPI_DEVICES)
    {
        return NULL;
    }
    return (void *)&s_spiHandles[deviceIdx];
}

static SPP_RetVal_t SPP_PORTS_HAL_ESP32_spiDeviceInit(void *p_handle)
{
    if (p_handle == NULL)
    {
        return K_SPP_ERROR_NULL_POINTER;
    }
    if (s_spiDevCount >= K_ESP32_MAX_SPI_DEVICES)
    {
        return K_SPP_ERROR;
    }

    spi_device_handle_t *p_h = (spi_device_handle_t *)p_handle;
    spi_device_interface_config_t devCfg = {0};

    if (s_spiDevCount == K_ESP32_SPI_IDX_ICM)
    {
        devCfg.clock_speed_hz = 1 * 1000 * 1000;
        devCfg.mode = 0;
        devCfg.spics_io_num = K_ESP32_PIN_CS_ICM;
        devCfg.queue_size = 1;
    }
    else if (s_spiDevCount == K_ESP32_SPI_IDX_BMP)
    {
        devCfg.clock_speed_hz = 500 * 1000;
        devCfg.mode = 3;
        devCfg.spics_io_num = K_ESP32_PIN_CS_BMP;
        devCfg.queue_size = 1;
    }
    else if (s_spiDevCount == K_ESP32_SPI_IDX_SDC)
    {
        devCfg.clock_speed_hz = 400 * 1000; /* 400 kHz — max allowed during SD init (spec §7.2.1) */
        devCfg.mode = 0;                    /* SPI Mode 0: CPOL=0, CPHA=0 (spec §7) */
        devCfg.spics_io_num = K_ESP32_PIN_CS_SDC;
        devCfg.queue_size = 1;
    }
    else
    {
        ESP_LOGE(k_tag, "Unexpected device index %u", s_spiDevCount);
        return K_SPP_ERROR;
    }

    esp_err_t ret = spi_bus_add_device(K_ESP32_SPI_HOST, &devCfg, p_h);
    if (ret != ESP_OK)
    {
        ESP_LOGE(k_tag, "spi_bus_add_device failed: %s", esp_err_to_name(ret));
        return K_SPP_ERROR;
    }

    s_spiDevCount++;
    return K_SPP_OK;
}

static SPP_RetVal_t SPP_PORTS_HAL_ESP32_spiTransmit(void *p_handle, spp_uint8_t *p_data, spp_uint8_t length)
{
    if ((p_handle == NULL) || (p_data == NULL) || (length == 0U))
    {
        return K_SPP_ERROR_NULL_POINTER;
    }

    spi_device_handle_t hDev = *(spi_device_handle_t *)p_handle;
    if (hDev == NULL)
    {
        return K_SPP_ERROR_NULL_POINTER;
    }

    spi_transaction_t trans = {0};
    trans.length = 8U * (size_t)length;
    trans.tx_buffer = p_data;
    trans.rx_buffer = p_data;

    esp_err_t ret = spi_device_polling_transmit(hDev, &trans);
    return (ret == ESP_OK) ? K_SPP_OK : K_SPP_ERROR_ON_SPI_TRANSACTION;
}

static SPP_RetVal_t SPP_PORTS_HAL_ESP32_spiDeviceSetSpeed(void *p_handle, spp_uint32_t speedHz)
{
    if (p_handle == NULL)
    {
        return K_SPP_ERROR_NULL_POINTER;
    }

    spi_device_handle_t *p_h = (spi_device_handle_t *)p_handle;

    spi_bus_remove_device(*p_h);

    spi_device_interface_config_t devCfg = {0};
    devCfg.clock_speed_hz = (int)speedHz;
    devCfg.mode = 0;
    devCfg.spics_io_num = K_ESP32_PIN_CS_SDC;
    devCfg.queue_size = 1;

    esp_err_t ret = spi_bus_add_device(K_ESP32_SPI_HOST, &devCfg, p_h);
    if (ret != ESP_OK)
    {
        ESP_LOGE(k_tag, "spiDeviceSetSpeed failed: %s", esp_err_to_name(ret));
        return K_SPP_ERROR;
    }

    return K_SPP_OK;
}

/* ----------------------------------------------------------------
 * GPIO
 * ---------------------------------------------------------------- */

static void IRAM_ATTR SPP_PORTS_HAL_ESP32_gpioIsr(void *p_arg)
{
    SPP_GpioIsrCtx_t *p_ctx = (SPP_GpioIsrCtx_t *)p_arg;
    *p_ctx->p_flag = true;
}

static SPP_RetVal_t SPP_PORTS_HAL_ESP32_gpioConfigInterrupt(spp_uint32_t pin, spp_uint32_t intrType, spp_uint32_t pull)
{
    gpio_config_t ioCfg = {
        .pin_bit_mask = (1ULL << pin),
        .mode = GPIO_MODE_INPUT,
        .intr_type = (gpio_int_type_t)intrType,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
    };

    if (pull == 1U)
    {
        ioCfg.pull_up_en = GPIO_PULLUP_ENABLE;
    }
    if (pull == 2U)
    {
        ioCfg.pull_down_en = GPIO_PULLDOWN_ENABLE;
    }

    gpio_config(&ioCfg);
    return K_SPP_OK;
}

static SPP_RetVal_t SPP_PORTS_HAL_ESP32_gpioRegisterIsr(spp_uint32_t pin, void *p_isrCtx)
{
    static spp_bool_t s_serviceInstalled = false;
    if (!s_serviceInstalled)
    {
        gpio_install_isr_service(0);
        s_serviceInstalled = true;
    }
    gpio_isr_handler_add((gpio_num_t)pin, SPP_PORTS_HAL_ESP32_gpioIsr, p_isrCtx);
    return K_SPP_OK;
}

/* ----------------------------------------------------------------
 * Storage
 * ---------------------------------------------------------------- */

static SPP_RetVal_t SPP_PORTS_HAL_ESP32_storageInit(void)
{
    esp_err_t ret;

    if (s_busInitialized == false)
    {
        return K_SPP_ERROR;
    }

    // sdspi_device_config_t sdCfg = {
    //     .host_id = K_ESP32_SPI_HOST,
    //     .gpio_cs = K_ESP32_PIN_CS_SDC,
    //     .gpio_wp_polarity = 1,
    //     .duty_cycle_pos = 0,
    //     .wait_for_miso = 0,
    // };

    sdspi_device_config_t sdCfg = SDSPI_DEVICE_CONFIG_DEFAULT();
    sdCfg.host_id = K_ESP32_SPI_HOST;
    sdCfg.gpio_cs = K_ESP32_PIN_CS_SDC;

    ret = sdspi_host_deinit();

    ret = sdspi_host_init();
    if (ret != ESP_OK)
    {
        return K_SPP_ERROR;
    }

    ret = sdspi_host_init_device(&sdCfg, &sdHandle);
    if (ret != ESP_OK)
    {
        (void)sdspi_host_deinit();
        return K_SPP_ERROR;
    }

    sdmmc_host_t host = SDSPI_HOST_DEFAULT(); // revisar
    host.slot = sdHandle;

    ret = sdmmc_card_init(&host, &sdCard);
    if (ret != ESP_OK)
    {
        ret = sdspi_host_remove_device(sdHandle);
        ret = sdspi_host_deinit();
        return K_SPP_ERROR;
    }

    s_storageInitialized = true;
    return K_SPP_OK;
}

static SPP_RetVal_t SPP_PORTS_HAL_ESP32_storageWrite(const void *p_buffer, spp_uint32_t first_block, spp_uint16_t count)
{
    if (s_storageInitialized == false || count == 0)
    {
        return K_SPP_ERROR;
    }

    if (p_buffer == NULL)
    {
        return K_SPP_ERROR_NULL_POINTER;
    }

    if (sdCard.csd.capacity <= first_block || sdCard.csd.capacity - first_block < count)
    {
        return K_SPP_ERROR;
    }

    esp_err_t ret = sdmmc_write_sectors(&sdCard, p_buffer, first_block, count);

    if (ret != ESP_OK)
    {
        return K_SPP_ERROR;
    }

    return K_SPP_OK;
}

/* ----------------------------------------------------------------
 * Time
 * ---------------------------------------------------------------- */

static spp_uint32_t SPP_PORTS_HAL_ESP32_getTimeMs(void)
{
    return (spp_uint32_t)(esp_timer_get_time() / 1000LL);
}

static void SPP_PORTS_HAL_ESP32_delayMs(spp_uint32_t ms)
{
    spp_uint32_t start = SPP_PORTS_HAL_ESP32_getTimeMs();
    while ((SPP_PORTS_HAL_ESP32_getTimeMs() - start) < ms)
    { /* busy-wait */
    }
}

/* ----------------------------------------------------------------
 * UART
 * ---------------------------------------------------------------- */
static SPP_RetVal_t SPP_PORTS_HAL_ESP32_uartPortInit(void)
{
    esp_err_t ret = ESP_OK;
    static spp_bool_t s_uartPortInitialized = false;

    if (s_uartPortInitialized == true)
    {
        return K_SPP_OK;
    }

    const uart_port_t uart_num = K_ESP32_UART_PORT_ID;

    uart_config_t espUartPortCfg = {
        .baud_rate = K_ESP32_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0,
    };

    ret = uart_param_config(uart_num, &espUartPortCfg);
    if (ret != ESP_OK)
    {
        return K_SPP_ERROR;
    }

    ret = uart_set_pin(uart_num, K_ESP32_UART_TX_PIN, K_ESP32_UART_RX_PIN, K_ESP32_UART_RTS_PIN, K_ESP32_UART_CTS_PIN);

    if (ret != ESP_OK)
    {
        return K_SPP_ERROR;
    }

    ret = uart_driver_install(uart_num, K_ESP32_UART_RX_BUFFER_SIZE, K_ESP32_UART_TX_BUFFER_SIZE, 0, NULL, 0);

    if (ret != ESP_OK)
    {
        return K_SPP_ERROR;
    }

    s_uartPortInitialized = true;

    return K_SPP_OK;
}

static SPP_RetVal_t SPP_PORTS_HAL_ESP32_uartTransmit(const void *p_data, spp_uint32_t len)
{
    if ((p_data == NULL) || (len == 0U))
    {
        return K_SPP_ERROR_NULL_POINTER;
    }

    const spp_uint8_t *p_dataToTransmit = (spp_uint8_t *)p_data;

    const int written = uart_write_bytes(K_ESP32_UART_PORT_ID, (void *)p_dataToTransmit, (size_t)len);

    if (written != (int)len)
    {
        return K_SPP_ERROR;
    }

    return K_SPP_OK;
}
