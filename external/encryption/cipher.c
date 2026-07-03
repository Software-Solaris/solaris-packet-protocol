/**
 * @file cipher.c
 * @brief SPP packet AES-128-GCM cipher context implementation.
 *
 * Encrypts a raw SPP packet buffer in-place, producing the output frame:
 *   [ nonce (12 B) | ciphertext (sizeof SPP_Packet_t) | tag (16 B) ]
 */

/* ----------------------------------------------------------------
 * INCLUDES
 * ---------------------------------------------------------------- */

#include "external/encryption/cipher.h"
#include "external/encryption/encryption.h"

#include <string.h>

/* ----------------------------------------------------------------
 * VARIABLES
 * ---------------------------------------------------------------- */

static const spp_uint8_t s_key[K_SPP_CIPHER_KEY_LEN] = K_SPP_CIPHER_KEY;

/* ----------------------------------------------------------------
 * STATIC FUNCTIONS
 * ---------------------------------------------------------------- */

/**
 * @brief Derive the 12-byte GCM nonce from an SPP packet header.
 *
 * @param[in]  p_packet  Packet to derive nonce from.
 * @param[out] p_nonce   12-byte output buffer.
 */
static void s_buildNonce(const SPP_Packet_t *p_packet,
                         spp_uint8_t p_nonce[K_SPP_CIPHER_NONCE_LEN])
{
    memset(p_nonce, 0, K_SPP_CIPHER_NONCE_LEN);

    /* Bytes 0-3: timestampMs little-endian */
    p_nonce[0] = (spp_uint8_t)( p_packet->secondaryHeader.timestampMs        & 0xFFU);
    p_nonce[1] = (spp_uint8_t)((p_packet->secondaryHeader.timestampMs >>  8U) & 0xFFU);
    p_nonce[2] = (spp_uint8_t)((p_packet->secondaryHeader.timestampMs >> 16U) & 0xFFU);
    p_nonce[3] = (spp_uint8_t)((p_packet->secondaryHeader.timestampMs >> 24U) & 0xFFU);

    /* Bytes 4-5: seq little-endian */
    p_nonce[4] = (spp_uint8_t)( p_packet->primaryHeader.seq        & 0xFFU);
    p_nonce[5] = (spp_uint8_t)((p_packet->primaryHeader.seq >>  8U) & 0xFFU);

    /* Bytes 6-11: reserved, already zeroed */
}

/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */

SPP_RetVal_t SPP_EXTERNAL_ENCRYPTION_CYPHER_encryptPacket(spp_uint8_t *p_buffer, spp_uint16_t *p_outSize)
{
    if (p_buffer == NULL || p_outSize == NULL)
    {
        return K_SPP_ERROR_NULL_POINTER;
    }

    spp_uint8_t nonce[K_SPP_CIPHER_NONCE_LEN];
    spp_uint8_t packet_copy[sizeof(SPP_Packet_t)];

    /* Save a copy before we overwrite the buffer */
    memcpy(packet_copy, p_buffer, sizeof(SPP_Packet_t));

    /* Derive nonce from the original packet header fields */
    s_buildNonce((const SPP_Packet_t *)packet_copy, nonce);

    /* Encrypt into buffer at offset NONCE_LEN, leaving room for the nonce prefix.
     * Output layout: p_buffer[ 0..11 ] = nonce (written below)
     *                p_buffer[12..73 ] = ciphertext  (sizeof SPP_Packet_t = 62 B)
     *                p_buffer[74..89 ] = GCM tag     (16 B)                       */
    SPP_EXTERNAL_ENCRYPTION_gcmEncrypt(
        s_key,
        nonce,
        NULL, 0,
        packet_copy,
        sizeof(SPP_Packet_t),
        p_buffer + K_SPP_CIPHER_NONCE_LEN
    );

    /* Prepend nonce at the start of the frame */
    memcpy(p_buffer, nonce, K_SPP_CIPHER_NONCE_LEN);

    *p_outSize = (spp_uint16_t)K_SPP_CIPHER_FRAME_LEN;
    return K_SPP_OK;
}
