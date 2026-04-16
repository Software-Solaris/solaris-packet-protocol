/**
 * @file spp.h
 * @brief Top-level SPP include — pulls in the full public API.
 *
 * Include this single header to access all SPP functionality.
 * Platform ports still need to provide @ref SPP_OsalPort_t and
 * @ref SPP_HalPort_t implementations registered before @ref SPP_Core_init().
 */

#ifndef SPP_H
#define SPP_H

/* Core */
#include "spp/core/version.h"
#include "spp/core/types.h"
#include "spp/core/returntypes.h"
#include "spp/core/packet.h"
#include "spp/core/core.h"

/* Port contracts */
#include "spp/osal/port.h"
#include "spp/hal/port.h"

/* OSAL dispatch API */
#include "spp/osal/task.h"
#include "spp/osal/queue.h"
#include "spp/osal/mutex.h"
#include "spp/osal/event.h"

/* HAL dispatch API */
#include "spp/hal/spi.h"
#include "spp/hal/gpio.h"
#include "spp/hal/storage.h"

/* Services */
#include "spp/services/service.h"
#include "spp/services/databank/databank.h"
#include "spp/services/db_flow/db_flow.h"
#include "spp/services/log/log.h"

/* Utilities */
#include "spp/util/macros.h"
#include "spp/util/crc.h"
#include "spp/util/structof.h"

#endif /* SPP_H */
