/**
 * @file crc.h
 * @brief CRC-16/CCITT checksum utility.
 *
 * Naming conventions used in this file:
 * - Constants/macros: K_SPP_CRC_*
 * - Public functions: SPP_Util_crc16()
 */

#ifndef SPP_CRC_H
#define SPP_CRC_H

#include "spp/core/types.h"

/* ----------------------------------------------------------------
 * Constants
 * ---------------------------------------------------------------- */

/** @brief CRC-16/CCITT polynomial (0x1021). */
#define K_SPP_CRC_POLY (0x1021U)

/** @brief CRC-16 initial value. */
#define K_SPP_CRC_INIT (0xFFFFU)

/* ----------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------- */

/**
 * @brief Compute a CRC-16/CCITT checksum over a byte buffer.
 *
 * @param[in] p_data   Pointer to the data buffer.
 * @param[in] length   Number of bytes to process.
 *
 * @return 16-bit CRC value.
 */
spp_uint16_t SPP_Util_crc16(const spp_uint8_t *p_data, spp_uint32_t length);

#endif /* SPP_CRC_H */
