/**
 * @file uart.h
 * @brief UART telemetry consumer service.
 *
 * Receives every SPP packet routed by the publisher/subscriber service and
 * transmits it over the UART port exactly as it was produced (full
 * @ref SPP_Packet_t, no re-framing).
 *
 */

#ifndef SPP_SERVICES_UART_H
#define SPP_SERVICES_UART_H

#include "spp/core/returnTypes.h"
#include "spp/services/service.h"

/* ----------------------------------------------------------------
 * DEFINES
 * ---------------------------------------------------------------- */
#define K_UART_CONSUMER_PRIO (3)     /**< Consumer priority */
#define K_UART_TIMEOUT_MS    (1000U) /**< Consumer timeout in ms. */
#define K_UART_MAILBOX_SIZE  (8U)    /**< Packets buffered before dropping the oldest. */

/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */

/**
 * @brief  Get the UART consumer contract.
 * @return Pointer to the static consumer contract.
 */
const SPP_SERVICE_ConsumerContract_t *SPP_SERVICES_UART_getConsumerContract(void);

/**
 * @brief  Get the result of the last UART service init.
 * @return K_SPP_OK if the UART port was initialised, error code otherwise.
 */
SPP_RetVal_t SPP_SERVICES_UART_getInitStatus(void);

#endif /* SPP_SERVICES_UART_H */
