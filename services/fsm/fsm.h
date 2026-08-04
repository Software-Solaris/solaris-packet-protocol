/**
 * @file fsm.h
 * @brief Finite State Machine (FSM) module for the SPP core.
 */

#ifndef SPP_CORE_FSM_H
#define SPP_CORE_FSM_H

#include "spp/core/returnTypes.h"
#include "spp/core/types.h"

/* ----------------------------------------------------------------
* DEFINES
* ---------------------------------------------------------------- */
#define K_FSM_TIMEOUT_MS 10000U // Only executed once, np with stops


/* ----------------------------------------------------------------
* STRUCTS
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
 * @brief A single row in the FSM transition table.
 */
typedef struct
{
    FSM_State_t fromState;
    FSM_SubState_t fromSubState;
    FSM_State_t toState;
    FSM_SubState_t toSubState;
    spp_bool_t (*guard)(void);
    void (*action)(void);
    void (*stateFunction)(void);
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


typedef union
{
    spp_uint16_t errors;
    struct
    {
        spp_uint16_t fsmInitError           : 1;
        spp_uint16_t halInitError           : 1;
        spp_uint16_t bmpPubsubError         : 1;
        spp_uint16_t icmPubsubError         : 1;
        spp_uint16_t datalogggerPubsubError : 1;
        spp_uint16_t e22mbl01PubsubError    : 1;
        spp_uint16_t bmpInitError           : 1;
        spp_uint16_t icmInitError           : 1;
        spp_uint16_t dataloggerInitError    : 1;
        spp_uint16_t e22mbl01InitError      : 1;
        spp_uint16_t reserved               : 6;
    };
} FsmErrors_t;


/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */
/**
* @brief    Get the pointer to the FSM errors bit.
* @return   Pointer to the FSM errors bit.
*/
FsmErrors_t *SPP_CORE_FSM_getErrorsBit(void);
/**
* @brief    Initialize the FSM.
* @param    p_halPorts  Pointer to the HAL ports.
*/
void FSM_init(void *p_halPorts);
/**
 * @brief Evaluate the transition table and advance the FSM if a guard passes.
 *        Call this once per superloop tick.
 */
void FSM_tick(void);


#endif /* SPP_CORE_FSM_H */
