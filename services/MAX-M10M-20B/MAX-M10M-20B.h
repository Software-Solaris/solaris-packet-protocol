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
// Structure: Header (Sync) | Class | ID | Length | (Payload) | Cheksum
// Header 0xb5 0x62

static const spp_uint8_t K_M10M_CTRL_RST_CMD[] = {
    0xB5, 0x62, // Sync
    0x06, 0x04, // UBX-CFG-RST
    0x04, 0x00, // Length = 4
    0x00, 0x00, // navBbrMask = 0x0000 (Hot start)
    0x01,       // resetMode = Controlled software reset
    0x00,       // Reserved
    0x0F, 0x66  // Checksum
};

static const spp_uint8_t K_M10M_ID_OUT[] = {
    0xB5, 0x62, // Sync
    0x27, 0x03, // UBX-SEC-UNIQID
    0x00, 0x00, // Length = 0 (poll request)
    0x2A, 0xA5  // Checksum
};

// Layer (bitfield): RAM (0000 0001 = 0x01) BBR (0000 0010 = 0x02) FLASH (0000 0100 = 0x04)
static const spp_uint8_t K_M10M_AIR4_DYNMODEL_SET[] = {
    0xB5, 0x62,             // Sync
    0x06, 0x8A,             // UBX-CFG-VALSET
    0x09, 0x00,             // Length = 9
    0x00,                   // version
    0x01,                   // layer = RAM
    0x00, 0x00,             // reserved
    0x21, 0x00, 0x11, 0x20, // CFG-NAVSPG-DYNMODEL
    0x08,                   // Airborne <4g
    0xF4, 0x51              // Checsum
};

// Layer: RAM (0x00) BBR (0x01) FLASH (0x02) DFEAULT (0x07)
static const spp_uint8_t K_M10M_AIR4_DYNMODEL_GET[] = {
    0xB5, 0x62,             // Sync
    0x06, 0x8B,             // UBX-CFG-VALGET
    0x08, 0x00,             // Length = 8
    0x00,                   // version
    0x00,                   // layer = RAM
    0x00, 0x00,             // position
    0x21, 0x00, 0x11, 0x20, // CFG-NAVSPG-DYNMODEL
    0xEB, 0x57              // Checksum
};

static const spp_uint8_t K_M10M_DYNMODEL_ACK_OUT[] = {
    0xB5, 0x62, // Sync
    0x05, 0x01, // UBX-ACK-ACK
    0x02, 0x00, // Length
    0x06, 0x8A, // clsID, msdID <-> UBX-CFG-VALSET
    0x98, 0xC1  // Checksum
};

static const spp_uint8_t K_M10M_DYNMODEL_NAK_OUT[] = {
    0xB5, 0x62, // Sync
    0x05, 0x00, // UBX-ACK-NAK
    0x02, 0x00, // Length
    0x06, 0x8A, // clsID, msdID <-> UBX-CFG-VALSET
    0x97, 0xBC  // Checksum
};
/* ----------------------------------------------------------------
*  PUBLIC FUNCTIONS
* ---------------------------------------------------------------- */
const SPP_SERVICE_ProducerContract_t *SPP_SERVICES_M10M_getProducerContract(void);

#endif /* SPP_MAXM1020B_H */