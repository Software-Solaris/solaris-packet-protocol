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


/* -----------------------------------------
    DEFINES
--------------------------------------------*/

#define K_UART_NUM 2 // hay que ver que puerto UART es el que esta libre en el ESP32
#define K_BAUD_RATE 9600 // velocidad de transmision - 9600 para el E22 obligatorio por UART
#define K_BUF_SIZE 512   // el buffer del devkit son 255 bytes
#define K_DATA_BITS 8    // BITS UTILES POR TRAMA
#define K_PARITY 0       // BIT EXTRA PARA CORRECCION DE ERRORES, LO DESACTIVAMOS
#define K_STOP_BITS 1    // numero de bits necesarios para que se pare la transmision (1- el mas basico)
#define K_FLOW_CTRL 0    // desactivamos las lineas de aviso de buffers llenos (no se llenaran, lo desactivamos)

#define K_E22MBL01_SERVICE_APID (0x0005U)
#define K_E22MBL01_TASK_TIMEOUT_MS 5000U

/* -----------------------------------------
    STATIC FUNCTIONS DECLARATIONS
--------------------------------------------*/
static SPP_RetVal_t SPP_E22_init(void);
static SPP_RetVal_t SPP_E22_consumeData(void *p_data);



/* -----------------------------------------
    VARIABLES
--------------------------------------------*/
const SPP_HAL_Uart_Config_t SPP_uart_config = {
    .baud_rate = K_BAUD_RATE,
    .data_bits = K_DATA_BITS,
    .parity = K_PARITY,
    .stop_bits = K_STOP_BITS,
    .flow_ctrl = K_FLOW_CTRL,
};

static SPP_SERVICE_ConsumerContract_t e22Contract = {
    .consumerID = K_E22MBL01_SERVICE_APID,
    .priority = 0,
    .p_nameConsumer = "e22-mbl01",
    .tiemoutMs = K_E22MBL01_TASK_TIMEOUT_MS,
    .suscribeToApid = 0x0004U, // puse el del BMP
    .p_mailBox = 00, // no se que poner
    .init = SPP_E22_init,
    .consumeData = SPP_E22_consumeData,
}

static int s_frame_number = 1;

/* -----------------------------------------
    STATIC FUNCTIONS IMPLEMENTATION
--------------------------------------------*/
static SPP_RetVal_t SPP_E22_init(void)
{
    SPP_RetVal_t ret = K_SPP_OK;

    ret = SPP_HAL_UART_init(K_UART_NUM, &SPP_uart_config);
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    return K_SPP_OK;
}

static SPP_RetVal_t SPP_E22_consumeData(void *p_data)
{
    SPP_RetVal_t ret = K_SPP_OK;
    if(p_data == NULL)
    {
        return K_SPP_ERROR_NULL_POINTER;
    }

    SPP_Packet_t *p_packet = (SPP_Packet_t *)p_data;

    // hay que extraer los datos del paquete y llamar a enviar trama

    SPP_E22MBL01_send_frame(s_frame_number, data);
    s_frame_number++;

    return K_SPP_OK;
}

static void SPP_E22_send_frame(int frame_number, char *data)
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