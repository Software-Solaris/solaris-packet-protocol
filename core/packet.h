#ifndef SPP_CORE_PACKET_H
#define SPP_CORE_PACKET_H

// Basado en Space Packet Protocol (SPP) -> primary header + secondary header + payload + crc
// Escalable: en V2 se puede hacer secondary opcional, etc.

#include "types.h"

#define SPP_PKT_VERSION     1
#define SPP_PKT_PAYLOAD_MAX 48

typedef struct
{
    spp_uint8_t  version;
    spp_uint16_t apid;
    spp_uint16_t seq;
    spp_uint16_t payload_len;
} spp_packet_primary_t;

typedef struct
{
    spp_uint32_t timestamp_ms;
    spp_uint8_t  drop_counter;
} spp_packet_secondary_t;

typedef struct
{
    spp_packet_primary_t   primary_header;
    spp_packet_secondary_t secondary_header;
    spp_uint8_t            payload[SPP_PKT_PAYLOAD_MAX];
    spp_uint16_t           crc;
} spp_packet_t;

#endif /* SPP_CORE_PACKET_H */