/********************************************************************
 *    VEDOUT_RIM.ino
 *
 *    Data handling for Battery Management System (ved)
 *
 ********************************************************************/

#include <Arduino.h>
#include <ArduinoJson.h>
#include <StateMachineLib.h>
#include <endian.h>
#include <esp_err.h>

/********************************************************************
 * Type definitions
 ********************************************************************/
enum VEDOUTStateIds
{
    CONTROLLER_init,
    CONTROLLER_retracted,
    CONTROLLER_retracting,
    CONTROLLER_extended,
    CONTROLLER_extending,
    CONTROLLER_no_position,
    CONTROLLER_calibrating
};

/********************************************************************
 * Global data
 ********************************************************************/
static StateMachine stateMachine(7, 9);

/********************************************************************
 * Controller Initializiation State
 ********************************************************************/
static void fnStateInit()
{
    // TODO: Blink LED
}
static bool fnInitToRetracted()
{
    return true;
}

/********************************************************************
 * Controller Retracted State
 ********************************************************************/
static void fnStateRetracted()
{
    // TODO: up down led on
    // TODO: increment number of times up
}
static bool fnRetractedToExtending()
{
    // TODO: check if azimuth is in the middle position
}

/********************************************************************
 * Controller Retracting State
 ********************************************************************/
static void fnStateRetracting()
{
    // TODO: lock DMC
    // TODO: stop if down button pressed
}
static bool fnRetractingToNoPosition() {}
static bool fnRetractingToRetracted() {}

/********************************************************************
 * Controller Extended State
 ********************************************************************/
static void fnStateExtended()
{
    // TODO: turn down led on
    // TODO: Free DMC
    // TODO: increment number of times down
    // TODO: scale wheel position to voltage and send to azimuth
    // TODO: check if both buttons are long pressed for callibration
}
static bool fnExtendedToRetracting()
{
    // TODO: check if azimuth is in the middle position
}
static bool fnExtendedToCalibrating() {}

/********************************************************************
 * Controller Extending State
 ********************************************************************/
static void fnStateExtending()
{
    // TODO: stop if up button pressed
}
static bool fnExtendingToNoPosition() {}
static bool fnExtendingToExtended() {}

/********************************************************************
 * Controller Calibration State
 ********************************************************************/
static void fnStateCalibrating() {
    // TODO: control azimuth via cli
    // TODO: stop on any button press
    // TODO: command "factory reset"
}
static bool fnCalibratingToExtended() {}

/********************************************************************
 * Controller No Position State
 ********************************************************************/
static void fnStateNoPosition()
{
    // TODO: slow blinking LEDs
    // TODO: lock DMC
    // TODO: give error
}

/********************************************************************
 * Setup Controller State Machine
 ********************************************************************/
void CONTROLLER_setup_statemachine()
{

    stateMachine.AddTransition(CONTROLLER_init, CONTROLLER_retracted, fnInitToRetracted);
    stateMachine.SetOnEntering(CONTROLLER_init, fnStateInit);

    stateMachine.AddTransition(CONTROLLER_retracting, CONTROLLER_retracted, fnRetractingToRetracted);
    stateMachine.AddTransition(CONTROLLER_retracting, CONTROLLER_no_position, fnRetractingToNoPosition);
    stateMachine.SetOnEntering(CONTROLLER_retracting, fnStateExtending);

    stateMachine.AddTransition(CONTROLLER_retracted, CONTROLLER_extending, fnRetractedToExtending);
    stateMachine.SetOnEntering(CONTROLLER_retracted, fnStateRetracted);

    stateMachine.AddTransition(CONTROLLER_extending, CONTROLLER_extended, fnExtendingToExtended);
    stateMachine.AddTransition(CONTROLLER_extending, CONTROLLER_no_position, fnExtendingToNoPosition);
    stateMachine.SetOnEntering(CONTROLLER_extending, fnStateExtending);

    stateMachine.AddTransition(CONTROLLER_extended, CONTROLLER_retracting, fnExtendedToRetracting);
    stateMachine.AddTransition(CONTROLLER_extended, CONTROLLER_calibrating, fnExtendedToCalibrating);
    stateMachine.SetOnEntering(CONTROLLER_extended, fnStateExtended);

    stateMachine.AddTransition(CONTROLLER_calibrating, CONTROLLER_extended, fnCalibratingToExtended);
    stateMachine.SetOnEntering(CONTROLLER_calibrating, fnStateCalibrating);

    stateMachine.SetOnEntering(CONTROLLER_no_position, fnStateNoPosition);

    // Initial state
    stateMachine.SetState(CONTROLLER_init, false, true);
}
