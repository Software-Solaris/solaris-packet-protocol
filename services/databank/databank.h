#ifndef DATABANK_H 
#define DATABANK_H

#include <stdint.h>
#include "../../core/returntypes.h"
#include "../../core/macros.h"

typedef struct{
    uint8_t *protocol_id;
    uint8_t packet_id;
    uint32_t timestamp;
    uint32_t data;
    uint32_t error_correction;
}spp_packet_t;

typedef struct{
    spp_packet_t *free_packets[DATA_BANK_SIZE];
    uint32_t number_of_free_packets;
}data_pool_t;




retval_t DataBank_Init(void);
spp_packet_t* DataBank_GetPacket(void);
retval_t DataBank_ReturnPacket(spp_packet_t* packet);

#endif