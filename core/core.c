/**
 * @file core.c
 * @brief SPP core initialization implementation.
 */

/* ---------------------------------------------------------------- */
/*  Includes                                                        */
/* ---------------------------------------------------------------- */

#include "spp/core/core.h"
#include "spp/services/databank/databank.h"

/* ---------------------------------------------------------------- */
/*  Static Variables                                                */
/* ---------------------------------------------------------------- */

static retval_t s_ret = SPP_ERROR;

/* ---------------------------------------------------------------- */
/*  Public Functions                                                */
/* ---------------------------------------------------------------- */

retval_t Core_Init(void)
{
    s_ret = SPP_LOG_Init();
    if (s_ret != SPP_OK)
    {
        return s_ret;
    }

    SPP_LOG_SetLevel(SPP_LOG_VERBOSE);

    s_ret = SPP_DATABANK_init();
    if (s_ret != SPP_OK)
    {
        return s_ret;
    }

    return SPP_OK;
}
