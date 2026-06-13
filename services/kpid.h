/**
 * @file kpid.h
 * @brief SPP service union with bitmap.
 */

#include "spp/core/types.h"

typedef union
{
    spp_uint16_t value;

    struct
    {
        spp_uint16_t bit0  : 1;
        spp_uint16_t bit1  : 1;
        spp_uint16_t bit2  : 1;
        spp_uint16_t bit3  : 1;
        spp_uint16_t bit4  : 1;
        spp_uint16_t bit5  : 1;
        spp_uint16_t bit6  : 1;
        spp_uint16_t bit7  : 1;
        spp_uint16_t bit8  : 1;
        spp_uint16_t bit9  : 1;
        spp_uint16_t bit10 : 1;
        spp_uint16_t bit11 : 1;
        spp_uint16_t bit12 : 1;
        spp_uint16_t bit13 : 1;
        spp_uint16_t bit14 : 1;
        spp_uint16_t bit15 : 1;
    } bits;

} SPP_Kpid_t;

/**
 * @brief Use Example:
 * SPP_Kpid_t kpid = {0};
 * 
 * Activate bit1: kpid.bits.bit1 = 1;
 * Then: kpid.value = 0x0002
 * 
 * Activate bit2: kpi.bits.bit2 = 1; // with kpid.bits.bit1 = 0
 * Then: kpid.value = 0x0004
 * 
 * Or u can do:
 * kpid.value = 0x0006; // 0x0004|0x0002
 * Then: kpid.bits.bit1 = 1 & kpid.bits.bit2 = 1
 */