// Aqui van las definiciones de los tipos que quieras crear

// El types.h que faltaba (por Xoel)

#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include "core/returntypes.h"


typedef void* spp_queue_handle_t;



typedef uint32_t SppStackSizeBytes;  

typedef enum {
    SPP_RETVAL_OK = 0,
    SPP_RETVAL_ERROR_INVALID_ARG,
    SPP_RETVAL_ERROR_OS
} SppRetVal_t;

typedef struct {
    void (*task_func)(void*);   
    const char* name;            
    SppStackSizeBytes stack_bytes;
    uint32_t priority;          
    void* params;                
    SppTaskHandle* out_handle;   
} SppTaskCreateParams;

#endif