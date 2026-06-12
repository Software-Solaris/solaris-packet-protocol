/**
 * @file uart.c
 * @brief HAL UART functions implementation. Calls the function pointer and returns the result.
 */

/* ----------------------------------------------------------------
 * INCLUDES
 * ---------------------------------------------------------------- */
#include "spp/core/returnTypes.h"
#include "spp/hal/hal.h"
#include "spp/hal/uart/uart.h"

#ifdef SPP_ENCRYPTION
#include "external/encryption/cipher.h"
#include <string.h>
#endif

/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */

SPP_RetVal_t SPP_HAL_UART_portInit(void)
{
    SPP_RetVal_t ret = K_SPP_OK;
    const SPP_HalPort_t *p_port = SPP_HAL_getPort();

    if (p_port == NULL)
    {
        ret = K_SPP_ERROR_NULL_POINTER;
        return ret;
    }

    if (p_port->uart.uartPortInit == NULL)
    {
        ret = K_SPP_ERROR;
        return ret;
    }

    ret = p_port->uart.uartPortInit();

    return ret;
}

SPP_RetVal_t SPP_HAL_UART_transmit(const void *p_data, spp_uint16_t len)
{
    const SPP_HalPort_t *p_port = SPP_HAL_getPort();

    if (p_port == NULL || p_data == NULL)
    {
        return K_SPP_ERROR_NULL_POINTER;
    }

    if (p_port->uart.uartTransmit == NULL)
    {
        return K_SPP_ERROR;
    }

#ifdef SPP_ENCRYPTION
    spp_uint8_t encBuf[K_SPP_CIPHER_FRAME_LEN];
    spp_uint16_t encLen = 0U;

    memcpy(encBuf, p_data, len);

    SPP_RetVal_t ret = SPP_EXTERNAL_ENCRYPTION_CYPHER_encryptPacket(encBuf, &encLen);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    return p_port->uart.uartTransmit(encBuf, (spp_uint32_t)encLen);
#else
    return p_port->uart.uartTransmit(p_data, len);
#endif
}
