/**
 * @file MAX-M10M-20B_service.c
 * @brief Ublox GNSS (MAX-M10M-20B) driver + SPP service implementation.
 */

#include "spp/services/maxm10m20b/maxm10m20b.h"

#include "spp/hal/uart/uart.h"
#include "spp/hal/time/time.h"
#include "spp/services/log/log.h"
#include "spp/services/databank/databank.h"
#include "spp/core/packet.h"
#include "spp/core/pubsub/pubsub.h"
#include "spp/services/fsm/fsm.h"


#include <string.h>

/* ----------------------------------------------------------------
 * CONSTANTS
 * ---------------------------------------------------------------- */
#define K_M10M_TAG "MAX-M10M-20B"


/* ----------------------------------------------------------------
 * STATIC FUNCTIONS DECLARATIONS
 * ---------------------------------------------------------------- */
static SPP_RetVal_t SPP_SERVICES_M10M_init(void);
static SPP_RetVal_t SPP_SERVICES_M10M_acquireData(SPP_Kpid_t kpid);
/* ----------------------------------------------------------------
* VARIABLES
* ---------------------------------------------------------------- */
static const SPP_SERVICE_ProducerContract_t g_m10mProducerContract = {.p_nameProducer = "GNSS (UBLOX MAX-M10M-20B)",
                                                                      .tiemoutMs = K_M10M_TIMEOUT_MS,
                                                                      .init = SPP_SERVICES_M10M_init,
                                                                      .acquireData = NULL};
static spp_uint8_t s_M10M_id[18] = {};
static spp_uint8_t s_M10M_dynModel[17] = {};
static spp_uint8_t s_M10M_dynModel_ack[10] = {};
static spp_uint32_t s_readBytes = 0;

/* ----------------------------------------------------------------
* PUBLIC FUNCTIONS
* ---------------------------------------------------------------- */
const SPP_SERVICE_ProducerContract_t *SPP_SERVICES_M10M_getProducerContract(void)
{
    return &g_m10mProducerContract;
}

/* ----------------------------------------------------------------
* STATIC FUNCTIONS
* ---------------------------------------------------------------- */
static SPP_RetVal_t SPP_SERVICES_M10M_init(void)
{
    FsmErrors_t *p_fsmErrors = SPP_CORE_FSM_getErrorsBit();
    SPP_RetVal_t ret = SPP_HAL_UART_transmit(K_M10M_CTRL_RST_CMD, sizeof(K_M10M_CTRL_RST_CMD));
    if (ret != K_SPP_OK)
    {
        p_fsmErrors->m10mInitError = 1;
        return K_SPP_ERROR;
    }

    SPP_HAL_TIME_delayMs(200);

    ret = SPP_HAL_UART_transmit(K_M10M_ID_OUT, sizeof(K_M10M_ID_OUT));
    if (ret != K_SPP_OK)
    {
        p_fsmErrors->m10mInitError = 1;
        return K_SPP_ERROR;
    }

    ret = SPP_HAL_UART_read(s_M10M_id, sizeof(s_M10M_id), &s_readBytes);
    if (ret != K_SPP_OK || s_readBytes != sizeof(s_M10M_id))
    {
        p_fsmErrors->m10mInitError = 1;
        return K_SPP_ERROR;
    }

    ret = SPP_HAL_UART_transmit(K_M10M_AIR4_DYNMODEL_SET, sizeof(K_M10M_AIR4_DYNMODEL_SET));
    if (ret != K_SPP_OK)
    {
        p_fsmErrors->m10mInitError = 1;
        return K_SPP_ERROR;
    }

    ret = SPP_HAL_UART_read(s_M10M_dynModel_ack, sizeof(s_M10M_dynModel_ack), &s_readBytes);
    if (ret != K_SPP_OK || s_readBytes != sizeof(s_M10M_dynModel_ack))
    {
        p_fsmErrors->m10mInitError = 1;
        return K_SPP_ERROR;
    }

    spp_int16_t comp = memcmp(s_M10M_dynModel_ack, K_M10M_DYNMODEL_ACK_OUT, sizeof(K_M10M_DYNMODEL_ACK_OUT));
    if (comp != 0)
    {
        p_fsmErrors->m10mInitError = 1;
        return K_SPP_ERROR;
    }

    ret = SPP_HAL_UART_transmit(K_M10M_AIR4_DYNMODEL_GET, sizeof(K_M10M_AIR4_DYNMODEL_GET));
    if (ret != K_SPP_OK)
    {
        p_fsmErrors->m10mInitError = 1;
        return K_SPP_ERROR;
    }

    ret = SPP_HAL_UART_read(s_M10M_dynModel, sizeof(s_M10M_dynModel), &s_readBytes);
    if (ret != K_SPP_OK || (s_M10M_dynModel[14] != 0x08))
    {
        p_fsmErrors->m10mInitError = 1;
        return K_SPP_ERROR;
    }

    return K_SPP_OK;
}