/**
 * @file initEvents.h
 * @brief Init info service.
 */

#ifndef SPP_EVENTS_H
#define SPP_EVENTS_H

#include "stdint.h"
#include "spp/core/returnTypes.h"
#include "spp/core/types.h"
#include "services/service.h"

/* ----------------------------------------------------------------
*  STRCUCTS
* ---------------------------------------------------------------- */
typedef struct
{
    spp_uint16_t seq;
} initEvents_t;

/* ----------------------------------------------------------------
*  PUBLIC FUNCTIONS
* ---------------------------------------------------------------- */
const SPP_SERVICE_ProducerContract_t *SPP_SERVICES_INITEVENTS_getProducerContract(void);

#endif /*SPP_EVENTS_H*/