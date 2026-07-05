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
#define K_SPP_KPID_LOG (0x0001U) /* bit  0 - reserved for log */
#define K_SPP_KPID_FSM (0x0002U) /* bit  1 - reserved for fsm */

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