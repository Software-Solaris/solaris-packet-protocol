/**
 * @file datalogger.h
 * @brief SD card packet logger service.
 */

#ifndef SPP_DATALOGGER_H
#define SPP_DATALOGGER_H

#include "spp/core/returnTypes.h"
#include "spp/core/packet.h"
#include "spp/services/service.h"

/* ----------------------------------------------------------------
 * DEFINES
 * ---------------------------------------------------------------- */

#define K_DATALOGGER_CONSUMER_ID   (0x01U)
#define K_DATALOGGER_CONSUMER_PRIO (0)
#define K_DATALOGGER_TIMEOUT_MS    (1000U)
#define K_DATALOGGER_SPI_DEV_IDX   (2U)
#define K_DATALOGGER_SECTOR_SIZE   (512U)
#define K_DATALOGGER_FIRST_SECTOR  ((spp_uint32_t)16384U)
#define K_DATALOGGER_BUFFER_SIZE   (2U * K_DATALOGGER_SECTOR_SIZE)
#define K_DATALOGGER_NO_PENDING    (0xFFU)

/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */

const SPP_SERVICE_ConsumerContract_t *SPP_SERVICES_DATALOGGER_getConsumerContract(void);

#endif /* SPP_DATALOGGER_H */