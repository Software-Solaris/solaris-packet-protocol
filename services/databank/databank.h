#ifndef DATABANK_H 
#define DATABANK_H

#include <stdint.h>
#include "core/returntypes.h"
#include "core/macros.h"
#include "core/packet.h"
#include "services/logging/spp_log.h"


typedef struct{
    spp_packet_t *free_packets[DATA_BANK_SIZE];
    uint32_t number_of_free_packets;
}SPP_Databank_t;


retval_t SPP_DATABANK_init(void);
spp_packet_t* SPP_DATABANK_getPacket(void);
retval_t SPP_DATABANK_returnPacket(spp_packet_t* packet);

#endif