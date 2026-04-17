/**
 * @file packet.h
 * @brief Solaris Packet Protocol (SPP) packet structure definitions.
 *
 * Defines the on-wire layout of an SPP packet, inspired by the ECSS Space
 * Packet Protocol standard.  Every sensor reading or telemetry message in
 * the system is transported as an @ref SPP_Packet_t.
 *
 * Naming conventions used in this file:
 * - Constants/macros: K_SPP_PKT_*
 * - Types: SPP_Packet*_t
 */

#ifndef SPP_PACKET_H
#define SPP_PACKET_H

#include "spp/core/types.h"

/* ----------------------------------------------------------------
 * Packet constants
 * ---------------------------------------------------------------- */

/** @brief Current SPP protocol version embedded in each packet. */
#define K_SPP_PKT_VERSION     (1U)

/** @brief Maximum payload size in bytes per packet. */
#define K_SPP_PKT_PAYLOAD_MAX (48U)

/* ----------------------------------------------------------------
 * Reserved APIDs
 * ---------------------------------------------------------------- */

/** @brief APID reserved for SPP log message packets. */
#define K_SPP_APID_LOG (0x0001U)

/* ----------------------------------------------------------------
 * Packet header types
 * ---------------------------------------------------------------- */

/**
 * @brief Primary header — routing and framing information.
 */
typedef struct
{
    spp_uint8_t  version;     /**< Protocol version (= K_SPP_PKT_VERSION).        */
    spp_uint16_t apid;        /**< Application Process Identifier.                */
    spp_uint16_t seq;         /**< Packet sequence counter (wraps at UINT16_MAX). */
    spp_uint16_t payloadLen;  /**< Length of the payload field in bytes.          */
} SPP_PacketPrimary_t;

/**
 * @brief Secondary header — timing and reliability metadata.
 */
typedef struct
{
    spp_uint32_t timestampMs; /**< System timestamp at packet creation (ms). */
    spp_uint8_t  dropCounter; /**< Number of packets dropped since last reset. */
} SPP_PacketSecondary_t;

/* ----------------------------------------------------------------
 * Full packet type
 * ---------------------------------------------------------------- */

/**
 * @brief Complete SPP packet.
 *
 * Producers fill @c primaryHeader, @c secondaryHeader, and @c payload, then
 * push the packet into the @c db_flow FIFO.  Consumers pop it, process it,
 * and return it to the databank.
 */
typedef struct
{
    SPP_PacketPrimary_t   primaryHeader;          /**< Routing / framing header.  */
    SPP_PacketSecondary_t secondaryHeader;         /**< Timing / metadata header.  */
    spp_uint8_t           payload[K_SPP_PKT_PAYLOAD_MAX]; /**< Raw payload bytes. */
    spp_uint16_t          crc;                    /**< CRC-16 over the full packet (0 = not computed). */
} SPP_Packet_t;

#endif /* SPP_PACKET_H */
