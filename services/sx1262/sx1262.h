#ifndef SPP_SERVICES_SX1262_H
#define SPP_SERVICES_SX1262_H

#include "spp/core/returnTypes.h"
#include "spp/core/packet.h"
#include "spp/services/service.h"

/* -----------------------------------------
    DEFINES
--------------------------------------------*/
#define K_SX1262_MAILBOX_SIZE    100U
#define K_SX1262_TASK_TIMEOUT_MS 1000U

#define SX1262_OPCODE_SET_STANDBY             0x80
#define SX1262_OPCODE_SET_TX                  0x83
#define SX1262_OPCODE_SET_PACKET_TYPE         0x8A
#define SX1262_OPCODE_SET_RF_FRECUENCY        0x86
#define SX1262_OPCODE_SET_PA_CONFIG           0x95
#define SX1262_OPCODE_SET_TX_PARAMS           0x8E
#define SX1262_OPCODE_SET_BUFFER_BASE_ADDRESS 0x8F
#define SX1262_OPCODE_WRITE_BUFFER            0x0E
#define SX1262_OPCODE_SET_MODULATION_PARAMS   0x8B
#define SX1262_OPCODE_SET_PACKET_PARAMS       0x8C
#define SX1262_OPCODE_SET_DIO_IRQ_PARAMS      0x08
#define SX1262_OPCODE_CLEAR_IRQ_STATUS        0x02
#define SX1262_OPCODE_CALIBRATE_IMAGE         0x98
#define SX1262_OPCODE_WRITE_REGISTER          0x0D
#define K_SX1262_TOTAL_PACKET_SIZE            64U
#define K_SX1262_MAX_PACKETS_TO_TRANSMIT      50U
#define SX1262_MAX_BUSY_TIMEOUT_MS            1000U
#define SX1262_MAX_DONE_TIMEOUT_MS            2000U

#define SX1262_BUSY_PIN   5
#define SX1262_DIO1_PIN   6
#define SX126X_NRESET_PIN 7
#define K_SX1262_SPI      2U

/* Debug */
#define SX1262_OPCODE_GET_IRQ_STATUS 0x12
#define SX1262_IRQ_TX_DONE           0x0001U
#define SX1262_IRQ_TIMEOUT           0x0200U

#define SX1262_OPCODE_SET_REGULATOR_MODE    0x96U
#define SX1262_REGULATOR_DC_DC              0x01U
#define SX1262_OPCODE_SET_DIO3_AS_TCXO_CTRL 0x97U
#define SX1262_TCXO_VOLTAGE_1V7             0x01U
#define SX1262_TCXO_STARTUP_DELAY           0x000140UL
#define SX1262_OPCODE_CALIBRATE             0x89U
#define SX1262_CALIBRATE_ALL                0x7FU
#define SX1262_OPCODE_CLEAR_DEVICE_ERRORS   0x07U
#define SX1262_OPCODE_GET_DEVICE_ERRORS     0x17U

typedef enum
{
    SX1262_TX_STATE_IDLE,
    SX1262_TX_STATE_WRITE_BUFFER,
    SX1262_TX_STATE_SET_PARAMS,
    SX1262_TX_STATE_SET_TX,
    SX1262_TX_STATE_WAIT_TX_DONE
} SX1262_TxState_t;

typedef struct
{
    void *p_spi;
    spp_uint32_t frame_number;
    SX1262_TxState_t tx_state;
    spp_uint32_t timer_ms;
} SX1262_Ctx_t;

/* -----------------------------------------
    PUBLIC DECLARATIONS
--------------------------------------------*/
const SPP_SERVICE_ConsumerContract_t *SPP_SERVICES_SX1262_getConsumerContract(void);


#endif /* SPP_SERVICES_SX1262_H */
