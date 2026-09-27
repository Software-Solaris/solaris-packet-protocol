/**
 * @file sx1262.c
 * @brief sx1262 module telemetry + SPP service implementation.
 */

#include "spp/services/sx1262/sx1262.h"

#include <stdio.h>
#include <string.h>
#include "spp/hal/spi/spi.h"
#include "spp/hal/gpio/gpio.h"
#include "spp/hal/time/time.h"
#include "spp/services/kpid.h"

/* -----------------------------------------
    STRUCTS
--------------------------------------------*/
typedef struct {
    void *p_spi;
    spp_uint32_t frame_number;
} SX1262_Ctx_t;

static SX1262_Ctx_t s_sx1262Ctx = {
    .p_spi = NULL,
    .frame_number = 1
};

/* -----------------------------------------
    VARIABLES
--------------------------------------------*/
static SPP_Packet_t mailboxData[K_SX1262_MAILBOX_SIZE] = {0};
static spp_uint8_t mailboxHead = 0;
static spp_uint8_t mailboxTail = 0;
static spp_uint8_t mailboxCount = 0;


volatile spp_bool_t g_txDoneFlag = false; // volatile to avoid optimizations
SPP_GpioIsrCtx_t g_dio1Ctx = { .p_flag = &g_txDoneFlag };


/* -----------------------------------------
    STATIC DECLARATIONS
--------------------------------------------*/
static SPP_RetVal_t SPP_SERVICES_SX1262_init(void);
static SPP_RetVal_t SPP_SERVICES_SX1262_consumeData(void*);
static SPP_RetVal_t SPP_SERVICES_SX1262_deliverToMailbox(const SPP_Packet_t);

static SPP_RetVal_t SX1262_WaitBusy(void);
static SPP_RetVal_t SX1262_SetStandby(void*);
static SPP_RetVal_t SX1262_SetPacketType(void*);
static SPP_RetVal_t SX1262_SetRfFrequency(void*);
static SPP_RetVal_t SX1262_SetPaConfig(void*);
static SPP_RetVal_t SX1262_CalibrateImage(void*);
static SPP_RetVal_t SX1262_SetTxParams(void*);
static SPP_RetVal_t SX1262_SetBufferBaseAddress(void*);
static SPP_RetVal_t SX1262_SetModulationParams(void*, spp_uint8_t, spp_uint8_t, spp_uint8_t, spp_uint8_t);
static SPP_RetVal_t SX1262_SetPacketParams(void*, spp_uint16_t, spp_uint8_t);
static SPP_RetVal_t SX1262_SetDioIrqParams(void*, spp_uint16_t, spp_uint16_t, spp_uint16_t, spp_uint16_t);
static SPP_RetVal_t SX1262_ClearIrqStatus(void*, spp_uint16_t);
static void         SX1262_InitInterrupts(void);
static SPP_RetVal_t SX1262_WaitForTxDone(void*);
static SPP_RetVal_t SX1262_WriteBuffer(void*, spp_uint8_t*, spp_uint8_t);
static SPP_RetVal_t SX1262_SetTx(void*, spp_uint32_t);
static SPP_RetVal_t SX1262_WriteRegister(void*, spp_uint16_t, spp_uint8_t*, spp_uint8_t);
static SPP_RetVal_t SX1262_Transmit(void*, spp_uint8_t*, spp_uint8_t);

static SPP_SERVICE_ConsumerContract_t sx1262Contract = {
    .priority = 3U,
    .p_nameConsumer = "sx1262",
    .tiemoutMs = K_SX1262_TASK_TIMEOUT_MS,
    .init = SPP_SERVICES_SX1262_init,
    .deliverToMailbox = SPP_SERVICES_SX1262_deliverToMailbox,
    .consumeData = SPP_SERVICES_SX1262_consumeData,
};

/* -----------------------------------------
    PUBLIC FUNCTIONS
--------------------------------------------*/
const SPP_SERVICE_ConsumerContract_t *SPP_SERVICES_SX1262_getConsumerContract(void) {
    return &sx1262Contract;
}


/* -----------------------------------------
    STATIC FUNCTIONS IMPLEMENTATION
--------------------------------------------*/
static SPP_RetVal_t SPP_SERVICES_SX1262_init(void){
    s_sx1262Ctx.p_spi = SPP_HAL_SPI_getHandle(K_SX1262_SPI); 
    if (s_sx1262Ctx.p_spi == NULL) return K_SPP_ERROR;

    (void)SPP_HAL_SPI_deviceInit(s_sx1262Ctx.p_spi);

    void *p_spi = s_sx1262Ctx.p_spi;

    // Hardware Reset
    SPP_HAL_GPIO_write(SX126X_NRESET_PIN, 0); // this function in the HAL does not exist, it should be created!!
    SPP_HAL_TIME_delayMs(1);
    SPP_HAL_GPIO_write(SX126X_NRESET_PIN, 1); // this function in the HAL does not exist, it should be created!!
    SPP_HAL_TIME_delayMs(5);


    SPP_RetVal_t ret = K_SPP_OK;
    SX1262_InitInterrupts();

    // Sequence of basic and radio configuration
    ret = SX1262_SetStandby(p_spi);
    if (ret != K_SPP_OK) return ret;

    ret = SX1262_SetPacketType(p_spi);
    if (ret != K_SPP_OK) return ret;

    ret = SX1262_SetRfFrequency(p_spi);
    if (ret != K_SPP_OK) return ret;

    ret = SX1262_SetPaConfig(p_spi);
    if (ret != K_SPP_OK) return ret;

    ret = SX1262_CalibrateImage(p_spi);
    if (ret != K_SPP_OK) return ret;

    ret = SX1262_SetTxParams(p_spi);
    if (ret != K_SPP_OK) return ret;

    ret = SX1262_SetBufferBaseAddress(p_spi);
    if (ret != K_SPP_OK) return ret;

    // Modulation and Lora packet parameters
    // SF7, BW 125kHz, CR 4/5, LDRO OFF
    ret = SX1262_SetModulationParams(p_spi, 0x07, 0x04, 0x01, 0x00);
    if (ret != K_SPP_OK) return ret;

    // Sync Word Configuration to Private Network (0x1424)
    spp_uint8_t syncWord[2] = {0x14, 0x24};
    ret = SX1262_WriteRegister(p_spi, 0x0740, syncWord, 2);
    if (ret != K_SPP_OK) return ret;

    // Interrupt configuration in DIO1 (TxDone + Timeout)
    ret = SX1262_SetDioIrqParams(p_spi, 0x0201, 0x0201, 0x0000, 0x0000);
    
    return ret;
}

static SPP_RetVal_t SPP_SERVICES_SX1262_deliverToMailbox(const SPP_Packet_t pqt){
    if (mailboxCount < K_SX1262_MAILBOX_SIZE){
        mailboxData[mailboxTail] = pqt;
        mailboxTail = (spp_uint8_t)((mailboxTail + 1U) % K_SX1262_MAILBOX_SIZE);
        mailboxCount++; // we use mailboxCount to know how many packets we have to send later
        return K_SPP_OK;
    }
    return K_SPP_ERROR;
}

static SPP_RetVal_t SPP_SERVICES_SX1262_consumeData(void *p_data){
    if (s_sx1262Ctx.p_spi == NULL) return K_SPP_ERROR;

    while (mailboxCount > 0) {
        SPP_Packet_t packet = mailboxData[mailboxHead];

        spp_uint8_t payloadLength = sizeof(SPP_Packet_t);
        SPP_RetVal_t txStatus = SX1262_Transmit(s_sx1262Ctx.p_spi, (spp_uint8_t*)&packet, payloadLength);

        if (txStatus == K_SPP_OK) {
            s_sx1262Ctx.frame_number++;
        }

        mailboxHead = (mailboxHead + 1U) % K_SX1262_MAILBOX_SIZE;
        mailboxCount--;
    }
    return K_SPP_OK;
}


static SPP_RetVal_t SX1262_Transmit(void *p_handle, spp_uint8_t *p_data, spp_uint8_t length)
{
    SPP_RetVal_t ret;

    ret = SX1262_WriteBuffer(p_handle, p_data, length);
    if (ret != K_SPP_OK) return ret;

    // Preamble = 8 symbols, Variable packet, packet length, CRC ON 
    ret = SX1262_SetPacketParams(p_handle, 0x0008, length);
    if (ret != K_SPP_OK) return ret;

    // Clear flag
    g_txDoneFlag = false;

    // Signal transmission (SetTx with 1 sec timeout: 0x00FA00)
    ret = SX1262_SetTx(p_handle, 0x00FA00);
    if (ret != K_SPP_OK) return ret;

    // Wait for TX completion interrupt flag
    return SX1262_WaitForTxDone(p_handle);
}


/* -----------------------------------------
    STATE CHANGES
--------------------------------------------*/
static SPP_RetVal_t SX1262_SetStandby(void *p_handle)
{
    spp_uint8_t buffer[2];
    buffer[0] = SX1262_OPCODE_SET_STANDBY; // Opcode SetStandby
    buffer[1] = 0x00; // STDBY_RC (without TXCO)

    SPP_RetVal_t ret = SX1262_WaitBusy();
    if (ret != K_SPP_OK)
        return ret;
    return SPP_HAL_SPI_transmit(p_handle, buffer, 2);
}

static SPP_RetVal_t SX1262_SetTx(void *p_handle, spp_uint32_t timeout)
{
    // SetTx requires 1 byte of Opcode and 3 bytes of parameters
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
static SPP_RetVal_t SX1262_SetPacketType(void *p_handle)
{
    // LoRa
    spp_uint8_t buffer[2];
    buffer[0] = SX1262_OPCODE_SET_PACKET_TYPE;
    buffer[1] = 0x01; // Lora Protocol

    SPP_RetVal_t ret = SX1262_WaitBusy();
    if (ret != K_SPP_OK)
        return ret;
    return SPP_HAL_SPI_transmit(p_handle, buffer, 2);
}

static SPP_RetVal_t SX1262_SetRfFrequency(void *p_handle)
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

static SPP_RetVal_t SX1262_SetPaConfig(void *p_handle)
{
    spp_uint8_t buffer[5];
    buffer[0] = SX1262_OPCODE_SET_PA_CONFIG;
    buffer[1] = 0x02; // paDutyCycle to +14 dBm in SX1262
    buffer[2] = 0x02; // hpMax to +14 dBm in SX1262
    buffer[3] = 0x00; // deviceSel --> SX1262
    buffer[4] = 0x01; // paLut

    SPP_RetVal_t ret = SX1262_WaitBusy();
    if (ret != K_SPP_OK)
        return ret;
    return SPP_HAL_SPI_transmit(p_handle, buffer, 5);
}

static SPP_RetVal_t SX1262_CalibrateImage(void *p_handle)
{
    spp_uint8_t buffer[3];
    buffer[0] = SX1262_OPCODE_CALIBRATE_IMAGE;
    buffer[1] = 0xD7; // Calibration to 868 MHz (SX1262 is calibrated to 918MHZ (default))
    buffer[2] = 0xDB;

    SPP_RetVal_t ret = SX1262_WaitBusy();
    if (ret != K_SPP_OK)
        return ret;

    return SPP_HAL_SPI_transmit(p_handle, buffer, 3);
}

static SPP_RetVal_t SX1262_SetTxParams(void *p_handle)
{
    spp_uint8_t buffer[3];
    buffer[0] = SX1262_OPCODE_SET_TX_PARAMS;
    buffer[1] = 0x16; // power: +22 dBm (due to restrictions in SetPaConfig --> it will go down to +14 dBm)
    buffer[2] = 0x04; // RampTime: 200 us (intermediate value)

    SPP_RetVal_t ret = SX1262_WaitBusy();
    if (ret != K_SPP_OK)
        return ret;
    return SPP_HAL_SPI_transmit(p_handle, buffer, 3);
}

/* -----------------------------------------
    MEMORY MANAGEMENT
--------------------------------------------*/
static SPP_RetVal_t SX1262_SetBufferBaseAddress(void *p_handle)
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

static SPP_RetVal_t SX1262_WriteBuffer(void *p_handle, spp_uint8_t *p_data, spp_uint8_t length)
{
    /* The maximum length of the buffer is 255 bytes
    Adding 1 byte of Opcode and 1 byte of Offset, the array maximum length is 257 bytes */
    spp_uint8_t buffer[257];
    
    buffer[0] = SX1262_OPCODE_WRITE_BUFFER;
    buffer[1] = 0x00; // the address where we start to write

    for (spp_uint8_t i = 0; i < length; i++)
    {
        buffer[2 + i] = p_data[i];
    }

    SPP_RetVal_t ret = SX1262_WaitBusy();
    if (ret != K_SPP_OK)
        return ret;
    
    // The total length of the SPI transaction is the length + 2 bytes of header
    return SPP_HAL_SPI_transmit(p_handle, buffer, length + 2);
}

/* -----------------------------------------
    LoRa Parameters
--------------------------------------------*/
static SPP_RetVal_t SX1262_SetModulationParams(void *p_handle, spp_uint8_t sf, spp_uint8_t bw, spp_uint8_t cr, spp_uint8_t ldro)
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

static SPP_RetVal_t SX1262_SetPacketParams(void *p_handle, spp_uint16_t preambleLength, spp_uint8_t payloadLength)
{
    // The command requires 9 bytes of parameters according the general structure (10 bytes total) (for LoRa only the 6 firsts are used)
    spp_uint8_t buffer[10] = {0};
    
    buffer[0] = SX1262_OPCODE_SET_PACKET_PARAMS;
    buffer[1] = (spp_uint8_t)(preambleLength >> 8); // MSB
    buffer[2] = (spp_uint8_t)(preambleLength & 0xFF); // LSB
    buffer[3] = 0x00; // Variable packet length (explicit header)
    buffer[4] = payloadLength; // Exact length of the packet in bytes
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
static SPP_RetVal_t SX1262_SetDioIrqParams(void *p_handle, spp_uint16_t irqMask, spp_uint16_t dio1Mask, spp_uint16_t dio2Mask, spp_uint16_t dio3Mask)
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

static SPP_RetVal_t SX1262_ClearIrqStatus(void *p_handle, spp_uint16_t clearMask)
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

static void SX1262_InitInterrupts(void)
{
    // This instruction configures the pin as an input with interruption by rising edge
    SPP_HAL_GPIO_configInterrupt(SX1262_DIO1_PIN, 1, 0); 
    
    // This instruction registers the context to the ISR can modify g_txDoneFlag
    SPP_HAL_GPIO_registerIsr(SX1262_DIO1_PIN, &g_dio1Ctx);
}

static SPP_RetVal_t SX1262_WaitForTxDone(void *p_handle)
{
    const spp_uint32_t TIMEOUT_MAX_MS = 2000; // ms 
    spp_uint32_t startTime = SPP_HAL_TIME_getTimeMs();

    // Waiting loop waiting for the flag of the ISR
    while (g_txDoneFlag == false)
    {
        if ((SPP_HAL_TIME_getTimeMs() - startTime) >= TIMEOUT_MAX_MS)
        {
            return K_SPP_ERROR; 
        }
    }

    // Reset the flag for the next transmission
    g_txDoneFlag = false;

    // Clear the flag in the internal register of SX1262 sending 0x0201
    return SX1262_ClearIrqStatus(p_handle, 0x0201);
}

static SPP_RetVal_t SX1262_WriteRegister(void *p_handle, spp_uint16_t address, spp_uint8_t *p_data, spp_uint8_t length)
{
    // Maximum length: opcode + address + data
    spp_uint8_t buffer[16]; 
    
    buffer[0] = 0x0D; // Opcode WriteRegister
    buffer[1] = (spp_uint8_t)(address >> 8);   // MSB
    buffer[2] = (spp_uint8_t)(address & 0xFF); // LSB
    
    for (spp_uint8_t i = 0; i < length; i++)
    {
        buffer[3 + i] = p_data[i];
    }

    SPP_RetVal_t ret = SX1262_WaitBusy();
    if (ret != K_SPP_OK) return ret;

    return SPP_HAL_SPI_transmit(p_handle, buffer, length + 3);
}

/* -----------------------------------------
    AUXILIAR FUNCTION (Wait the chip to finish any operation that is running)
--------------------------------------------*/
static SPP_RetVal_t SX1262_WaitBusy(void)
{
    const spp_uint32_t TIMEOUT_MAX_MS = 1000; // in miliseconds
    
    spp_uint32_t startTime = SPP_HAL_TIME_getTimeMs();

    while (SPP_HAL_GPIO_read(SX1262_BUSY_PIN) == 1) // this function in the HAL does not exist, it should be created!!
    {
        // Check if the maximum time of waiting was reached
        if ((SPP_HAL_TIME_getTimeMs() - startTime) >= TIMEOUT_MAX_MS)
        {
            return K_SPP_ERROR;
        }
    }

    return K_SPP_OK;
}
