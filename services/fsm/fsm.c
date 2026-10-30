/**
 * @file fsm.c
 * @brief Finite State Machine (FSM) implementation for the SPP core.
 */


/* ----------------------------------------------------------------
 * INCLUDES
 * ---------------------------------------------------------------- */
#include "spp/services/fsm/fsm.h"
#include "spp/core/pubsub/pubsub.h"

/* ----------------------------------------------------------------
 * STRUCTS
 * ---------------------------------------------------------------- */
typedef struct
{
    const FSM_Transition_t *p_table;
    spp_uint8_t size;
} FSM_Table_t;

/* ----------------------------------------------------------------
 * VARIABLES
 * ---------------------------------------------------------------- */
static spp_uint16_t s_fsmErrors = {0};

static FSM_Table_t s_tables[K_FSM_MAX_TABLES];
static spp_uint8_t s_tableCount = 0U;

static FSM_Handle_t s_fsmHandle = {
    .state = FSM_STATE_INIT,
    .subState = FSM_SUBSTATE_NONE,
    .prevState = FSM_STATE_INIT,
    .prevSubState = FSM_SUBSTATE_NONE,
};

/* ----------------------------------------------------------------
 * STATIC FUNCTIONS
 * ---------------------------------------------------------------- */
/**
 * @brief  Run one complete PUBSUB cycle: acquire producer data and dispatch
 *         it to the consumer mailboxes, then call consumeData().
 */
static void FSM_pubsubCycle(void)
{
    SPP_SERVICES_PUBSUB_callProducers();
    SPP_SERVICES_PUBSUB_callConsumers();
}

/* ----------------------------------------------------------------
 * SERVICE TRANSITION TABLE
 *
 * Rows with guard == NULL run their stateFunction every tick while the FSM
 * is in the matching state/sub-state. The service table keeps the PUBSUB
 * cycle alive in every active state.
 * ---------------------------------------------------------------- */
static const FSM_Transition_t s_fsmBaseTable[] = {
    {
        .fromState = FSM_STATE_READY,
        .fromSubState = FSM_SUBSTATE_NONE,
        .toState = FSM_STATE_READY,
        .toSubState = FSM_SUBSTATE_NONE,
        .guard = NULL,
        .action = NULL,
        .stateFunction = FSM_pubsubCycle,
    },
    {
        .fromState = FSM_STATE_FLIGHT,
        .fromSubState = FSM_SUBSTATE_NONE,
        .toState = FSM_STATE_FLIGHT,
        .toSubState = FSM_SUBSTATE_NONE,
        .guard = NULL,
        .action = NULL,
        .stateFunction = FSM_pubsubCycle,
    },
    {
        .fromState = FSM_STATE_FLIGHT,
        .fromSubState = FSM_SUBSTATE_FLIGHT_ASCENDING,
        .toState = FSM_STATE_FLIGHT,
        .toSubState = FSM_SUBSTATE_FLIGHT_ASCENDING,
        .guard = NULL,
        .action = NULL,
        .stateFunction = FSM_pubsubCycle,
    },
    {
        .fromState = FSM_STATE_FLIGHT,
        .fromSubState = FSM_SUBSTATE_FLIGHT_DESCENDING,
        .toState = FSM_STATE_FLIGHT,
        .toSubState = FSM_SUBSTATE_FLIGHT_DESCENDING,
        .guard = NULL,
        .action = NULL,
        .stateFunction = FSM_pubsubCycle,
    },
    {
        .fromState = FSM_STATE_FLIGHT,
        .fromSubState = FSM_SUBSTATE_FLIGHT_PARACHUTE_DEPLOYED,
        .toState = FSM_STATE_FLIGHT,
        .toSubState = FSM_SUBSTATE_FLIGHT_PARACHUTE_DEPLOYED,
        .guard = NULL,
        .action = NULL,
        .stateFunction = FSM_pubsubCycle,
    },
    {
        .fromState = FSM_STATE_LANDED,
        .fromSubState = FSM_SUBSTATE_NONE,
        .toState = FSM_STATE_LANDED,
        .toSubState = FSM_SUBSTATE_NONE,
        .guard = NULL,
        .action = NULL,
        .stateFunction = FSM_pubsubCycle,
    },
};

/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */
spp_uint16_t *SPP_CORE_FSM_getErrorsBit(void)
{
    return &s_fsmErrors;
}

SPP_RetVal_t FSM_init(void)
{
    s_tableCount = 0U;

    s_fsmHandle.state = FSM_STATE_INIT;
    s_fsmHandle.subState = FSM_SUBSTATE_NONE;
    s_fsmHandle.prevState = FSM_STATE_INIT;
    s_fsmHandle.prevSubState = FSM_SUBSTATE_NONE;

    return FSM_addTable(s_fsmBaseTable,
                        (spp_uint8_t)(sizeof(s_fsmBaseTable) / sizeof(s_fsmBaseTable[0])));
}

SPP_RetVal_t FSM_addTable(const FSM_Transition_t *p_transitionTable, const spp_uint8_t tableSize)
{
    if (p_transitionTable == NULL)
    {
        return K_SPP_ERROR_NULL_POINTER;
    }

    if (tableSize == 0U || tableSize > K_FSM_MAX_TABLE_SIZE)
    {
        return K_SPP_ERROR_INVALID_PARAMETER;
    }

    if (s_tableCount >= K_FSM_MAX_TABLES)
    {
        return K_SPP_ERROR_REGISTRY_FULL;
    }

    s_tables[s_tableCount].p_table = p_transitionTable;
    s_tables[s_tableCount].size = tableSize;
    s_tableCount++;

    return K_SPP_OK;
}

void FSM_tick(void)
{
    for (spp_uint8_t t = 0U; t < s_tableCount; t++)
    {
        for (spp_uint8_t i = 0U; i < s_tables[t].size; i++)
        {
            const FSM_Transition_t *p_t = &s_tables[t].p_table[i];

            if (p_t->fromState != s_fsmHandle.state || p_t->fromSubState != s_fsmHandle.subState)
            {
                continue;
            }

            if (p_t->guard == NULL)
            {
                if (p_t->stateFunction != NULL)
                {
                    p_t->stateFunction();
                }
                continue;
            }

            if (!p_t->guard())
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

            return;
        }
    }
}
