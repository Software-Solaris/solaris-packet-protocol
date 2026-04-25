/**
 * @file sx1261.h
 * @brief One-line description of this header.
 *
 * Extended description if needed. Describe the purpose of the
 * SX1261, not the implementation details.
 *
 * Naming conventions used in this file:
 *  - Constants / macros : K_SPP_SX1261_*
 *  - Types              : SPP_SX1261Name_t
 *  - Public functions   : SPP_SX1261_functionName()
 *  - Pointer params     : p_paramName
 *  - Static module vars : s_varName
 */

#ifndef SPP_SX1261_H
#define SPP_SX1261_H

#include "spp/core/returnTypes.h"
#include "spp/core/types.h"

/* ----------------------------------------------------------------
 * Constants
 * ---------------------------------------------------------------- */

/** @brief <description of constant>. */
#define K_SPP_SX1261_EXAMPLE     (42U)
#define K_SPP_SX1261_READ_OPCODE 0x80

/* ----------------------------------------------------------------
 * Types
 * ---------------------------------------------------------------- */
typedef enum
{
    SPP_SX1261_STDBY_RC = 0x00,  // Device running on RC13M
    SPP_SX1261_STDBY_XOSC = 0x01 // Device running on XTAL 32MHz
} SPP_SX1261_StdbyConfig_t;

/* ----------------------------------------------------------------
 * Public Functions
 * ---------------------------------------------------------------- */


SPP_RetVal_t SPP_SERVICES_SX1261_init(void);

#endif /* SPP_SX1261_H */