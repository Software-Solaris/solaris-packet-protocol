#include "core.h"
#include "../services/databank/databank.h"

static retval_t ret = SPP_ERROR;

retval_t Core_Init(void){

    ret = SPP_LOG_Init();
    if (ret != SPP_OK) {
        return ret;
    }
    
    SPP_LOG_SetLevel(SPP_LOG_VERBOSE);

    ret = SPP_DATABANK_init();
    if(ret != SPP_OK){
        return ret;
    }
    
    return SPP_OK;
}

