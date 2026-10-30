/**
 * @file gpioPort.c
 * @brief ESP32-S3 GPIO port implementation.
 */

/* ----------------------------------------------------------------
 * INCLUDES
 * ---------------------------------------------------------------- */
#include "gpioPort.h"

#include "spp/hal/gpio/gpio.h"

#include "driver/gpio.h"
#include "esp_attr.h"

/* ----------------------------------------------------------------
 * STATIC FUNCTIONS
 * ---------------------------------------------------------------- */

static void IRAM_ATTR SPP_PORTS_HAL_ESP32_gpioIsr(void *p_arg)
{
    SPP_GpioIsrCtx_t *p_ctx = (SPP_GpioIsrCtx_t *)p_arg;
    *p_ctx->p_flag = true;
}

/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */

SPP_RetVal_t SPP_PORTS_HAL_ESP32_gpioConfigInterrupt(spp_uint32_t pin, spp_uint32_t intrType, spp_uint32_t pull)
{
    SPP_RetVal_t ret = K_SPP_ERROR;

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

    ret = gpio_config(&ioCfg);

    if (ret != K_SPP_OK)
    {
        return ret;
    }

    return K_SPP_OK;
}

SPP_RetVal_t SPP_PORTS_HAL_ESP32_gpioRegisterIsr(spp_uint32_t pin, void *p_isrCtx)
{
    esp_err_t ret = ESP_FAIL;
    static spp_bool_t s_serviceInstalled = false;
    if (!s_serviceInstalled)
    {
        ret = gpio_install_isr_service(0);

        if (ret != ESP_OK)
        {
            return K_SPP_ERROR;
        }

        s_serviceInstalled = true;
    }

    ret = gpio_isr_handler_add((gpio_num_t)pin, SPP_PORTS_HAL_ESP32_gpioIsr, p_isrCtx);

    if (ret != ESP_OK)
    {
        return K_SPP_ERROR;
    }

    return K_SPP_OK;
}

spp_uint16_t SPP_PORTS_HAL_ESP32_gpioRead(spp_uint32_t pin)
{
    spp_uint16_t level = (spp_uint16_t)gpio_get_level((gpio_num_t)pin);

    return level;
}

SPP_RetVal_t SPP_PORTS_HAL_ESP32_gpioWrite(spp_uint32_t pin, spp_uint8_t level)
{
    esp_err_t ret = gpio_set_direction((gpio_num_t)pin, GPIO_MODE_OUTPUT);
    if (ret != ESP_OK)
    {
        return K_SPP_ERROR;
    }

    ret = gpio_set_level((gpio_num_t)pin, level);
    if (ret != ESP_OK)
    {
        return K_SPP_ERROR;
    }

    return K_SPP_OK;
}
