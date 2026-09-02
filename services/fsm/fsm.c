/**
 * @file fsm.c
 * @brief Finite State Machine (FSM) implementation for the SPP core.
 */


/* ----------------------------------------------------------------
 * INCLUDES
 * ---------------------------------------------------------------- */
#include "spp/services/fsm/fsm.h"

/* ----------------------------------------------------------------
* VARIABLES
* ---------------------------------------------------------------- */

static spp_uint16_t s_fsmErrors = {0};

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
spp_uint16_t *SPP_CORE_FSM_getErrorsBit(void)
{
    return &s_fsmErrors;
}

SPP_RetVal_t FSM_init(const FSM_Transition_t *p_transitionTable, const spp_uint8_t tableSize)
{
    if (p_transitionTable == NULL)
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