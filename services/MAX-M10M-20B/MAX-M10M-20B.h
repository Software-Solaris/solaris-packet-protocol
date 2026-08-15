/**
 * @file MAX-M10M-20B.h
 * @brief UBlox GNSS (MAX-M10M-20B) service.
 */

#ifndef SPP_MAXM10M20B_H
#define SPP_MAXM102M0B_H

#include "spp/core/returnTypes.h"
#include "spp/core/packet.h"
#include "spp/core/types.h"
#include "spp/services/service.h"

/* ----------------------------------------------------------------
 * CONSTANTS
 * ---------------------------------------------------------------- */

/* Service */
#define K_M10M_TASK_PRIO           (0)
#define K_M10M_TIMEOUT_MS          (1000U)
#define K_M10M_SERVICE_PAYLOAD_LEN (12U)
#define K_M10M_LOG_TAG             "MAX-M10M-20B"

/* ----------------------------------------------------------------
*  PUBLIC FUNCTIONS
* ---------------------------------------------------------------- */
const SPP_SERVICE_ProducerContract_t *SPP_SERVICES_M10M_getProducerContract(void);

#endif /* SPP_MAXM1020B_H */