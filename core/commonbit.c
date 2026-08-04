

/**
 * @file commonbit.c
 * @brief Common bit functions for the SPP core library.
 */


/* ----------------------------------------------------------------
 * INCLUDES
 * ---------------------------------------------------------------- */
#include "spp/core/commonbit.h"

/* ----------------------------------------------------------------
* VARIABLES
* ---------------------------------------------------------------- */

static CommonBitErrors_t s_commonBit = {0};

/* ----------------------------------------------------------------
* PUBLIC FUNCTIONS
* ---------------------------------------------------------------- */

CommonBitErrors_t *SPP_CORE_COMMONBIT_getBit(void)
{
    return &s_commonBit;
}