/**
 * @file e22-mbl01_service.c
 * @brief e22-mbl01 module telemetry + SPP service implementation.
 *
 * Use the UART driver to send and receive data through the air channel
 * and store the data in the SD card
 */

#include <stdio.h>
#include <string.h>
#include "spp/services/e22-mbl01/e22-mbl01.h"
#include "spp/core/types.h"
#include "spp/core/packet.h"
#include "spp/core/returnTypes.h"
#include "spp/hal/uart/uart.h"

/* -----------------------------------------
    DEFINES
--------------------------------------------*/

#define K_E22MBL01_TASK_TIMEOUT_MS 5000U
#define K_E22MBL01_MAILBOX_SIZE    128U
#define K_E22MBL01_SERVICE_APID    (0x0004U)

/* -----------------------------------------
    VARIABLES
--------------------------------------------*/
static SPP_Packet_t mailboxData[K_E22MBL01_MAILBOX_SIZE] = {0};
static spp_uint8_t s_mailboxCount = 0;
static spp_uint32_t s_frame_number = 1;

/* -----------------------------------------
    STATIC FUNCTIONS DECLARATIONS
--------------------------------------------*/
static SPP_RetVal_t SPP_SERVICES_E22MBL01_init(void);
static SPP_RetVal_t SPP_SERVICES_E22MBL01_consumeData(void *p_data);
static SPP_RetVal_t SPP_SERVICES_E22MBL01_deliverToMailbox(const SPP_Packet_t p_pkt);
// static SPP_RetVal_t SPP_SERVICES_E22MBL01_send_frame(spp_uint32_t frame_number, const char *data);

static void SPP_SERVICES_E22MBL01_bmpPacket_to_text(SPP_Packet_t *p_pkt, char *dataStr);
static void SPP_SERVICES_E22MBL01_icmPacket_to_text(SPP_Packet_t *p_pkt, char *dataStr);


static SPP_SERVICE_ConsumerContract_t e22Contract = {
    .consumerID = K_E22MBL01_SERVICE_APID,
    .priority = 1,
    .p_nameConsumer = "e22-mbl01",
    .tiemoutMs = K_E22MBL01_TASK_TIMEOUT_MS,
    .suscribeToApid = 0U,
    .init = SPP_SERVICES_E22MBL01_init,
    .deliverToMailbox = SPP_SERVICES_E22MBL01_deliverToMailbox,
    .consumeData = SPP_SERVICES_E22MBL01_consumeData,
};

/* -----------------------------------------
    PUBLIC FUNCTIONS IMPLEMENTATION
--------------------------------------------*/
SPP_SERVICE_ConsumerContract_t *SPP_SERVICES_E22MBL01_getConsumerContract()
{
    return &e22Contract;
}

/* -----------------------------------------
    STATIC FUNCTIONS IMPLEMENTATION
--------------------------------------------*/
static SPP_RetVal_t SPP_SERVICES_E22MBL01_init(void)
{
    SPP_RetVal_t ret = K_SPP_OK;

    const spp_uint8_t testData[3] = {0xFA, 0xBA, 0xDA};
    ret = SPP_HAL_UART_transmit((const void *)testData, sizeof(testData));
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    return K_SPP_OK;
}

static SPP_RetVal_t SPP_SERVICES_E22MBL01_deliverToMailbox(const SPP_Packet_t packet)
{
    if (s_mailboxCount < K_E22MBL01_MAILBOX_SIZE)
    {
        mailboxData[s_mailboxCount] = packet;
        s_mailboxCount++;
    }
    else
    {
        s_mailboxCount = 0;
        mailboxData[s_mailboxCount] = packet;
        s_mailboxCount++;
    }
    return K_SPP_OK;
}

static SPP_RetVal_t SPP_SERVICES_E22MBL01_consumeData(void *p_data)
{
    (void)p_data;
    for (spp_uint8_t index = 0; index < s_mailboxCount; index++)
    {
        SPP_Packet_t packet = mailboxData[index];
        if (packet.primaryHeader.apid == 4U)
        {
            printf("Sending packet with apid 4\n");
        }

        SPP_RetVal_t ret = SPP_HAL_UART_transmit(&packet, sizeof(packet));
        if (ret != K_SPP_OK)
        {
            return ret;
        }
    }
    s_mailboxCount = 0;

    return K_SPP_OK;
}
