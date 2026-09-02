/**
 * @file fsm.c
 * @brief Finite State Machine (FSM) implementation for the SPP core.
 */


/* ----------------------------------------------------------------
 * INCLUDES
 * ---------------------------------------------------------------- */
#include "spp/services/fsm/fsm.h"
#include "spp/services/service.h"
#include "spp/services/bmp390/bmp390.h"
#include "spp/services/icm20948/icm20948.h"
#include "spp/services/datalogger/datalogger.h"
#include "spp/services/e22-mbl01/e22-mbl01.h"
#include "spp/services/maxm10m20b/maxm10m20b.h"
#include "spp/core/returnTypes.h"
#include "spp/services/kpid.h"
#include "spp/core/pubsub/pubsub.h"
#include "spp/core/core.h"
#include "spp/core/commonbit.h"
#include "spp/services/databank/databank.h"
#include "spp/services/kpid.h"
#include "spp/hal/hal.h"


/* ----------------------------------------------------------------
* VARIABLES
* ---------------------------------------------------------------- */

FsmErrors_t s_fsmErrors = {0};

static void *s_p_ports;

static const FSM_Transition_t *p_s_transitionTable = NULL;

static FSM_Handle_t s_fsmHandle = {
    .state = FSM_STATE_INIT,
    .subState = FSM_SUBSTATE_NONE,
    .prevState = FSM_STATE_INIT,
    .prevSubState = FSM_SUBSTATE_NONE,
};

static spp_uint8_t s_tableSize = 0;


/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */
FsmErrors_t *SPP_CORE_FSM_getErrorsBit(void)
{
    return &s_fsmErrors;
}

SPP_RetVal_t FSM_init(void *p_halPorts, const FSM_Transition_t *p_transitionTable, const spp_uint8_t tableSize)
{
    if ((p_transitionTable == NULL) || (p_halPorts == NULL))
    {
        return K_SPP_ERROR_NULL_POINTER;
    }

    // Check the size of the table
    if (tableSize > K_FSM_MAX_TABLE_SIZE || tableSize == 0)
    {
        return K_SPP_ERROR_INVALID_PARAMETER;
    }

    // Assign the table parameters to the static variables
    p_s_transitionTable = p_transitionTable;
    s_tableSize = tableSize;
    s_p_ports = p_halPorts;

    return K_SPP_OK;
}

void FSM_tick(void)
{
    for (spp_uint8_t i = 0; i < s_tableSize; i++)
    {
        // TODO: review if this is correct
        const FSM_Transition_t *p_t = &p_s_transitionTable[i];

        if (p_t->fromState != s_fsmHandle.state || p_t->fromSubState != s_fsmHandle.subState)
        {
            continue;
        }

        if (p_t->guard == NULL)
        {
            p_t->stateFunction();
            continue;
        }
        else if (!p_t->guard())
        {
            continue;
        }

        s_fsmHandle.prevState = s_fsmHandle.state;
        s_fsmHandle.prevSubState = s_fsmHandle.subState;
        s_fsmHandle.state = p_t->toState;
        s_fsmHandle.subState = p_t->toSubState;

        if (p_t->action != NULL)
        {
            p_t->action();
        }
        if (p_t->stateFunction != NULL)
        {
            p_t->stateFunction();
        }

        break;
    }
}