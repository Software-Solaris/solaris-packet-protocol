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
 * @brief Initializes the SPP core services.
 *
 * Initializes logging, the databank and PUBSUB, including registered
 * producers and consumers.
 *
 * Must be called after HAL initialization and module registration.
 *
 * @return K_SPP_OK on success, error code otherwise.
 */
SPP_RetVal_t SPP_CORE_init(void);

#endif /* SPP_CORE_H */
