// Datapool (FIFO)-> Gestionar punteros a los N structs (paquetes) que tienen reservado su espacio en RAM

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
} spp_datapool_cfg_t;

retval_t SPP_Datapool_Init(void* p_handle, void* p_cfg); // Crear colas, resgistrar la memoria ()
retval_t SPP_Datapool_Acquire(void* handle, void* p_timeout_ms, void* pp_elem); // Sacar un puntero FREE y lo manda para rellenar
retval_t SPP_Datapool_Publish(void* handle, void* p_elem); // Meter puntero en READY
retval_t SPP_Datapool_ReceiveReady(void* handle, void* p_timeout_ms, void* pp_elem); // Sacar puntero de READY para el receptor
retval_t SPP_Datapool_Release(void* handle, void* p_elem); // Devolver puntero a FREE
retval_t SPP_Datapool_Reset(void* handle); // Volver al estado inicial