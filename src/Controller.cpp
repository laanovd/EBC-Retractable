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
#include "GPIO.cpp"

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
 * Global constants
 ********************************************************************/
const int emergency_blink_interval = 250;
const int no_position_blink_interval = 500;
const int calibrating_blink_interval = 500;
const int moving_blink_interval = 500;
const int init_blink_interval = 500;

/********************************************************************
 * Global data
 ********************************************************************/
static StateMachine stateMachine(8, 20);

/********************************************************************
 * Controller Initializiation State
 ********************************************************************/
static void fnStateInit() {}
static bool fnInitToCalibrating()
{
    return true;
}

/********************************************************************
 * Controller Retracted State
 ********************************************************************/
static void fnStateRetracted()
{
    LED_DOWN_set_interval(-1);
    LED_UP_set_interval(0);
    // TODO: increment number of times up
}
static bool fnRetractedToExtending()
{
    // TODO: check if azimuth is in the middle position
}
static bool fnRetractedToEmergencyStop()
{
    return EMERGENCY_BUTTON_is_pressed();
}

/********************************************************************
 * Controller Retracting State
 ********************************************************************/
static void fnStateRetracting()
{
    LED_UP_set_interval(moving_blink_interval);
    LED_DOWN_set_interval(-1);
    // TODO: lock DMC
}
static bool fnRetractingToNoPosition() {}
static bool fnRetractingToRetracted() {}
static bool fnRetractingToEmergencyStop()
{
    return EMERGENCY_BUTTON_is_pressed();
}

/********************************************************************
 * Controller Extended State
 ********************************************************************/
static void fnStateExtended()
{
    LED_DOWN_set_interval(0);
    LED_UP_set_interval(-1);
    // TODO: Free DMC
    // TODO: increment number of times down
    // TODO: scale wheel position to voltage and send to azimuth
}
static bool fnExtendedToRetracting()
{
    // TODO: check if azimuth is in the middle position
}
static bool fnExtendedToCalibrating()
{
}
static bool fnExtendedToEmergencyStop()
{
    return EMERGENCY_BUTTON_is_pressed();
}

/********************************************************************
 * Controller Extending State
 ********************************************************************/
static void fnStateExtending()
{
    LED_DOWN_set_interval(moving_blink_interval);
    LED_UP_set_interval(-1);
}
static bool fnExtendingToNoPosition()
{
    // TODO: stop if up button pressed
    // TODO: check expiration timer
}
static bool fnExtendingToExtended() {}
static bool fnExtendingToEmergencyStop()
{
    return EMERGENCY_BUTTON_is_pressed();
}

/********************************************************************
 * Controller Calibration State
 ********************************************************************/
static void fnStateCalibrating()
{
    LED_DOWN_set_interval(calibrating_blink_interval);
    LED_UP_set_interval(calibrating_blink_interval);
    LED_ERROR_set_low();
    // TODO: control azimuth via cli
    // TODO: command "factory reset"
}
static bool fnCalibratingToNoPosition()
{
    // TODO: stop on any button press
}
static bool fnCalibratingToEmergencyStop()
{
    return EMERGENCY_BUTTON_is_pressed();
}

/********************************************************************
 * Controller No Position State
 ********************************************************************/
static void fnStateNoPosition()
{
    LED_UP_set_interval(no_position_blink_interval);
    LED_DOWN_set_interval(no_position_blink_interval);
    // TODO: lock DMC
    // TODO: give error
    LED_ERROR_set_high();
}
static bool fnNoPositionToExtended() {}
static bool fnNoPositionToExtending() {}
static bool fnNoPositionToRetracted() {}
static bool fnNoPositionToRetracting() {}
static bool fnNoPositionToEmergencyStop()
{
    return EMERGENCY_BUTTON_is_pressed();
}

/********************************************************************
 * Controller No Position State
 ********************************************************************/
static void fnStateEmergencyStop()
{
    LED_UP_set_interval(emergency_blink_interval);
    LED_DOWN_set_interval(emergency_blink_interval);
    // TODO: lock DMC
    // TODO: give error
    LED_ERROR_set_high();
}
static bool fnEmergencyStopToCalibrating(){}

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
