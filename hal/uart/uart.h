/**
 * @file uart.h
 * @brief SPP UART HAL API — dispatches through the registered HAL port.
 */

#include "spp/core/types.h"
#include "spp/core/returnTypes.h"

/* ----------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------- */

/**
  * @brief Initialise the UART port.
  * 
  * @return K_SPP_OK on success.
  */
SPP_RetVal_t SPP_HAL_UART_portInit(void);


/**
  * @brief Perform UART transaction.
  * 
  * @return K_SPP_OK on success.
  */
SPP_RetVal_t SPP_HAL_UART_transmit(const void *p_data, spp_uint16_t len);

/**
  * @brief Perform UART read.
  * 
  * @return K_SPP_OK on success.
  */
SPP_RetVal_t SPP_HAL_UART_read(void *p_data, spp_uint32_t len, spp_uint32_t *p_readBytes);