/**
 * @file uartPort.c
 * @brief ESP32-S3 UART port implementation.
 */

/* ----------------------------------------------------------------
 * INCLUDES
 * ---------------------------------------------------------------- */
#include "uartPort.h"

#include "macrosEsp32.h"

#include "driver/uart.h"

/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */

SPP_RetVal_t SPP_PORTS_HAL_ESP32_uartPortInit(void)
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

SPP_RetVal_t SPP_PORTS_HAL_ESP32_uartTransmit(const void *p_data, spp_uint32_t len)
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

/**
 * @brief Reads data from the ESP32 UART.
 *
 * @param[out] p_data      Pointer to the buffer where received data will be stored.
 * @param[in]  len         Number of bytes to read.
 * @param[out] p_readBytes Pointer to the variable where the number of bytes read will be stored.
 */
SPP_RetVal_t SPP_PORTS_HAL_ESP32_uartRead(void *p_data, spp_uint32_t len, spp_uint32_t *p_readBytes)
{
    if ((p_data == NULL) || (len == 0U))
    {
        return K_SPP_ERROR_NULL_POINTER;
    }

    spp_uint8_t *p_dataToRead = (spp_uint8_t *)p_data;

    const int read = uart_read_bytes(K_ESP32_UART_PORT_ID, p_dataToRead, (size_t)len, K_ESP32_UART_MAX_DELAY);

    if (read < 0)
    {
        return K_SPP_ERROR;
    }

    *p_readBytes = (spp_uint32_t)read;
    return K_SPP_OK;
}
