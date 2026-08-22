#ifndef SPP_SX1262_H
#define SPP_SX1262_H

#include "spp/core/types.h"
#include "spp/core/returnTypes.h"


/* -----------------------------------------
    DEFINES
--------------------------------------------*/

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

#define SX1262_BUSY_PIN 4 // hay que ver en que PIN estara el BUSY del SX1262
#define SX126X_DIO1_PIN 5 // hay que ver en que PIN estara el DIO1





#endif /* SPP_SX1262_H */