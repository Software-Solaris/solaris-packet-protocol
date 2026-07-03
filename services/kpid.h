#ifndef SPP_SERVICES_KPID_H
#define SPP_SERVICES_KPID_H
/**
 * @file kpid.h
 * @brief APID bitmask assignments for every SPP producer.
 *
 * Each producer owns exactly one bit of the 16-bit APID field.
 * To add a producer: pick the next free bit and add a line here.
 * To remove one: delete its line. No other IDs are affected.
 */

#include "spp/core/types.h"

/* ----------------------------------------------------------------
 * APID assignments  (one bit per producer, must not repeat)
 * ---------------------------------------------------------------- */
#define K_SPP_KPID_LOG        (0x0001U) /* bit  0 - reserved for log */
#define K_SPP_KPID_ICM20948   (0x0002U) /* bit  1                    */
#define K_SPP_KPID_BMP390     (0x0004U) /* bit  2                    */
#define K_SPP_KPID_UART       (0x0008U) /* bit  3                    */
#define K_SPP_KPID_RESERVED4  (0x0010U) /* bit  4 - reserved       */
#define K_SPP_KPID_RESERVED5  (0x0020U) /* bit  5 - reserved       */
#define K_SPP_KPID_RESERVED6  (0x0040U) /* bit  6 - reserved       */
#define K_SPP_KPID_RESERVED7  (0x0080U) /* bit  7 - reserved       */
#define K_SPP_KPID_RESERVED8  (0x0100U) /* bit  8 - reserved       */
#define K_SPP_KPID_RESERVED9  (0x0200U) /* bit  9 - reserved       */
#define K_SPP_KPID_RESERVED10 (0x0400U) /* bit 10 - reserved       */
#define K_SPP_KPID_RESERVED11 (0x0800U) /* bit 11 - reserved       */
#define K_SPP_KPID_RESERVED12 (0x1000U) /* bit 12 - reserved       */
#define K_SPP_KPID_RESERVED13 (0x2000U) /* bit 13 - reserved       */
#define K_SPP_KPID_RESERVED14 (0x4000U) /* bit 14 - reserved       */
#define K_SPP_KPID_RESERVED15 (0x8000U) /* bit 15 - reserved       */

/* ----------------------------------------------------------------
 * KPID type
 * ---------------------------------------------------------------- */
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

#endif /* SPP_SERVICES_KPID_H */