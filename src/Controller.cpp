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

#include "Storage.h"
#include "GPIO.cpp"
#include "DMC.cpp"
#include "Azimuth.cpp"

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
 * Global variables
 ********************************************************************/
const int emergency_blink_interval = 250;
const int no_position_blink_interval = 500;
const int calibrating_blink_interval = 500;
const int moving_blink_interval = 500;
const int init_blink_interval = 500;

int timer_millis = 0;

/********************************************************************
 * Setup variables
 ********************************************************************/
static JsonDocument controller_data;
#define JSON_RETRACTED_COUNT "retracted"
#define JSON_EXTENDED_COUNT "extended"
#define JSON_MOVE_TIMEOUT "move-timeout"

static void CONTROLLER_init_int(const char *key, int default_value)
{
    int integer;
    if (STORAGE_get_int(key, integer))
    {
        integer = default_value;
        STORAGE_set_int(key, integer);
    }
    controller_data[key] = integer;
}

static void CONTROLLER_setup_variables(void)
{
    CONTROLLER_init_int(JSON_RETRACTED_COUNT, 0);
    CONTROLLER_init_int(JSON_EXTENDED_COUNT, 0);
    CONTROLLER_init_int(JSON_MOVE_TIMEOUT, 15);
}

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

    controller_data[JSON_EXTENDED_COUNT] = controller_data[JSON_EXTENDED_COUNT] + 1;
    STORAGE_set_int(JSON_EXTENDED_COUNT, controller_data[JSON_EXTENDED_COUNT]);
}
static bool fnRetractedToExtending()
{
    if (BUTTON_DOWN_is_pressed())
        // TODO: check if azimuth is in the middle position
        return true;

    return false;
}
static bool fnRetractedToEmergencyStop()
{
    if (BUTTON_EMERGENCY_is_pressed())
        return true;

    return false;
}

/********************************************************************
 * Controller Retracting State
 ********************************************************************/
static void fnStateRetracting()
{
    LED_UP_set_interval(moving_blink_interval);
    LED_DOWN_set_interval(-1);
    DMC_disable();
    timer_millis = millis();
    // TODO: disable analog out and wait x seconds
}

static bool fnRetractingToNoPosition()
{
    if (BUTTON_UP_is_pressed())
        return true;
    if (BUTTON_DOWN_is_pressed())
        return true;
    if (millis() - timer_millis >= controller_data[JSON_MOVE_TIMEOUT] * 1000)
        return true;

    return false;
}

static bool fnRetractingToRetracted()
{
    if (RETRACTABLE_is_retracted())
        return true;

    return false;
}

static bool fnRetractingToEmergencyStop()
{
    if (BUTTON_EMERGENCY_is_pressed())
        return true;

    return false;
}

/********************************************************************
 * Controller Extended State
 ********************************************************************/
static void fnStateExtended()
{
    LED_DOWN_set_interval(0);
    LED_UP_set_interval(-1);

    controller_data[JSON_RETRACTED_COUNT] = controller_data[JSON_RETRACTED_COUNT] + 1;
    STORAGE_set_int(JSON_RETRACTED_COUNT, controller_data[JSON_RETRACTED_COUNT]);

    DMC_enable();
    AZIMUTH_enable();
}

static bool fnExtendedToRetracting()
{
    if (BUTTON_UP_is_pressed())
        // TODO: check if azimuth is in the middle position
        return true;

    return false;
}

static bool fnExtendedToCalibrating()
{
    if (BUTTON_COMBINED_is_pressed())
        return true;

    return false;
}

static bool fnExtendedToEmergencyStop()
{
    if (BUTTON_EMERGENCY_is_pressed())
        return true;

    return false;
}

/********************************************************************
 * Controller Extending State
 ********************************************************************/
static void fnStateExtending()
{
    LED_DOWN_set_interval(moving_blink_interval);
    LED_UP_set_interval(-1);
    timer_millis = millis();
}

static bool fnExtendingToNoPosition()
{
    if (BUTTON_UP_is_pressed())
        return true;
    if (BUTTON_DOWN_is_pressed())
        return true;
    if (millis() - timer_millis >= controller_data[JSON_MOVE_TIMEOUT] * 1000)
        return true;

    return false;
}

static bool fnExtendingToExtended()
{
    if (RETRACTABLE_is_extended())
        return true;

    return false;
}

static bool fnExtendingToEmergencyStop()
{
    if (BUTTON_EMERGENCY_is_pressed())
        return true;

    return false;
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
    if (BUTTON_UP_is_pressed())
        return true;
    if (BUTTON_DOWN_is_pressed())
        return true;

    return false;
}

static bool fnCalibratingToEmergencyStop()
{
    if (BUTTON_EMERGENCY_is_pressed())
        return true;

    return false;
}

/********************************************************************
 * Controller No Position State
 ********************************************************************/
static void fnStateNoPosition()
{
    LED_UP_set_interval(no_position_blink_interval);
    LED_DOWN_set_interval(no_position_blink_interval);
    DMC_disable();
    // TODO: give error
    LED_ERROR_set_high();
}
static bool fnNoPositionToExtended()
{
    if (RETRACTABLE_is_extended())
        return true;

    return false;
}

static bool fnNoPositionToExtending() {}

static bool fnNoPositionToRetracted()
{
    if (RETRACTABLE_is_retracted())
        return true;

    return false;
}

static bool fnNoPositionToRetracting() {}

static bool fnNoPositionToEmergencyStop()
{
    if (BUTTON_EMERGENCY_is_pressed())
        return true;

    return false;
}

/********************************************************************
 * Controller No Position State
 ********************************************************************/
static void fnStateEmergencyStop()
{
    LED_UP_set_interval(emergency_blink_interval);
    LED_DOWN_set_interval(emergency_blink_interval);
    DMC_disable();
    // TODO: give error
    LED_ERROR_set_high();
}
static bool fnEmergencyStopToCalibrating()
{
    if (BUTTON_EMERGENCY_is_pressed())
        return true;

    return false;
}

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
