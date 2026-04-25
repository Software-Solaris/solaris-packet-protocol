/**
 * @file module.c
 * @brief Implementation of <module name>.
 *
 * See module.h for the public API.
 */

#include "spp/core/returnTypes.h"
#include "spp/core/types.h"
#include "spp/hal/spi.h"
#include "spp/services/sx1261/sx1261.h"


/* ----------------------------------------------------------------
 * Static Variables
 * ---------------------------------------------------------------- */

/* ----------------------------------------------------------------
 * Private Functions
 * ---------------------------------------------------------------- */

/* ----------------------------------------------------------------
 * Public Functions
 * ---------------------------------------------------------------- */
SPP_RetVal_t SPP_SERVICES_SX1261_init(void)
{
    SPP_RetVal_t ret = K_SPP_ERROR;
    spp_uint8_t variable = 0x00;
    spp_uint8_t sendDataBuffer[2] = {K_SPP_SX1261_READ_OPCODE, SPP_SX1261_STDBY_XOSC};
    ret = SPP_HAL_spiTransmit(&variable, sendDataBuffer, 0x00);
    return ret;
}

/* ----------------------------------------------------------------
 * Service descriptor
 * ---------------------------------------------------------------- */