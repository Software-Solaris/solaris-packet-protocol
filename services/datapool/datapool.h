// Datapool (FIFO)-> Gestionar punteros a los N structs (paquetes) que tienen reservado su espacio en RAM
// Dos FIFO: uno para los punteros libres (FREE) y otro para los punteros listos para ser procesados (READY)
#include "returntypes.h"
#include "types.h"

// Gestión en caso de overflow 
typedef enum
{
    SPP_POOL_OVERFLOW_BLOCK = 0,        // Espera a que haya espacio libre
    SPP_POOL_OVERFLOW_DROP_NEW = 1      // Descarta si está lleno
} spp_pool_overflow_policy_t;  
// En resumen, decidir si esperar o tirar el paquete que llega

typedef struct
{
    void* p_mem_base; // Donde estan los structs reservados en RAM
    spp_uint16_t elem_size; // Tamaño de cada struct
    spp_uint16_t elem_count; // Numero de structs disponibles
    spp_pool_overflow_policy_t overflow_policy;
    void* p_free_q_ctrl; // Para crear la cola FREE
    void* p_ready_q_ctrl; // Para crear la cola READY
    void* p_free_q_storage; // Guardar los FREE
    void* p_ready_q_storage; // Guardar los READY
} SPP_DatapoolCfg;

typedef struct 
{
    const SPP_DatapoolCfg* cfg;
    void *q_free_handle;
    void *q_ready_handle;
}SPP_DatapoolHandle;

retval_t SPP_Datapool_Init(SPP_DatapoolHandle* p_handle, const SPP_DatapoolCfg* p_cfg); // Crear colas, resgistrar la memoria
retval_t SPP_Datapool_Acquire(SPP_DatapoolHandle* p_handle, const spp_uint32_t* p_timeout_ms, void* p_out_elem); // Sacar un puntero FREE y lo manda para rellenar
retval_t SPP_Datapool_Publish(SPP_DatapoolHandle* p_handle, void* p_elem); // Meter puntero en READY
retval_t SPP_Datapool_ReceiveReady(SPP_DatapoolHandle* p_handle, const spp_uint32_t* p_timeout_ms, void* p_out_elem); // Sacar puntero de READY para el receptor
retval_t SPP_Datapool_Release(SPP_DatapoolHandle* p_handle, void* p_elem); // Devolver puntero a FREE
retval_t SPP_Datapool_Reset(SPP_DatapoolHandle* p_handle); // Volver al estado inicial

// Notacion: p_elem es el puntero a un elemento del pool y p_out_elem es el puntero al buffer donde la función escribe  ese puntero.
/*
 Ejemplo: 
 void* p_slot
 SPP_Datapool_Acquire(p_datapool_handle, timeout, &p_slot);
 SPP_Datapool_Publish(p_datapool_handle, p_slot);
 Asi evitamos dobles punteros.
*/ 