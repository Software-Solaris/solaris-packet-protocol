/**
 * @file fsm.h
 * @brief Finite State Machine (FSM) module for the SPP core.
 */

#ifndef SPP_CORE_FSM_H
#define SPP_CORE_FSM_H

#include "spp/core/returnTypes.h"
#include "spp/core/types.h"

/* ----------------------------------------------------------------
 * TYPES
 * ---------------------------------------------------------------- */

/**
 * @brief Top-level FSM states.
 */
typedef enum
{
    FSM_STATE_INIT = 0,
    FSM_STATE_READY,
    FSM_STATE_FLIGHT,
    FSM_STATE_LANDED,
} FSM_State_t;

/**
 * @brief FSM sub-states. Use FSM_SUBSTATE_NONE for states with no sub-state.
 */
typedef enum
{
    FSM_SUBSTATE_NONE = 0,
    FSM_SUBSTATE_FLIGHT_ASCENDING,
    FSM_SUBSTATE_FLIGHT_DESCENDING,
    FSM_SUBSTATE_FLIGHT_PARACHUTE_DEPLOYED,
} FSM_SubState_t;

/**
 * @brief FSM events. FSM_EVENT_NONE means the transition is guard-driven only.
 *        Add new events here to support forced transitions in the future.
 */
typedef enum
{
    FSM_EVENT_NONE = 0,
} FSM_Event_t;

/**
 * @brief A single row in the FSM transition table.
 */
typedef struct
{
    FSM_State_t fromState;
    FSM_SubState_t fromSubState;
    FSM_State_t toState;
    FSM_SubState_t toSubState;
    FSM_Event_t event;
    spp_bool_t (*guard)(void);
    void (*action)(void);
} FSM_Transition_t;

/**
 * @brief FSM handle — holds the current and previous state of the machine.
 */
typedef struct
{
    FSM_State_t state;
    FSM_SubState_t subState;
    FSM_State_t prevState;
    FSM_SubState_t prevSubState;
} FSM_Handle_t;


/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */

/**
 * @brief Evaluate the transition table and advance the FSM if a guard passes.
 *        Call this once per superloop tick.
 */
void FSM_tick(void);


#endif /* SPP_CORE_FSM_H */
