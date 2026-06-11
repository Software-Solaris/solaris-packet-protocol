/**
 * @file e22-mbl01_service.c
 * @brief e22-mbl01 module telemetry + SPP service implementation.
 *
 * Use the UART driver to send and receive data through the air channel
 * and store the data in the SD card
 */

#include <stdio.h>
#include <string.h>
#include "HAL/uart.h"
#include "spp/core/types.h"


/* -----------------------------------------
    DEFINES
--------------------------------------------*/

#define K_UART_NUM 2 // hay que ver que puerto UART es el que esta libre en el ESP32


#define K_E22MBL01_SERVICE_APID    (0x0005U)
#define K_BMP390_SERVICE_APID      (0x0004U)
#define K_E22MBL01_TASK_TIMEOUT_MS 5000U
#define K_E22MBL01_MAILBOX_SIZE    128

/* -----------------------------------------
    STATIC FUNCTIONS DECLARATIONS
--------------------------------------------*/
static SPP_RetVal_t SPP_SERVICES_E22MBL01_init(void);
static SPP_RetVal_t SPP_SERVICES_E22MBL01_consumeData(void *p_data);


/* -----------------------------------------
    VARIABLES
--------------------------------------------*/
static SPP_Packet_t mailboxData[K_E22MBL01_MAILBOX_SIZE] = {0};

static SPP_SERVICE_ConsumerContract_t e22Contract =
    {
        .consumerID = K_E22MBL01_SERVICE_APID,
        .priority = 0,
        .p_nameConsumer = "e22-mbl01",
        .tiemoutMs = K_E22MBL01_TASK_TIMEOUT_MS,
        .suscribeToApid = K_BMP390_SERVICE_APID,
        .p_mailBox = mailboxData,
        .init = SPP_SERVICES_E22MBL01_init,
        .consumeData = SPP_SERVICES_E22MBL01_consumeData,
}

static int s_frame_number = 1;

/* -----------------------------------------
    STATIC FUNCTIONS IMPLEMENTATION
--------------------------------------------*/
static SPP_RetVal_t SPP_SERVICES_E22MBL01_init(void)
{
    SPP_RetVal_t ret = K_SPP_OK;

    spp_uint8_t testInitData[3U] = {0xFA, 0xBA, 0xDA};
    ret = SPP_HAL_UART_transmit(testInitData, sizeof(testInitData));
    if (ret != K_SPP_OK)
    {
        return ret;
    }
}

static SPP_RetVal_t SPP_SERVICES_E22MBL01_consumeData(void)
{
    SPP_RetVal_t ret = K_SPP_OK;


    SPP_RetVal_t ret = K_SPP_OK;
    if (p_data == NULL)
    {
        return K_SPP_ERROR_NULL_POINTER;
    }

    SPP_Packet_t *p_packet = (SPP_Packet_t *)p_data;

    // hay que extraer los datos del paquete y llamar a enviar trama

    SPP_E22MBL01_send_frame(s_frame_number, data);
    s_frame_number++;

    return K_SPP_OK;
}


static void SPP_SERVICES_E22MBL01_send_frame(int frame_number, char *data)
{
    if (frame_number > 99999)
    { // suponiendo una trama cada segundo: tiempo sobradamente para cubrir todo el vuelo
        return;
    }

    // le añadimos un identificador unico a cada mensaje
    const char *prefix_identifier = "TX_UV_";

    char identifier[16];
    sprintf(identifier, "%s%d", prefix_identifier, frame_number);

    // añadimos los datos
    char message[128];
    sprintf(mesagge, "%s DATOS_%d %s\n", identifier, strlen(data), data);

    // vaciar buffers de la UART antes de transmitir
    SPP_uart_flush_input(K_UART_NUM);

    // Transmitir string al devkit
    SPP_uart_write_bytes(K_UART_NUM, mesagge, strlen(mesagge));

    // FORMATO DEL MENSAJE: "TX_UV_{identificador} DATOS_{lon_datos_bytes} {SENSOR}={VALOR},{SENSOR}={VALOR}...\n"
}