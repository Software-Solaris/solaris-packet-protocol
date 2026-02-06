#include "core.h"
#include "../services/databank/databank.h"

static retval_t ret = SPP_OK;

retval_t Core_Init(void){
    ret = SPP_DATABANK_init();
    if(ret != SPP_OK){
        return ret;
    }
    return SPP_OK;
}

