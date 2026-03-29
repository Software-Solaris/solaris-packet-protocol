/**
 * @file core.h
 * @brief SPP core initialization interface.
 */

#ifndef CORE_H
#define CORE_H

#ifdef __cplusplus
extern "C"
{
#endif

    /* ---------------------------------------------------------------- */
    /*  Includes                                                        */
    /* ---------------------------------------------------------------- */

#include "macros.h"
#include "returntypes.h"

    /* ---------------------------------------------------------------- */
    /*  Public Functions                                                */
    /* ---------------------------------------------------------------- */

    /**
 * @brief Initialize the SPP core services (logging and databank).
 * @return retval_t SPP_OK on success, SPP_ERROR on failure.
 */
    retval_t Core_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* CORE_H */
