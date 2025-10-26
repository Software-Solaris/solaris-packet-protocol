#include "core.h"
#include "../services/databank/databank.h"

static SppRetVal_t ret = SPP_OK;

SppRetVal_t Core_Init(void){
    ret = DataBank_Init();
    if(ret != SPP_OK){
        return ret;
    }
    return SPP_OK;
}

