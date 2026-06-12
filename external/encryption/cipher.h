/**
 * @file cipher.h
 * @brief SPP packet AES-128-GCM cipher context.
 *
 * Provides a single function that encrypts a raw SPP packet buffer in-place,
 * prepending the derived nonce and appending the GCM authentication tag.
 * The output frame layout is:
 *
 *   [ nonce (12 B) | ciphertext (sizeof SPP_Packet_t) | tag (16 B) ]
 *
 * The nonce is derived from the packet's timestamp and sequence counter to
 * guarantee uniqueness per transmission. The symmetric key is stored in this
 * header and must match on both ends of the link.
 *
 * Naming conventions used in this file:
 * - Constants/macros: K_SPP_CIPHER_*
 * - Public functions: SPP_CIPHER_*
 * - Pointer params:   p_*
 */

#ifndef SPP_EXTERNAL_CIPHER_H
#define SPP_EXTERNAL_CIPHER_H

#include "spp/core/types.h"
#include "spp/core/packet.h"
#include "spp/core/returnTypes.h"

/* ----------------------------------------------------------------
 * DEFINES
 * ---------------------------------------------------------------- */

#define K_SPP_CIPHER_KEY_LEN   (16U) /**< AES-128 key length in bytes.            */
#define K_SPP_CIPHER_NONCE_LEN (12U) /**< GCM recommended nonce length in bytes.  */
#define K_SPP_CIPHER_TAG_LEN   (16U) /**< GCM authentication tag length in bytes. */

/**
 * @brief Total output frame size after encryption.
 *
 * Layout: nonce | ciphertext | tag.
 * Declare the output buffer with this size:
 * @code
 *     spp_uint8_t buf[K_SPP_CIPHER_FRAME_LEN];
 * @endcode
 */
#define K_SPP_CIPHER_FRAME_LEN (K_SPP_CIPHER_NONCE_LEN + sizeof(SPP_Packet_t) + K_SPP_CIPHER_TAG_LEN)

/**
 * @brief AES-128 symmetric key shared between transmitter and receiver.
 *
 * Replace the placeholder bytes with the actual pre-shared key before
 * deployment. Both ends of the link must use the identical value.
 */
#define K_SPP_CIPHER_KEY \
    { 0x68, 0x6F, 0x6C, 0x79, 0x66, 0x75, 0x63, 0x6B, \
      0x69, 0x6E, 0x67, 0x73, 0x68, 0x69, 0x74, 0x21 }

/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */

/**
 * @brief Encrypt an SPP packet buffer in-place with AES-128-GCM.
 *
 * Reads sizeof(SPP_Packet_t) bytes from @p p_buffer, encrypts them, and
 * overwrites @p p_buffer with the output frame:
 *
 *   [ nonce (12 B) | ciphertext (sizeof SPP_Packet_t) | tag (16 B) ]
 *
 * The nonce is derived from the packet header:
 *   - Bytes  0-3 : timestampMs (little-endian).
 *   - Bytes  4-5 : seq         (little-endian).
 *   - Bytes 6-11 : 0x00 (reserved).
 *
 * @p p_buffer must point to a buffer of at least @ref K_SPP_CIPHER_FRAME_LEN
 * bytes so the output frame fits.
 *
 * @param[in,out] p_buffer   On entry: raw SPP packet bytes.
 *                           On exit:  encrypted frame (nonce | ciphertext | tag).
 * @param[out]    p_outSize  Set to @ref K_SPP_CIPHER_FRAME_LEN on success.
 *
 * @return K_SPP_OK on success, K_SPP_ERROR_NULL_POINTER if any pointer is NULL.
 */
SPP_RetVal_t SPP_EXTERNAL_ENCRYPTION_CYPHER_encryptPacket(spp_uint8_t *p_buffer, spp_uint16_t *p_outSize);

#endif /* SPP_EXTERNAL_CIPHER_H */
