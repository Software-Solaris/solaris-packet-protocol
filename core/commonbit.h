#ifndef SPP_CORE_ERROR_H
#define SPP_CORE_ERROR_H

#include "spp/core/types.h"
typedef union
{
    spp_uint16_t errors;
    struct
    {
        spp_uint8_t coreInitError     : 1;
        spp_uint8_t logInitError      : 1;
        spp_uint8_t dataBankInitError : 1;
        spp_uint8_t pubsubInitError   : 1;
        spp_uint16_t reserved         : 15;
    };
} CommonBitErrors_t;


/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */
/**
* @brief    Get the pointer to the common bit.
* @return   Pointer to the common bit.
*/
CommonBitErrors_t *SPP_CORE_COMMONBIT_getBit(void);


#endif /* SPP_CORE_ERROR_H */