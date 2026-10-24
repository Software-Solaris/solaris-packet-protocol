/**
 * @file uart.c
 * @brief UART telemetry consumer service implementation.
 *
 * The service keeps a small FIFO of SPP packets delivered by the PUBSUB
 * router.  Every superloop tick, @ref SPP_SERVICES_UART_consumeData transmits
 * the oldest pending packet over the UART HAL, byte-for-byte.
 */

/* ----------------------------------------------------------------
 * INCLUDES
 * ---------------------------------------------------------------- */
#include "spp/services/uart/uart.h"

#include "spp/hal/uart/uart.h"

/* ----------------------------------------------------------------
 * STATIC FUNCTIONS DECLARATIONS
 * ---------------------------------------------------------------- */
static SPP_RetVal_t SPP_SERVICES_UART_init(void);
static SPP_RetVal_t SPP_SERVICES_UART_deliverToMailbox(const SPP_Packet_t packet);
static SPP_RetVal_t SPP_SERVICES_UART_consumeData(void *p_data);

/* ----------------------------------------------------------------
 * VARIABLES
 * ---------------------------------------------------------------- */
static SPP_Packet_t s_mailbox[K_UART_MAILBOX_SIZE];
static spp_uint8_t s_mailboxHead = 0U;
static spp_uint8_t s_mailboxTail = 0U;
static spp_uint8_t s_mailboxCount = 0U;

static SPP_RetVal_t s_initStatus = K_SPP_ERROR_NOT_INITIALIZED;

static const SPP_SERVICE_ConsumerContract_t g_uartConsumerContract = {
    .priority = K_UART_CONSUMER_PRIO,
    .p_nameConsumer = "uart",
    .tiemoutMs = K_UART_TIMEOUT_MS,
    .init = SPP_SERVICES_UART_init,
    .deliverToMailbox = SPP_SERVICES_UART_deliverToMailbox,
    .consumeData = SPP_SERVICES_UART_consumeData,
};

/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */
const SPP_SERVICE_ConsumerContract_t *SPP_SERVICES_UART_getConsumerContract(void)
{
    return &g_uartConsumerContract;
}

SPP_RetVal_t SPP_SERVICES_UART_getInitStatus(void)
{
    return s_initStatus;
}

/* ----------------------------------------------------------------
 * STATIC FUNCTIONS
 * ---------------------------------------------------------------- */

/**
 * @brief Initialise the UART port and clear the packet mailbox.
 * @return K_SPP_OK on success, error code from the HAL otherwise.
 */
static SPP_RetVal_t SPP_SERVICES_UART_init(void)
{
    s_mailboxHead = 0U;
    s_mailboxTail = 0U;
    s_mailboxCount = 0U;

    s_initStatus = SPP_HAL_UART_portInit();
    return s_initStatus;
}

/**
 * @brief Store a packet delivered by the PUBSUB router.
 * @param  packet  Packet dispatched by the router.
 * @return K_SPP_OK always; when the FIFO is full the oldest packet is dropped.
 */
static SPP_RetVal_t SPP_SERVICES_UART_deliverToMailbox(const SPP_Packet_t packet)
{
    s_mailbox[s_mailboxTail] = packet;
    s_mailboxTail = (spp_uint8_t)((s_mailboxTail + 1U) % K_UART_MAILBOX_SIZE);

    if (s_mailboxCount < K_UART_MAILBOX_SIZE)
    {
        s_mailboxCount++;
    }
    else
    {
        s_mailboxHead = (spp_uint8_t)((s_mailboxHead + 1U) % K_UART_MAILBOX_SIZE);
    }

    return K_SPP_OK;
}

/**
 * @brief Transmit the oldest pending SPP packet over UART.
 * @param  p_data  Unused (context pointer reserved for future use).
 * @return K_SPP_OK on success (or empty mailbox), error code from the HAL otherwise.
 */
static SPP_RetVal_t SPP_SERVICES_UART_consumeData(void *p_data)
{
    (void)p_data;

    if (s_mailboxCount == 0U)
    {
        return K_SPP_OK;
    }

    SPP_RetVal_t ret = SPP_HAL_UART_transmit(&s_mailbox[s_mailboxHead], (spp_uint16_t)sizeof(SPP_Packet_t));
    if (ret == K_SPP_OK)
    {
        s_mailboxHead = (spp_uint8_t)((s_mailboxHead + 1U) % K_UART_MAILBOX_SIZE);
        s_mailboxCount--;
    }

    return ret;
}
