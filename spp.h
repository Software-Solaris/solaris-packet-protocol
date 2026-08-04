/**
 * @file spp.h
 * @brief Top-level SPP include — pulls in the full public API.
 *
 * Include this single header to access all SPP functionality.
 * Platform ports need to provide a @ref SPP_HalPort_t implementation
 * registered before @ref SPP_CORE_init().
 */

#ifndef SPP_H
#define SPP_H

/* Core */
#include "spp/core/version.h"
#include "spp/core/types.h"
#include "spp/core/returnTypes.h"
#include "spp/core/packet.h"
#include "spp/core/core.h"

/* Port contracts */
#include "spp/hal/hal.h"

/* HAL dispatch API */
#include "spp/hal/spi/spi.h"
#include "spp/hal/uart/uart.h"
#include "spp/hal/gpio/gpio.h"
#include "spp/hal/storage/storage.h"
#include "spp/hal/time/time.h"

/* Services */
#include "spp/services/service.h"
#include "spp/services/databank/databank.h"
#include "spp/core/pubsub/pubsub.h"
#include "spp/services/log/log.h"

/* Utilities */
#include "spp/util/macros.h"
#include "spp/util/crc.h"
#include "spp/util/structof.h"

#endif /* SPP_H */
