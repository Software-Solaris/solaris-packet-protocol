/**
 * @file storagePort.c
 * @brief ESP32-S3 SD card storage port implementation.
 */

/* ----------------------------------------------------------------
 * INCLUDES
 * ---------------------------------------------------------------- */
#include "storagePort.h"

#include "macrosEsp32.h"
#include "../spi/spiPort.h"

#include "driver/spi_common.h"
#include "driver/gpio.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* ----------------------------------------------------------------
 * VARIABLES
 * ---------------------------------------------------------------- */
static sdspi_dev_handle_t sdHandle;
static sdmmc_card_t sdCard;
static spp_bool_t s_storageInitialized = false;

/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */

SPP_RetVal_t SPP_PORTS_HAL_ESP32_storageInit(void)
{
    esp_err_t ret;

    if (SPP_PORTS_HAL_ESP32_spiIsBusInitialized() == false)
    {
        return K_SPP_ERROR;
    }

    sdspi_device_config_t sdCfg = SDSPI_DEVICE_CONFIG_DEFAULT();
    sdCfg.host_id = K_ESP32_SPI_HOST;
    sdCfg.gpio_cs = K_ESP32_PIN_CS_SDC;

    (void)sdspi_host_deinit();

    gpio_set_direction(K_ESP32_PIN_CS_BMP, GPIO_MODE_OUTPUT);
    gpio_set_direction(K_ESP32_PIN_CS_ICM, GPIO_MODE_OUTPUT);
    gpio_set_level(K_ESP32_PIN_CS_BMP, 1);
    gpio_set_level(K_ESP32_PIN_CS_ICM, 1);

    for (int attempt = 0; attempt < 5; attempt++)
    {
        ret = sdspi_host_init();
        if (ret != ESP_OK)
        {
            continue;
        }

        ret = sdspi_host_init_device(&sdCfg, &sdHandle);
        if (ret != ESP_OK)
        {
            (void)sdspi_host_deinit();
            continue;
        }

        sdmmc_host_t host = SDSPI_HOST_DEFAULT();
        host.slot = sdHandle;

        vTaskDelay(pdMS_TO_TICKS(10));

        ret = sdmmc_card_init(&host, &sdCard);
        if (ret == ESP_OK)
        {
            s_storageInitialized = true;
            return K_SPP_OK;
        }

        (void)sdspi_host_remove_device(sdHandle);
        (void)sdspi_host_deinit();
        vTaskDelay(pdMS_TO_TICKS(50));
    }

    return K_SPP_ERROR;
}

SPP_RetVal_t SPP_PORTS_HAL_ESP32_storageWrite(const void *p_buffer, spp_uint32_t first_block, spp_uint16_t count)
{
    esp_err_t ret;

    if (s_storageInitialized == false || count == 0)
    {
        return K_SPP_ERROR;
    }

    if (p_buffer == NULL)
    {
        return K_SPP_ERROR_NULL_POINTER;
    }

    if (sdCard.csd.capacity <= first_block || (sdCard.csd.capacity - first_block) < count)
    {
        return K_SPP_ERROR;
    }

    ret = sdmmc_write_sectors(&sdCard, p_buffer, first_block, count);
    if (ret != ESP_OK)
    {
        return K_SPP_ERROR;
    }

    return K_SPP_OK;
}
