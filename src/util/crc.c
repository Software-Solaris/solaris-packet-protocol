/**
 * @file crc.c
 * @brief CRC-16/CCITT implementation.
 */

#include "spp/util/crc.h"

spp_uint16_t SPP_Util_crc16(const spp_uint8_t *p_data, spp_uint32_t length)
{
    spp_uint16_t crc = K_SPP_CRC_INIT;

    for (spp_uint32_t i = 0U; i < length; i++)
    {
        crc ^= (spp_uint16_t)((spp_uint16_t)p_data[i] << 8U);
        for (spp_uint8_t bit = 0U; bit < 8U; bit++)
        {
            if ((crc & 0x8000U) != 0U)
            {
                crc = (spp_uint16_t)((crc << 1U) ^ K_SPP_CRC_POLY);
            }
            else
            {
                crc = (spp_uint16_t)(crc << 1U);
            }
        }
    }
    return crc;
}
