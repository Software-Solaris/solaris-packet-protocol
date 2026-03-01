#include "db_flow.h"
#include "services/logging/spp_log.h"
#include <stddef.h>

static const char* TAG = "DB_FLOW";

static spp_packet_t* s_ready[DB_FLOW_READY_SIZE];
static uint32_t s_head = 0;
static uint32_t s_tail = 0;
static uint32_t s_count = 0;
static uint8_t  s_inited = 0;

retval_t DB_FLOW_Init(void)
{
    s_head = 0;
    s_tail = 0;
    s_count = 0;
    for (uint32_t i = 0; i < DB_FLOW_READY_SIZE; i++) {
        s_ready[i] = NULL;
    }
    s_inited = 1;
    SPP_LOGI(TAG, "Ready FIFO init size=%u", (unsigned)DB_FLOW_READY_SIZE);
    return SPP_OK;
}

retval_t DB_FLOW_PushReady(spp_packet_t* p_pkt)
{
    if (!s_inited) return SPP_ERROR;
    if (p_pkt == NULL) return SPP_ERROR_NULL_POINTER;

    if (s_count >= DB_FLOW_READY_SIZE) {
        return SPP_ERROR; // FIFO llena
    }

    s_ready[s_tail] = p_pkt;
    s_tail = (s_tail + 1u) % DB_FLOW_READY_SIZE;
    s_count++;
    return SPP_OK;
}

retval_t DB_FLOW_PopReady(spp_packet_t** pp_pkt)
{
    if (!s_inited) return SPP_ERROR;
    if (pp_pkt == NULL) return SPP_ERROR_NULL_POINTER;

    if (s_count == 0u) {
        *pp_pkt = NULL;
        return SPP_ERROR; // FIFO vacía
    }

    *pp_pkt = s_ready[s_head];
    s_ready[s_head] = NULL;
    s_head = (s_head + 1u) % DB_FLOW_READY_SIZE;
    s_count--;
    return SPP_OK;
}

uint32_t DB_FLOW_ReadyCount(void)
{
    return s_count;
}