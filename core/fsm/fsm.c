/**
 * @file fsm.c
 * @brief Finite State Machine (FSM) implementation for the SPP core.
 */


/* ----------------------------------------------------------------
 * INCLUDES
 * ---------------------------------------------------------------- */
#include "spp/core/fsm/fsm.h"
#include "spp/services/service.h"


/* ----------------------------------------------------------------
 * VARIABLES
 * ---------------------------------------------------------------- */

static const FSM_Transition_t s_transitionTable[] = {
    {
        .fromState = FSM_STATE_INIT,
        .fromSubState = FSM_SUBSTATE_NONE,
        .toState = FSM_STATE_READY,
        .toSubState = FSM_SUBSTATE_NONE,
        .event = FSM_EVENT_NONE,
        .guard = NULL,
        .action = NULL,
    },
    {
        .fromState = FSM_STATE_READY,
        .fromSubState = FSM_SUBSTATE_NONE,
        .toState = FSM_STATE_FLIGHT,
        .toSubState = FSM_SUBSTATE_FLIGHT_ASCENDING,
        .event = FSM_EVENT_NONE,
        .guard = NULL,
        .action = NULL,
    },
    {
        .fromState = FSM_STATE_FLIGHT,
        .fromSubState = FSM_SUBSTATE_FLIGHT_ASCENDING,
        .toState = FSM_STATE_FLIGHT,
        .toSubState = FSM_SUBSTATE_FLIGHT_DESCENDING,
        .event = FSM_EVENT_NONE,
        .guard = NULL,
        .action = NULL,
    },
    {
        .fromState = FSM_STATE_FLIGHT,
        .fromSubState = FSM_SUBSTATE_FLIGHT_DESCENDING,
        .toState = FSM_STATE_FLIGHT,
        .toSubState = FSM_SUBSTATE_FLIGHT_PARACHUTE_DEPLOYED,
        .event = FSM_EVENT_NONE,
        .guard = NULL,
        .action = NULL,
    },
    {
        .fromState = FSM_STATE_FLIGHT,
        .fromSubState = FSM_SUBSTATE_FLIGHT_PARACHUTE_DEPLOYED,
        .toState = FSM_STATE_LANDED,
        .toSubState = FSM_SUBSTATE_NONE,
        .event = FSM_EVENT_NONE,
        .guard = NULL,
        .action = NULL,
    },
};

static FSM_Handle_t s_fsmHandle = {
    .state = FSM_STATE_INIT,
    .subState = FSM_SUBSTATE_NONE,
    .prevState = FSM_STATE_INIT,
    .prevSubState = FSM_SUBSTATE_NONE,
};

static SPP_SERVICE_ConsumerContract_t s_fsmConsumer = {
    .consumerID = 0,
    .priority = 0,
    .p_nameConsumer = "FSM",
    .tiemoutMs = 0,
    .suscribeToApid = 0,
    .p_mailBox = NULL,
    .init = NULL,
    .consumeData = NULL,
};


/* ----------------------------------------------------------------
 * PRIVATE FUNCTIONS
 * ---------------------------------------------------------------- */

/* Guards */
static spp_bool_t guard_healthcheckOk(void)
{
    return false;
}
static spp_bool_t guard_launchDetected(void)
{
    return false;
}
static spp_bool_t guard_apogeeDetected(void)
{
    return false;
}
static spp_bool_t guard_altitudeThreshold(void)
{
    return false;
}
static spp_bool_t guard_landingDetected(void)
{
    return false;
}

/* Actions */
static void action_onEnterReady(void)
{
}
static void action_onEnterAscending(void)
{
}
static void action_onEnterDescending(void)
{
}
static void action_onDeployChute(void)
{
}
static void action_onEnterLanded(void)
{
}


/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */

void FSM_tick(void)
{
    const spp_uint8_t tableSize = sizeof(s_transitionTable) / sizeof(s_transitionTable[0]);

    for (spp_uint8_t i = 0; i < tableSize; i++)
    {
        const FSM_Transition_t *p_t = &s_transitionTable[i];

        if (p_t->fromState != s_fsmHandle.state || p_t->fromSubState != s_fsmHandle.subState)
        {
            continue;
        }

        if (p_t->guard != NULL && !p_t->guard())
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

        break;
    }
}
