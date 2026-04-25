/**
 * @file mocks.c
 * @brief Cgreen mocks for SPP unit tests.
 */

#include <cgreen/cgreen.h>
#include <cgreen/mocks.h>
#include "spp/core/returnTypes.h"
#include "spp/core/types.h"

SPP_RetVal_t SPP_HAL_spiTransmit(void *p_handle, spp_uint8_t *p_data, spp_uint8_t length)
{
    return (SPP_RetVal_t)mock(p_handle, p_data, length);
}


spp_uint32_t SPP_HAL_getTimeMs(void)
{
    return (spp_uint32_t)mock();
}