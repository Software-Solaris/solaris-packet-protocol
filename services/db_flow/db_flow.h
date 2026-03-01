#ifndef DB_FLOW_H
#define DB_FLOW_H

#include "core/returntypes.h"
#include "core/packet.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Tamaño FIFO ready (debug). Ponlo pequeño ahora; luego lo ajustas. */
#ifndef DB_FLOW_READY_SIZE
#define DB_FLOW_READY_SIZE 16
#endif

retval_t DB_FLOW_Init(void);

/* Mete un paquete ya relleno en la FIFO "ready" */
retval_t DB_FLOW_PushReady(spp_packet_t* p_pkt);

/* Saca un paquete de la FIFO "ready" */
retval_t DB_FLOW_PopReady(spp_packet_t** pp_pkt);

/* Utilidad para debug */
uint32_t DB_FLOW_ReadyCount(void);

#ifdef __cplusplus
}
#endif

#endif /* DB_FLOW_H */