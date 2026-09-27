#ifndef SPP_SERVICES_SX1262_H
#define SPP_SERVICES_SX1262_H

#include "spp/core/returnTypes.h"
#include "spp/core/packet.h"
#include "spp/services/service.h"

/* -----------------------------------------
    DEFINES
--------------------------------------------*/
#define K_SX1262_MAILBOX_SIZE       100U
#define K_SX1262_TASK_TIMEOUT_MS    1000U

#define SX1262_OPCODE_SET_STANDBY               0x80
#define SX1262_OPCODE_SET_TX                    0x83
#define SX1262_OPCODE_SET_PACKET_TYPE           0x8A
#define SX1262_OPCODE_SET_RF_FRECUENCY          0x86
#define SX1262_OPCODE_SET_PA_CONFIG             0x95
#define SX1262_OPCODE_SET_TX_PARAMS             0x8E
#define SX1262_OPCODE_SET_BUFFER_BASE_ADDRESS   0x8F
#define SX1262_OPCODE_WRITE_BUFFER              0x0E
#define SX1262_OPCODE_SET_MODULATION_PARAMS     0x8B
#define SX1262_OPCODE_SET_PACKET_PARAMS         0x8C
#define SX1262_OPCODE_SET_DIO_IRQ_PARAMS        0x08
#define SX1262_OPCODE_CLEAR_IRQ_STATUS          0x02
#define SX1262_OPCODE_CALIBRATE_IMAGE           0x98
#define SX1262_OPCODE_WRITE_REGISTER            0x0D

#define SX1262_BUSY_PIN   4 // SEARCH THE CORRECT PIN WHERE BUSY WILL BE CONNECTED IS NEEDED!!!!!
#define SX1262_DIO1_PIN   5 // SEARCH THE CORRECT PIN WHERE DIO1 WILL BE CONNECTED IS NEEDED!!!!!
#define SX126X_NRESET_PIN 6 // SEARCH THE CORRECT PIN WHERE NRESET WILL BE CONNECTED IS NEEDED!!!!!
#define K_SX1262_SPI      1U

/* -----------------------------------------
    PUBLIC DECLARATIONS
--------------------------------------------*/
const SPP_SERVICE_ConsumerContract_t *SPP_SERVICES_SX1262_getConsumerContract(void);


#endif /* SPP_SERVICES_SX1262_H */
