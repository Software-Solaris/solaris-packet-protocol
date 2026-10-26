/**
 * @file spiPort.c
 * @brief ESP32-S3 SPI port implementation.
 */

/* ----------------------------------------------------------------
 * INCLUDES
 * ---------------------------------------------------------------- */
#include "spiPort.h"

#include "macrosEsp32.h"

#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"

/* ----------------------------------------------------------------
 * VARIABLES
 * ---------------------------------------------------------------- */
static const char *const k_tag = "SPP_HAL";

static spi_device_handle_t s_spiHandles[K_ESP32_MAX_SPI_DEVICES];
static spp_bool_t s_busInitialized = false;

/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */

SPP_RetVal_t SPP_PORTS_HAL_ESP32_spiBusInit(void)
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
    gpio_config(&misoCfg);
    int misoLevel = gpio_get_level((gpio_num_t)K_ESP32_PIN_MISO);

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

void *SPP_PORTS_HAL_ESP32_spiGetHandle(spp_uint8_t deviceIdx)
{
    if (deviceIdx >= K_ESP32_MAX_SPI_DEVICES)
    {
        return NULL;
    }
    return (void *)&s_spiHandles[deviceIdx];
}

SPP_RetVal_t SPP_PORTS_HAL_ESP32_spiDeviceInit(void *p_handle)
{
    if (p_handle == NULL)
    {
        return K_SPP_ERROR_NULL_POINTER;
    }

    spp_uint8_t deviceIdx = K_ESP32_MAX_SPI_DEVICES;

    for (spp_uint8_t i = 0; i < K_ESP32_MAX_SPI_DEVICES; i++)
    {
        if (p_handle == &s_spiHandles[i])
        {
            deviceIdx = i;
            break;
        }
    }

    if (deviceIdx >= K_ESP32_MAX_SPI_DEVICES)
    {
        return K_SPP_ERROR;
    }

    spi_device_handle_t *p_h = (spi_device_handle_t *)p_handle;
    spi_device_interface_config_t devCfg = {0};

    if (deviceIdx == K_ESP32_SPI_IDX_ICM)
    {
        devCfg.clock_speed_hz = 1 * 1000 * 1000;
        devCfg.mode = 0;
        devCfg.spics_io_num = K_ESP32_PIN_CS_ICM;
        devCfg.queue_size = 1;
    }
    else if (deviceIdx == K_ESP32_SPI_IDX_BMP)
    {
        devCfg.clock_speed_hz = 500 * 1000;
        devCfg.mode = 0;
        devCfg.spics_io_num = K_ESP32_PIN_CS_BMP;
        devCfg.queue_size = 1;
    }
    else if (deviceIdx == K_ESP32_SPI_IDX_SX1262)
    {
        devCfg.clock_speed_hz = 1 * 1000 * 1000; // Max: 16MHz
        devCfg.mode = 0;
        devCfg.spics_io_num = K_ESP32_PIN_CS_SX1262;
        devCfg.queue_size = 1;
    }
    else
    {
        ESP_LOGE(k_tag, "Unexpected device index %u", deviceIdx);
        return K_SPP_ERROR;
    }

    esp_err_t ret = spi_bus_add_device(K_ESP32_SPI_HOST, &devCfg, p_h);
    if (ret != ESP_OK)
    {
        ESP_LOGE(k_tag, "spi_bus_add_device failed: %s", esp_err_to_name(ret));
        return K_SPP_ERROR;
    }

    return K_SPP_OK;
}

SPP_RetVal_t SPP_PORTS_HAL_ESP32_spiTransmit(void *p_handle, spp_uint8_t *p_data, spp_uint8_t length)
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

    spp_uint8_t i = 0U;

    while (i < length)
    {
        spi_transaction_t trans = {0};

        if ((p_data[i] & 0x80U) != 0U)
        {
            trans.length = 8U * 3U;
            trans.tx_buffer = &p_data[i];
            trans.rx_buffer = &p_data[i];

            i += 3U;
        }
        else
        {
            trans.length = 8U * 2U;
            trans.tx_buffer = &p_data[i];

            i += 2U;
        }

        esp_err_t ret = spi_device_polling_transmit(hDev, &trans);

        if (ret != ESP_OK)
        {
            return K_SPP_ERROR_ON_SPI_TRANSACTION;
        }
    }

    return K_SPP_OK;
}

SPP_RetVal_t SPP_PORTS_HAL_ESP32_spiDeviceSetSpeed(void *p_handle, spp_uint32_t speedHz)
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

spp_bool_t SPP_PORTS_HAL_ESP32_spiIsBusInitialized(void)
{
    return s_busInitialized;
}
