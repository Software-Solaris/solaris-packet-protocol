/**
 * @file packet.h
 * @brief SPP packet definitions based on the Space Packet Protocol.
 *
 * Structure: primary header + secondary header + payload + CRC.
 * Scalable: in V2 the secondary header can be made optional, etc.
 */

#ifndef SPP_CORE_PACKET_H
#define SPP_CORE_PACKET_H

#ifdef __cplusplus
extern "C"
{
#endif

    /* ---------------------------------------------------------------- */
    /*  Includes                                                        */
    /* ---------------------------------------------------------------- */

#include "spp/core/types.h"

/* ---------------------------------------------------------------- */
/*  Constants                                                       */
/* ---------------------------------------------------------------- */

/** @brief Current packet format version. */
#define SPP_PKT_VERSION 1

/** @brief Maximum payload size in bytes. */
#define SPP_PKT_PAYLOAD_MAX 48

    /* ---------------------------------------------------------------- */
    /*  Types                                                           */
    /* ---------------------------------------------------------------- */

    /** @brief Primary packet header (identification and length). */
    typedef struct
    {
        spp_uint8_t version;
        spp_uint16_t apid;
        spp_uint16_t seq;
        spp_uint16_t payload_len;
    } spp_packet_primary_t;

    /** @brief Secondary packet header (timing and diagnostics). */
    typedef struct
    {
        spp_uint32_t timestamp_ms;
        spp_uint8_t drop_counter;
    } spp_packet_secondary_t;

    /** @brief Complete SPP packet. */
    typedef struct
    {
        spp_packet_primary_t primary_header;
        spp_packet_secondary_t secondary_header;
        spp_uint8_t payload[SPP_PKT_PAYLOAD_MAX];
        spp_uint16_t crc;
    } spp_packet_t;

#ifdef __cplusplus
}
#endif

#endif /* SPP_CORE_PACKET_H */
