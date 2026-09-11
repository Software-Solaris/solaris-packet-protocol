/**
 * @file sx1262.c
 * @brief sx1262 module telemetry + SPP service implementation.
 */

#include "spp/services/sx1262/sx1262.h"

#include <stdio.h>
#include <string.h>
#include "spp/services/sx1262/sx1262.h"
#include "spp/core/packet.h"
#include "spp/hal/spi.h"
#include "spp/hal/gpio/gpio.h"
#include "spp/hal/time/time.h"


/* -----------------------------------------
    VARIABLES
--------------------------------------------*/
static SPP_Packet_t mailboxData[K_SX1262_MAILBOX_SIZE] = {0};

static spp_uint8_t mailboxHead = 0;
static spp_uint8_t mailboxTail = 0;
static spp_uint8_t mailboxCount = 0;

static spp_uint32_t s_frame_number = 1;

static SPP_SERVICE_ConsumerContract_t sx1262Contract = {
    .consumerID = K_SX1262_SERVICE_APID,
    .priority = 1,
    .p_nameConsumer = "sx1262",
    .tiemoutMs = K_SX1262_TASK_TIMEOUT_MS,
    .init = SPP_SERVICES_SX1262_init,
    .deliverToMailbox = SPP_SERVICES_SX1262_deliverToMailbox,
    .consumeData = SPP_SERVICES_SX1262_consumeData,
};

volatile spp_bool_t g_txDoneFlag = SPP_FALSE; // volatile para que no se optimice el bucle de espera
SPP_GpioIsrCtx_t g_dio1Ctx = { .p_flag = &g_txDoneFlag };


/* -----------------------------------------
    STATIC FUNCTIONS DECLARATIONS
--------------------------------------------*/
static SPP_RetVal_t SPP_SERVICES_SX1262_init(void);
static SPP_RetVal_t SPP_SERVICES_SX1262_consumeData(void *p_data);
static SPP_RetVal_t SPP_SERVICES_SX1262_deliverToMailbox(const SPP_Packet_t p_pkt);
static SPP_SERVICE_ConsumerContract_t SPP_SERVICES_SX1262_getConsumerContract();

static void SPP_SERVICES_SX1262_bmpPacket_to_text(SPP_Packet_t *p_pkt, char *dataStr);
static void SPP_SERVICES_SX1262_icmPacket_to_text(SPP_Packet_t *p_pkt, char *dataStr);


/* -----------------------------------------
    STATIC FUNCTIONS IMPLEMENTATION
--------------------------------------------*/
static SPP_RetVal_t SPP_SERVICES_SX1262_init(void){
    SPP_RetVal_t ret = K_SPP_OK;
    SX1262_InitInterrupts();

    return ret;
}

static SPP_RetVal_t SPP_SERVICES_SX1262_deliverToMailbox(const SPP_Packet_t p_pqt){

    if (mailboxCount < K_SX1262_MAILBOX_SIZE){
        mailboxData[mailboxTail] = *p_pqt;
        mailboxTail = (spp_uint8_t)((mailboxTail + 1U) % K_SX1262_MAILBOX_SIZE); // suma circular
        mailboxCount++; // usaremos mailboxCount para saber cuantos paquetes tendremos que enviar luego
        return K_SPP_OK;
    }
    else{
        return K_SPP_ERROR;
    }
}

// HAY QUE CAMBIAR ESTO
static SPP_RetVal_t SPP_SERVICES_SX1262_consumeData(void *p_data){
    char sensorsStr[256];
    char strBMP390[64];
    char strICM20948[128];

    while (mailboxCount > 0) {
        SPP_Packet_t packet = mailboxData[mailboxHead];

        sprintf(strBMP390, "BMP390:-");
        sprintf(strICM20948, "ICM20948:-");

        spp_uint8_t packetAPID = packet.primaryHeader.apid;
        if (packetAPID == K_BMP390_SERVICE_APID){
            SPP_SERVICES_SX1262_bmpPacket_to_text(&packet, strBMP390);
        }
        else if(packetAPID == K_ICM20948_SERVICE_APID){
            SPP_SERVICES_SX1262_icmPacket_to_text(&packet, strICM20948);
        }

        sprintf(sensorsStr, "%s,%s,", strBMP390, strICM20948);

        SPP_SERVICES_SX1262_send_frame(s_frame_number, sensorsStr);
        s_frame_number++;

        mailboxHead = (mailboxHead + 1U) % K_SX1262_MAILBOX_SIZE;
        mailboxCount--;
    }

    return K_SPP_OK;
}

static SPP_SERVICE_ConsumerContract_t SPP_SERVICES_SX1262_getConsumerContract(){
    return &sx1262Contract;
}

SPP_RetVal_t SX1262_Transmit(void *p_handle, spp_uint8_t *p_data, spp_uint8_t length)
{
    SPP_RetVal_t ret;

    // Secuencia de configuración base y radio
    ret = SX1262_SetStandby(p_handle);
    if (ret != K_SPP_OK) return ret;

    ret = SX1262_SetPacketType(p_handle);
    if (ret != K_SPP_OK) return ret;

    ret = SX1262_SetRfFrequency(p_handle);
    if (ret != K_SPP_OK) return ret;

    ret = SX1262_SetPaConfig(p_handle);
    if (ret != K_SPP_OK) return ret;

    ret = SX1262_CalibrateImage(p_handle);
    if (ret != K_SPP_OK) return ret;

    ret = SX1262_SetTxParams(p_handle);
    if (ret != K_SPP_OK) return ret;

    // Gestión de memoria
    ret = SX1262_SetBufferBaseAddress(p_handle);
    if (ret != K_SPP_OK) return ret;

    ret = SX1262_WriteBuffer(p_handle, p_data, length);
    if (ret != K_SPP_OK) return ret;

    // Parámetros de modulación y paquete LoRa
    // SF7, BW 125kHz, CR 4/5, LDRO OFF
    ret = SX1262_SetModulationParams(p_handle, 0x07, 0x04, 0x01, 0x00);
    if (ret != K_SPP_OK) return ret;

    // Preambulo = 8 simbolos, Paquete variable, Longitud del String, CRC ON
    ret = SX1262_SetPacketParams(p_handle, 0x0008, length);
    if (ret != K_SPP_OK) return ret;

    // Configuración de Interrupciones en DIO1 (TxDone + Timeout)
    ret = SX1262_SetDioIrqParams(p_handle, 0x0201, 0x0201, 0x0000, 0x0000);
    if (ret != K_SPP_OK) return ret;

    // Limpiar bandera
    g_txDoneFlag = SPP_FALSE;

    // Ejecutar transmision (SetTx con 1 seg timeout: 0x00FA00)
    ret = SX1262_SetTx(p_handle, 0x00FA00);
    if (ret != K_SPP_OK) return ret;

    // Esperar a que la ISR active la bandera de finalización
    ret = SX1262_WaitForTxDone(p_handle);
    
    return ret;
}

// ESTA FUNCION NO SE USARA
static void SPP_SERVICES_SX1262_bmpPacket_to_text(SPP_Packet_t *p_pkt, char *dataStr){
    float *payload = (float *)p_pkt->payload;

    float altitude = payload[0];
    float pressure = payload[1];
    float temperature = payload[2];

    sprintf(dataStr, "BMP390:alt=%.1f|P=%.1f|T=%.2f", altitude, pressure, temperature);
    
    return;
}

// ESTA FUNCION NO SE USARA
static void SPP_SERVICES_SX1262_icmPacket_to_text(SPP_Packet_t *p_pkt, char *dataStr){
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

    sprintf(dataStr, "ICM20948:ax=%.2f|ay=%.2f|az=%.2f|gx=%.2f|gy=%.2f|gz=%.2f|mx=%.2f|my=%.2f|mz=%.2f", ax, ay, az, gx, gy, gz, mx, my, mz);

    return;
}

/* -----------------------------------------
    STATE CHANGES
--------------------------------------------*/
SPP_RetVal_t SX1262_SetStandby(void *p_handle)
{
    spp_uint8_t buffer[2];
    buffer[0] = SX1262_OPCODE_SET_STANDBY; // Opcode SetStandby
    buffer[1] = 0x00; // STDBY_RC (sin TXCO)

    SPP_RetVal_t ret = SX1262_WaitBusy();
    if (ret != K_SPP_OK)
        return ret;
    return SPP_HAL_SPI_transmit(p_handle, buffer, 2);
}

SPP_RetVal_t SX1262_SetTx(void *p_handle, spp_uint32_t timeout)
{
    // SetTx requiere 1 byte de Opcode y 3 bytes de parámetros
    spp_uint8_t buffer[4];

    buffer[0] = SX1262_OPCODE_SET_TX;
    buffer[1] = (spp_uint8_t)((timeout >> 16) & 0xFF);
    buffer[2] = (spp_uint8_t)((timeout >> 8) & 0xFF);
    buffer[3] = (spp_uint8_t)(timeout & 0xFF);

    SPP_RetVal_t ret = SX1262_WaitBusy();
    if (ret != K_SPP_OK)
        return ret;

    ret = SPP_HAL_SPI_transmit(p_handle, buffer, 4);

    return ret;
}


/* -----------------------------------------
    BASIC PARAMETERS
--------------------------------------------*/
SPP_RetVal_t SX1262_SetPacketType(void *p_handle)
{
    // LoRa
    spp_uint8_t buffer[2];
    buffer[0] = SX1262_OPCODE_SET_PACKET_TYPE;
    buffer[1] = 0x01; // Protocolo LoRa

    SPP_RetVal_t ret = SX1262_WaitBusy();
    if (ret != K_SPP_OK)
        return ret;
    return SPP_HAL_SPI_transmit(p_handle, buffer, 2);
}

SPP_RetVal_t SX1262_SetRfFrequency(void *p_handle)
{
    spp_uint8_t buffer[5];
    buffer[0] = SX1262_OPCODE_SET_RF_FRECUENCY;
    buffer[1] = 0x36; // 868 MHz
    buffer[2] = 0x40;
    buffer[3] = 0x00;
    buffer[4] = 0x00;

    SPP_RetVal_t ret = SX1262_WaitBusy();
    if (ret != K_SPP_OK)
        return ret;
    return SPP_HAL_SPI_transmit(p_handle, buffer, 5);
}

SPP_RetVal_t SX1262_SetPaConfig(void *p_handle)
{
    spp_uint8_t buffer[5];
    buffer[0] = SX1262_OPCODE_SET_PA_CONFIG;
    buffer[1] = 0x02; // paDutyCycle para +14 dBm en SX1262
    buffer[2] = 0x02; // hpMax para +14 dBm en SX1262
    buffer[3] = 0x00; // deviceSel --> SX1262
    buffer[4] = 0x01; // paLut

    SPP_RetVal_t ret = SX1262_WaitBusy();
    if (ret != K_SPP_OK)
        return ret;
    return SPP_HAL_SPI_transmit(p_handle, buffer, 5);
}

SPP_RetVal_t SX1262_CalibrateImage(void *p_handle)
{
    spp_uint8_t buffer[3];
    buffer[0] = SX1262_OPCODE_CALIBRATE_IMAGE;
    buffer[1] = 0xD7; // Calibracion para 868 MHz
    buffer[2] = 0xDB;

    SPP_RetVal_t ret = SX1262_WaitBusy();
    if (ret != K_SPP_OK)
        return ret;

    return SPP_HAL_SPI_transmit(p_handle, buffer, 3);
}

SPP_RetVal_t SX1262_SetTxParams(void *p_handle)
{
    spp_uint8_t buffer[3];
    buffer[0] = SX1262_OPCODE_SET_TX_PARAMS;
    buffer[1] = 0x16; // power: +22 dBm (por las restricciones puestas en SetPaConfig --> bajara a +14 dBm)
    buffer[2] = 0x04; // RampTime: 200 us (valor intermedio)

    SPP_RetVal_t ret = SX1262_WaitBusy();
    if (ret != K_SPP_OK)
        return ret;
    return SPP_HAL_SPI_transmit(p_handle, buffer, 3);
}

/* -----------------------------------------
    MEMORY MANAGEMENT
--------------------------------------------*/
SPP_RetVal_t SX1262_SetBufferBaseAddress(void *p_handle)
{
    spp_uint8_t buffer[3];
    buffer[0] = SX1262_OPCODE_SET_BUFFER_BASE_ADDRESS;
    buffer[1] = 0x00; // txBaseAddress
    buffer[2] = 0x00; // rxBaseAddress

    SPP_RetVal_t ret = SX1262_WaitBusy();
    if (ret != K_SPP_OK)
        return ret;
    return SPP_HAL_SPI_transmit(p_handle, buffer, 3);
}

SPP_RetVal_t SX1262_WriteBuffer(void *p_handle, spp_uint8_t *p_data, spp_uint8_t length)
{
    /* La longitud maxima del buffer es 255 bytes
    Sumando 1 byte de Opcode y 1 byte de Offset, el array máximo es de 257 bytes */
    spp_uint8_t buffer[257];
    
    buffer[0] = SX1262_OPCODE_WRITE_BUFFER;
    buffer[1] = 0x00; // donde se empezará a escribir

    for (spp_uint8_t i = 0; i < length; i++)
    {
        buffer[2 + i] = p_data[i];
    }

    SPP_RetVal_t ret = SX1262_WaitBusy();
    if (ret != K_SPP_OK)
        return ret;
    
    // La longitud total de la transacción SPI es la longitud + 2 bytes de cabecera
    return SPP_HAL_SPI_transmit(p_handle, buffer, length + 2);
}

/* -----------------------------------------
    LoRa Parameters
--------------------------------------------*/
SPP_RetVal_t SX1262_SetModulationParams(void *p_handle, spp_uint8_t sf, spp_uint8_t bw, spp_uint8_t cr, spp_uint8_t ldro)
{
    spp_uint8_t buffer[5];
    buffer[0] = SX1262_OPCODE_SET_MODULATION_PARAMS;
    buffer[1] = sf;
    buffer[2] = bw;
    buffer[3] = cr;
    buffer[4] = ldro;

    SPP_RetVal_t ret = SX1262_WaitBusy();
    if (ret != K_SPP_OK)
        return ret;
    return SPP_HAL_SPI_transmit(p_handle, buffer, 5);
}

SPP_RetVal_t SX1262_SetPacketParams(void *p_handle, spp_uint16_t preambleLength, spp_uint8_t payloadLength)
{
    // El comando requiere 9 bytes de parámetros según la estructura genérica (10 bytes total) (para LoRa solo los 6 primeros)
    spp_uint8_t buffer[10] = {0}; // Inicializa todo a 0
    
    buffer[0] = SX1262_OPCODE_SET_PACKET_PARAMS;
    buffer[1] = (spp_uint8_t)(preambleLength >> 8); // MSB
    buffer[2] = (spp_uint8_t)(preambleLength & 0xFF); // LSB
    buffer[3] = 0x00; // Longitud de paquete variable (explicit header)
    buffer[4] = payloadLength; // Longitud exacta de tu String en bytes
    buffer[5] = 0x01; // CRC ON
    buffer[6] = 0x00; // Standard IQ

    SPP_RetVal_t ret = SX1262_WaitBusy();
    if (ret != K_SPP_OK)
        return ret;
    return SPP_HAL_SPI_transmit(p_handle, buffer, 10);
}

/* -----------------------------------------
    Interruption Management
--------------------------------------------*/
SPP_RetVal_t SX1262_SetDioIrqParams(void *p_handle, spp_uint16_t irqMask, spp_uint16_t dio1Mask, spp_uint16_t dio2Mask, spp_uint16_t dio3Mask)
{
    spp_uint8_t buffer[9];
    buffer[0] = SX1262_OPCODE_SET_DIO_IRQ_PARAMS;
    buffer[1] = (spp_uint8_t)(irqMask >> 8);
    buffer[2] = (spp_uint8_t)(irqMask & 0xFF);
    buffer[3] = (spp_uint8_t)(dio1Mask >> 8);
    buffer[4] = (spp_uint8_t)(dio1Mask & 0xFF);
    buffer[5] = (spp_uint8_t)(dio2Mask >> 8);
    buffer[6] = (spp_uint8_t)(dio2Mask & 0xFF);
    buffer[7] = (spp_uint8_t)(dio3Mask >> 8);
    buffer[8] = (spp_uint8_t)(dio3Mask & 0xFF);

    SX1262_WaitBusy();
    return SPP_HAL_SPI_transmit(p_handle, buffer, 9);
}

SPP_RetVal_t SX1262_ClearIrqStatus(void *p_handle, spp_uint16_t clearMask)
{
    spp_uint8_t buffer[3];
    buffer[0] = SX1262_OPCODE_CLEAR_IRQ_STATUS;
    buffer[1] = (spp_uint8_t)(clearMask >> 8);
    buffer[2] = (spp_uint8_t)(clearMask & 0xFF);

    SPP_RetVal_t ret = SX1262_WaitBusy();
    if (ret != K_SPP_OK)
        return ret;

    return SPP_HAL_SPI_transmit(p_handle, buffer, 3);
}

void SX1262_InitInterrupts(void)
{
    // Configura el pin como entrada con interrupción por flanco de subida
    SPP_HAL_GPIO_configInterrupt(SX126X_DIO1_PIN, 1, 0); 
    
    // Registra el contexto para que la ISR modifique g_txDoneFlag
    SPP_HAL_GPIO_registerIsr(SX126X_DIO1_PIN, &g_dio1Ctx);
}

SPP_RetVal_t SX1262_WaitForTxDone(void *p_handle)
{
    const spp_uint32_t TIMEOUT_MAX_MS = 2000; // ms 
    spp_uint32_t startTime = SPP_HAL_TIME_getMs();

    // Bucle de espera consultando la bandera modificada por la ISR
    while (g_txDoneFlag == SPP_FALSE)
    {
        if ((SPP_HAL_TIME_getMs() - startTime) >= TIMEOUT_MAX_MS)
        {
            return K_ERROR; 
        }
    }

    // Restablece la bandera lógica para la próxima transmisión
    g_txDoneFlag = SPP_FALSE;

    // Limpia la bandera física en el registro interno del SX1262 enviando 0x0201
    return SX1262_ClearIrqStatus(p_handle, 0x0201);
}

/* -----------------------------------------
    AUXILIAR FUNCTION (Espera a que el chip acabe cualquier operacion que esté haciendo)
--------------------------------------------*/
SPP_RetVal_t SX1262_WaitBusy(void)
{
    const spp_uint32_t TIMEOUT_MAX_MS = 1000; // en milisegundos
    
    spp_uint32_t startTime = SPP_HAL_TIME_getMs();

    while (SPP_HAL_GPIO_read(SX1262_BUSY_PIN) == 1) // esta funcion en el HAL no existe, hay que crearla
    {
        // Verifica si se ha superado el tiempo máximo de espera
        if ((SPP_HAL_TIME_getMs() - startTime) >= TIMEOUT_MAX_MS)
        {
            return K_ERROR;
        }
    }

    return K_SPP_OK;
}