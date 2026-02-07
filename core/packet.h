// Basado en Space Packet Protocol (SPP) -> división: primary header + secondary header + payload + crc
// Escalable: en V2 por ejemplo se puede meter una flag para que el secondary header sea opcional o añadir más campos sin cambiar la estructura general
#include "types.h"

#define SPP_PKT_VERSION 1
#define SPP_PKT_PAYLOAD_MAX 48

typedef struct
{
    spp_uint8_t version;
    spp_uint16_t apid;
    spp_uint16_t seq;
    spp_uint16_t payload_len;

}spp_packet_primary_t;

typedef struct
{
    spp_uint32_t timestamp_ms;
    spp_uint8_t drop_counter;

}spp_packet_secondary_t;

typedef struct
{
    spp_packet_primary_t primary_header;
    spp_packet_secondary_t secondary_header;
    spp_uint8_t payload[SPP_PKT_PAYLOAD_MAX];
    spp_uint16_t crc;
}spp_packet_t;
