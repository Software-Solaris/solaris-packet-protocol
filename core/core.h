/**
 * @file core.h
 * @brief SPP core initialisation and port registration API.
 */

#ifndef SPP_CORE_H
#define SPP_CORE_H

#include "spp/core/returnTypes.h"

/* ----------------------------------------------------------------
 * Lower-level API (available if you need finer control)
 * ---------------------------------------------------------------- */

/**
* @brief Initialise the core software stack.  Must be called before any other SPP function.
*        It starts the databank and the log systems.
*/
SPP_RetVal_t SPP_CORE_init(void);

#endif /* SPP_CORE_H */
