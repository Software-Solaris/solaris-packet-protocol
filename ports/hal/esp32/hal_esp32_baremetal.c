/**
 * @file hal_esp32_baremetal.c
 * @brief ESP32 HAL port for SPP — baremetal / polling variant.
 *
 * Identical to the FreeRTOS HAL except:
 *   - SPI uses spi_device_polling_transmit() — no FreeRTOS semaphore.
 *   - Time uses esp_timer_get_time() — no xTaskGetTickCount().
 *   - GPIO ISR does not call portYIELD_FROM_ISR().
 *
 * Register @ref g_esp32BaremetalHalPort before calling @ref SPP_Core_init().
 */

#include "spp/hal/port.h"
#include "spp/core/returntypes.h"
#include "spp/core/types.h"
#include "spp/osal/event.h"
#include "macros_esp32.h"

#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "driver/sdspi_host.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "esp_log.h"
#include "esp_timer.h"

#include <string.h>

/* ----------------------------------------------------------------
 * Private state
 * ---------------------------------------------------------------- */

static const char *const k_tag = "SPP_HAL_BM";

static spi_device_handle_t s_spiHandles[K_ESP32_MAX_SPI_DEVICES];
static spp_uint8_t         s_spiDevCount   = 0U;
static spp_bool_t          s_busInitialized = false;

static sdmmc_card_t *s_p_sdCard  = NULL;
static spp_bool_t    s_sdMounted = false;

/* ----------------------------------------------------------------
 * SPI — polling (no FreeRTOS semaphore)
 * ---------------------------------------------------------------- */

static SPP_RetVal_t bmSpiBusInit(void)
{
    if (s_busInitialized) { return K_SPP_OK; }

    spi_bus_config_t busCfg = {
        .miso_io_num     = K_ESP32_PIN_MISO,
        .mosi_io_num     = K_ESP32_PIN_MOSI,
        .sclk_io_num     = K_ESP32_PIN_CLK,
        .quadwp_io_num   = -1,
        .quadhd_io_num   = -1,
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

static void *bmSpiGetHandle(spp_uint8_t deviceIdx)
{
    if (deviceIdx >= K_ESP32_MAX_SPI_DEVICES) { return NULL; }
    return (void *)&s_spiHandles[deviceIdx];
}

static SPP_RetVal_t bmSpiDeviceInit(void *p_handle)
{
    if (p_handle == NULL)                          { return K_SPP_ERROR_NULL_POINTER; }
    if (s_spiDevCount >= K_ESP32_MAX_SPI_DEVICES)  { return K_SPP_ERROR; }

    spi_device_handle_t           *p_h    = (spi_device_handle_t *)p_handle;
    spi_device_interface_config_t  devCfg = {0};

    if (s_spiDevCount == K_ESP32_SPI_IDX_ICM)
    {
        devCfg.clock_speed_hz = 1 * 1000 * 1000;
        devCfg.mode           = 0;
        devCfg.spics_io_num   = K_ESP32_PIN_CS_ICM;
        devCfg.queue_size     = 1;   /* polling: queue depth 1 */
    }
    else if (s_spiDevCount == K_ESP32_SPI_IDX_BMP)
    {
        devCfg.clock_speed_hz = 500 * 1000;
        devCfg.mode           = 0;
        devCfg.spics_io_num   = K_ESP32_PIN_CS_BMP;
        devCfg.queue_size     = 1;
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

static SPP_RetVal_t bmSpiTransmit(void *p_handle, spp_uint8_t *p_data, spp_uint8_t length)
{
    if ((p_handle == NULL) || (p_data == NULL) || (length == 0U))
    {
        return K_SPP_ERROR_NULL_POINTER;
    }

    spi_device_handle_t hDev = *(spi_device_handle_t *)p_handle;
    if (hDev == NULL) { return K_SPP_ERROR_NULL_POINTER; }

    spp_uint8_t i = 0U;
    while (i < length)
    {
        spi_transaction_t trans = {0};

        if (p_data[i] & 0x80U)
        {
            trans.length    = 8U * 3U;
            trans.tx_buffer = &p_data[i];
            trans.rx_buffer = &p_data[i];
            i += 3U;
        }
        else
        {
            trans.length    = 8U * 2U;
            trans.tx_buffer = &p_data[i];
            i += 2U;
        }

        /* Polling transmit — no FreeRTOS semaphore required. */
        esp_err_t ret = spi_device_polling_transmit(hDev, &trans);
        if (ret != ESP_OK) { return K_SPP_ERROR_ON_SPI_TRANSACTION; }
    }
    return K_SPP_OK;
}

/* ----------------------------------------------------------------
 * GPIO — no portYIELD_FROM_ISR (baremetal)
 * ---------------------------------------------------------------- */

static void bmGpioIsr(void *p_arg)
{
    SPP_GpioIsrCtx_t *p_ctx = (SPP_GpioIsrCtx_t *)p_arg;
    /* In baremetal the event group is a plain bitmask — no yield needed. */
    (void)SPP_OSAL_eventSetFromIsr(p_ctx->p_eventGroup, p_ctx->bits, NULL, NULL);
}

static SPP_RetVal_t bmGpioConfigInterrupt(spp_uint32_t pin, spp_uint32_t intrType,
                                       spp_uint32_t pull)
{
    gpio_config_t ioCfg = {
        .pin_bit_mask = (1ULL << pin),
        .mode         = GPIO_MODE_INPUT,
        .intr_type    = (gpio_int_type_t)intrType,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
    };

    if (pull == 1U) { ioCfg.pull_up_en   = GPIO_PULLUP_ENABLE;   }
    if (pull == 2U) { ioCfg.pull_down_en = GPIO_PULLDOWN_ENABLE; }

    gpio_config(&ioCfg);
    return K_SPP_OK;
}

static SPP_RetVal_t bmGpioRegisterIsr(spp_uint32_t pin, void *p_isrCtx)
{
    static spp_bool_t s_serviceInstalled = false;
    if (!s_serviceInstalled)
    {
        gpio_install_isr_service(0);
        s_serviceInstalled = true;
    }
    gpio_isr_handler_add((gpio_num_t)pin, bmGpioIsr, p_isrCtx);
    return K_SPP_OK;
}

/* ----------------------------------------------------------------
 * Storage (unchanged from FreeRTOS HAL)
 * ---------------------------------------------------------------- */

static SPP_RetVal_t bmStorageMount(void *p_cfg)
{
    if (s_sdMounted) { return K_SPP_OK; }

    const SPP_StorageInitCfg_t *p_c = (const SPP_StorageInitCfg_t *)p_cfg;

    sdmmc_host_t          host    = SDSPI_HOST_DEFAULT();
    sdspi_device_config_t slotCfg = SDSPI_DEVICE_CONFIG_DEFAULT();
    slotCfg.gpio_cs = (gpio_num_t)p_c->pinCs;
    slotCfg.host_id = (spi_host_device_t)p_c->spiHostId;

    esp_vfs_fat_mount_config_t mountCfg = {
        .format_if_mount_failed = (bool)p_c->formatIfMountFailed,
        .max_files              = (int)p_c->maxFiles,
        .allocation_unit_size   = (size_t)p_c->allocationUnitSize,
    };

    esp_err_t ret = esp_vfs_fat_sdspi_mount(p_c->p_basePath, &host,
                                             &slotCfg, &mountCfg, &s_p_sdCard);
    if (ret != ESP_OK)
    {
        s_p_sdCard = NULL;
        ESP_LOGE(k_tag, "SD mount failed: %s", esp_err_to_name(ret));
        return K_SPP_ERROR;
    }

    s_sdMounted = true;
    return K_SPP_OK;
}

static SPP_RetVal_t bmStorageUnmount(void *p_cfg)
{
    if (!s_sdMounted) { return K_SPP_OK; }

    const SPP_StorageInitCfg_t *p_c = (const SPP_StorageInitCfg_t *)p_cfg;
    esp_err_t ret = esp_vfs_fat_sdcard_unmount(p_c->p_basePath, s_p_sdCard);
    s_sdMounted = false;
    s_p_sdCard  = NULL;
    return (ret == ESP_OK) ? K_SPP_OK : K_SPP_ERROR;
}

/* ----------------------------------------------------------------
 * Time — esp_timer, no FreeRTOS
 * ---------------------------------------------------------------- */

static spp_uint32_t bmGetTimeMs(void)
{
    return (spp_uint32_t)(esp_timer_get_time() / 1000LL);
}

/* ----------------------------------------------------------------
 * Port descriptor
 * ---------------------------------------------------------------- */

const SPP_HalPort_t g_esp32BaremetalHalPort = {
    .spiBusInit          = bmSpiBusInit,
    .spiGetHandle        = bmSpiGetHandle,
    .spiDeviceInit       = bmSpiDeviceInit,
    .spiTransmit         = bmSpiTransmit,
    .gpioConfigInterrupt = bmGpioConfigInterrupt,
    .gpioRegisterIsr     = bmGpioRegisterIsr,
    .storageMount        = bmStorageMount,
    .storageUnmount      = bmStorageUnmount,
    .getTimeMs           = bmGetTimeMs,
};
