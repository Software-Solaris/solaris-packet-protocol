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
    static spp_uint8_t mailboxTail = 0;

    if (s_mailboxCount < K_E22MBL01_MAILBOX_SIZE)
    {
        mailboxData[mailboxTail] = packet;
        mailboxTail = (spp_uint8_t)((mailboxTail + 1U) % K_E22MBL01_MAILBOX_SIZE); // suma circular
        s_mailboxCount++; // usaremos s_mailboxCount para saber cuantos paquetes tendremos que enviar luego
        return K_SPP_OK;
    }
    else
    {
        return K_SPP_ERROR;
    }
}

static SPP_RetVal_t SPP_SERVICES_E22MBL01_consumeData(void *p_data)
{
    (void)p_data;
    char sensorsStr[256];
    char strBMP390[64];
    char strICM20948[128];

    for (spp_uint8_t index = 0; index < s_mailboxCount; index++)
    {
        SPP_Packet_t packet = mailboxData[index];

        // sprintf(strBMP390, "BMP390:-");
        // sprintf(strICM20948, "ICM20948:-");

        // spp_uint8_t packetAPID = packet.primaryHeader.apid;
        // if (packetAPID == K_BMP390_SERVICE_APID)
        // {
        //     SPP_SERVICES_E22MBL01_bmpPacket_to_text(&packet, strBMP390);
        // }
        // else if (packetAPID == K_ICM20948_SERVICE_APID)
        // {
        //     SPP_SERVICES_E22MBL01_icmPacket_to_text(&packet, strICM20948);
        // }

        // sprintf(sensorsStr, "%s,%s,", strBMP390, strICM20948);

        // SPP_SERVICES_E22MBL01_send_frame(s_frame_number, sensorsStr);

        SPP_RetVal_t ret = SPP_HAL_UART_transmit(&packet, sizeof(packet));
        if (ret != K_SPP_OK)
        {
            return ret;
        }
        // s_frame_number++;
    }
    s_mailboxCount = 0;

    return K_SPP_OK;
}

// static SPP_RetVal_t SPP_SERVICES_E22MBL01_send_frame(spp_uint32_t frame_number, const char *data)
// {
//     // FORMATO DEL MENSAJE: "TX_UV_{identificador} DATA_{lon_datos_bytes} {SENSOR}:{VALOR|VALOR|VALOR...},{SENSOR}={VALOR|VALOR|VALOR...}...\n"
//     SPP_RetVal_t ret;

//     if (frame_number > 99999)
//     {
//         // suponiendo una trama cada segundo: tiempo sobradamente para cubrir todo el vuelo
//         return K_SPP_ERROR;
//     }

//     const char *prefix_identifier = "TX_UV_";

//     char identifier[16];
//     sprintf(identifier, "%s%u", prefix_identifier, frame_number);

//     char message[256];
//     sprintf(message, "%s DATA_%d %s\n", identifier, (int)strlen(data), data);

//     ret = SPP_HAL_UART_transmit(message, strlen(message));
//     if (ret != K_SPP_OK)
//     {
//         return ret;
//     }

//     return K_SPP_OK;
// }

SPP_SERVICE_ConsumerContract_t *SPP_SERVICES_E22MBL01_getConsumerContract()
{
    return &e22Contract;
}

static void SPP_SERVICES_E22MBL01_bmpPacket_to_text(SPP_Packet_t *p_pkt, char *dataStr)
{
    float *payload = (float *)p_pkt->payload;

    float altitude = payload[0];
    float pressure = payload[1];
    float temperature = payload[2];

    sprintf(dataStr, "BMP390:alt=%.1f|P=%.1f|T=%.2f", altitude, pressure, temperature);

    return;
}

static void SPP_SERVICES_E22MBL01_icmPacket_to_text(SPP_Packet_t *p_pkt, char *dataStr)
{
    float *payload = (float *)p_pkt->payload;

    float ax = payload[0];
    float ay = payload[1];
    float az = payload[2];
    float gx = payload[3];
    float gy = payload[4];
    float gz = payload[5];
    float mx = payload[6];
    float my = payload[7];
    float mz = payload[8];

    sprintf(dataStr, "ICM20948:ax=%.2f|ay=%.2f|az=%.2f|gx=%.2f|gy=%.2f|gz=%.2f|mx=%.2f|my=%.2f|mz=%.2f", ax, ay, az, gx,
            gy, gz, mx, my, mz);

    return;
}