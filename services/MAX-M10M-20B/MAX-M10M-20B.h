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

/* UBX MESSAGES */
// Structure: Header | Class | ID | Length | (Payload) | Cheksum
// Header 0xb5 0x62

static const spp_uint8_t K_M10M_CTRL_RST_CMD[] = {0xb5, 0x62, 0x06, 0x04, 0x04, 0x00,
                                                  0x00, 0x00, 0x01, 0x00, 0x0F, 0x66};
static const spp_uint8_t K_M10M_ID_OUT[] = {0xb5, 0x62, 0x27, 0x03, 0x00, 0x00, 0x2A, 0xA5};

/* ----------------------------------------------------------------
*  PUBLIC FUNCTIONS
* ---------------------------------------------------------------- */
const SPP_SERVICE_ProducerContract_t *SPP_SERVICES_M10M_getProducerContract(void);

#endif /* SPP_MAXM1020B_H */