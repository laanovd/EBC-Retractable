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
    CONTROLLER_calibrating,
    CONTROLLER_emergency_stop
};

/********************************************************************
 * Global data
 ********************************************************************/
static StateMachine stateMachine(8, 20);

/********************************************************************
 * Controller Initializiation State
 ********************************************************************/
static void fnStateInit()
{
    // TODO: Blink LED
    // ? Blink leds here or in main file?
}
static bool fnInitToCalibrating()
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
static bool fnRetractedToEmergencyStop() {}

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
static bool fnRetractingToEmergencyStop() {}


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
static bool fnExtendedToEmergencyStop() {}


/********************************************************************
 * Controller Extending State
 ********************************************************************/
static void fnStateExtending()
{
    // TODO: stop if up button pressed
}
static bool fnExtendingToNoPosition() {}
static bool fnExtendingToExtended() {}
static bool fnExtendingToEmergencyStop() {}


/********************************************************************
 * Controller Calibration State
 ********************************************************************/
static void fnStateCalibrating() {
    // TODO: control azimuth via cli
    // TODO: stop on any button press
    // TODO: command "factory reset"
}
static bool fnCalibratingToEmergencyStop() {}
static bool fnCalibratingToNoPosition() {}


/********************************************************************
 * Controller No Position State
 ********************************************************************/
static void fnStateNoPosition()
{
    // TODO: slow blinking LEDs
    // TODO: lock DMC
    // TODO: give error
}
static bool fnNoPositionToExtended() {} // ? This should also be possible right?
static bool fnNoPositionToExtending() {}
static bool fnNoPositionToRetracted() {} // ? This should also be possible right?
static bool fnNoPositionToRetracting() {}
static bool fnNoPositionToEmergencyStop() {}


/********************************************************************
 * Controller No Position State
 ********************************************************************/
static void fnStateEmergencyStop()
{
    // TODO: slow blinking LEDs
    // TODO: lock DMC
    // TODO: give error
}
static bool fnEmergencyStopToCalibrating() {}


/********************************************************************
 * Setup Controller State Machine
 ********************************************************************/
void CONTROLLER_setup_statemachine()
{

    stateMachine.AddTransition(CONTROLLER_init, CONTROLLER_retracted, fnInitToCalibrating);
    stateMachine.SetOnEntering(CONTROLLER_init, fnStateInit);

    stateMachine.AddTransition(CONTROLLER_retracting, CONTROLLER_retracted, fnRetractingToRetracted);
    stateMachine.AddTransition(CONTROLLER_retracting, CONTROLLER_no_position, fnRetractingToNoPosition);
    stateMachine.AddTransition(CONTROLLER_retracting, CONTROLLER_emergency_stop, fnRetractingToEmergencyStop);
    stateMachine.SetOnEntering(CONTROLLER_retracting, fnStateExtending);

    stateMachine.AddTransition(CONTROLLER_retracted, CONTROLLER_extending, fnRetractedToExtending);
    stateMachine.AddTransition(CONTROLLER_retracted, CONTROLLER_emergency_stop, fnRetractedToEmergencyStop);
    stateMachine.SetOnEntering(CONTROLLER_retracted, fnStateRetracted);

    stateMachine.AddTransition(CONTROLLER_extending, CONTROLLER_extended, fnExtendingToExtended);
    stateMachine.AddTransition(CONTROLLER_extending, CONTROLLER_no_position, fnExtendingToNoPosition);
    stateMachine.AddTransition(CONTROLLER_extending, CONTROLLER_emergency_stop, fnExtendingToEmergencyStop);
    stateMachine.SetOnEntering(CONTROLLER_extending, fnStateExtending);

    stateMachine.AddTransition(CONTROLLER_extended, CONTROLLER_retracting, fnExtendedToRetracting);
    stateMachine.AddTransition(CONTROLLER_extended, CONTROLLER_calibrating, fnExtendedToCalibrating);
    stateMachine.AddTransition(CONTROLLER_extended, CONTROLLER_emergency_stop, fnExtendedToEmergencyStop);
    stateMachine.SetOnEntering(CONTROLLER_extended, fnStateExtended);

    stateMachine.AddTransition(CONTROLLER_calibrating, CONTROLLER_no_position, fnCalibratingToNoPosition);
    stateMachine.AddTransition(CONTROLLER_calibrating, CONTROLLER_emergency_stop, fnCalibratingToEmergencyStop);
    stateMachine.SetOnEntering(CONTROLLER_calibrating, fnStateCalibrating);

    stateMachine.AddTransition(CONTROLLER_no_position, CONTROLLER_extended, fnNoPositionToEmergencyStop);
    stateMachine.AddTransition(CONTROLLER_no_position, CONTROLLER_extending, fnNoPositionToEmergencyStop);
    stateMachine.AddTransition(CONTROLLER_no_position, CONTROLLER_retracted, fnNoPositionToEmergencyStop);
    stateMachine.AddTransition(CONTROLLER_no_position, CONTROLLER_retracting, fnNoPositionToEmergencyStop);
    stateMachine.AddTransition(CONTROLLER_no_position, CONTROLLER_emergency_stop, fnNoPositionToEmergencyStop);
    stateMachine.SetOnEntering(CONTROLLER_no_position, fnStateNoPosition);

    stateMachine.AddTransition(CONTROLLER_emergency_stop, CONTROLLER_calibrating, fnEmergencyStopToCalibrating);
    stateMachine.SetOnEntering(CONTROLLER_emergency_stop, fnStateEmergencyStop);

    // Initial state
    stateMachine.SetState(CONTROLLER_init, false, true);
}
