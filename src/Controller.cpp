/********************************************************************
 *    VEDOUT_RIM.ino
 *
 *    Data handling for Battery Management System (ved)
 *
 ********************************************************************/
#include "Controller.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <StateMachineLib.h>
#include <endian.h>
#include <esp_err.h>

#include "Storage.h"
#include "CLI.h"
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
    CONTROLLER_preretracting,
    CONTROLLER_extended,
    CONTROLLER_extending,
    CONTROLLER_no_position,
    CONTROLLER_precalibrating,
    CONTROLLER_calibrating,
    CONTROLLER_emergency_stop
};

/********************************************************************
 * Global variables
 ********************************************************************/

int timer_millis = 0;

/********************************************************************
 * Setup variables
 ********************************************************************/
static JsonDocument controller_data;

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
    CONTROLLER_init_int(JSON_MOVE_TIMEOUT, JSON_MOVE_TIMEOUT_DEFAULT);
}

/********************************************************************
 * Global data
 ********************************************************************/
static StateMachine stateMachine(10, 25);

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
        return true;

    return false;
}

/********************************************************************
 * Controller Retracting Aligning State
 ********************************************************************/
static void fnStatePreretracting()
{
    LED_UP_set_interval(BLINK_INTERVAL_MOVING);
    LED_DOWN_set_interval(-1);
    timer_millis = millis();
}

static bool fnPreretractingToNoPosition()
{
    if (BUTTON_UP_is_pressed())
        return true;
    if (BUTTON_DOWN_is_pressed())
        return true;

    return false;
}

static bool fnPreretractingToRetracting()
{
    if (millis() - timer_millis >= controller_data[JSON_DELAY_TO_MIDDLE] * 1000)
        return true;

    return false;
}

/********************************************************************
 * Controller Retracting State
 ********************************************************************/
static void fnStateRetracting()
{
    timer_millis = millis();
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
    if (BUTTON_UP_is_pressed(250))
    {
        DMC_disable();
        AZIMUTH_disable();
        return true;
    }

    return false;
}

static bool fnExtendedToPrecalibrating()
{
    if (BUTTON_UP_is_pressed() && BUTTON_DOWN_is_pressed())
    {
        AZIMUTH_disable();
        DMC_disable();
        return true;
    }

    return false;
}

/********************************************************************
 * Controller Extending State
 ********************************************************************/
static void fnStateExtending()
{
    LED_DOWN_set_interval(BLINK_INTERVAL_MOVING);
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

/********************************************************************
 * Controller Pre-Calibration State
 ********************************************************************/
static int double_press_timer;
static void fnStatePrecalibrating()
{
    double_press_timer = millis();
}

static bool fnPrecalibratingToExtended()
{
    if (!BUTTON_UP_is_pressed())
        return true;
    if (!BUTTON_DOWN_is_pressed())
        return true;

    return false;
}

static bool fnPrecalibratingToCalibrating()
{
    if (millis() - double_press_timer >= DOUBLE_PRESS_HOLD_TIME)
        return true;

    return false;
}

/********************************************************************
 * Controller Calibration State
 ********************************************************************/
static void fnStateCalibrating()
{
    set_calibrating(true);
    LED_DOWN_set_interval(BLINK_INTERVAL_CALIBRATING);
    LED_UP_set_interval(BLINK_INTERVAL_CALIBRATING);
    LED_ERROR_set_low();
}

static bool fnCalibratingToNoPosition()
{
    if (BUTTON_UP_is_pressed())
    {
        set_calibrating(false);
        return true;
    }
    if (BUTTON_DOWN_is_pressed())
    {
        set_calibrating(false);
        return true;
    }

    return false;
}

/********************************************************************
 * Controller No Position State
 ********************************************************************/
static void fnStateNoPosition()
{
    LED_UP_set_interval(BLINK_INTERVAL_NO_POSITION);
    LED_DOWN_set_interval(BLINK_INTERVAL_NO_POSITION);
    LED_ERROR_set_high();
}
static bool fnNoPositionToExtended()
{
    if (RETRACTABLE_is_extended())
        return true;

    return false;
}

static bool fnNoPositionToExtending()
{
    if (BUTTON_DOWN_is_pressed())
        return true;

    return false;
}

static bool fnNoPositionToRetracted()
{
    if (RETRACTABLE_is_retracted())
        return true;

    return false;
}

static bool fnNoPositionToRetracting()
{
    if (BUTTON_UP_is_pressed())
        return true;

    return false;
}

/********************************************************************
 * Controller No Position State
 ********************************************************************/
static void fnStateEmergencyStop()
{
    LED_UP_set_interval(BLINK_INTERVAL_EMERGENCY);
    LED_DOWN_set_interval(BLINK_INTERVAL_EMERGENCY);
    LED_ERROR_set_high();
}

static bool fnAnyToEmergencyStop()
{
    if (BUTTON_EMERGENCY_is_pressed())
    {
        DMC_disable();
        AZIMUTH_disable();
        return true;
    }

    return false;
}

static bool fnEmergencyStopToCalibrating()
{
    if (true)
    {
        LED_ERROR_set_low();
        return true;
    }

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
    stateMachine.AddTransition(CONTROLLER_retracting, CONTROLLER_emergency_stop, fnAnyToEmergencyStop);
    stateMachine.SetOnEntering(CONTROLLER_retracting, fnStateExtending);

    stateMachine.AddTransition(CONTROLLER_preretracting, CONTROLLER_retracted, fnPreretractingToRetracting);
    stateMachine.AddTransition(CONTROLLER_preretracting, CONTROLLER_no_position, fnPreretractingToNoPosition);
    stateMachine.AddTransition(CONTROLLER_preretracting, CONTROLLER_emergency_stop, fnAnyToEmergencyStop);
    stateMachine.SetOnEntering(CONTROLLER_preretracting, fnStatePreretracting);

    stateMachine.AddTransition(CONTROLLER_retracted, CONTROLLER_extending, fnRetractedToExtending);
    stateMachine.AddTransition(CONTROLLER_retracted, CONTROLLER_emergency_stop, fnAnyToEmergencyStop);
    stateMachine.SetOnEntering(CONTROLLER_retracted, fnStateRetracted);

    stateMachine.AddTransition(CONTROLLER_extending, CONTROLLER_extended, fnExtendingToExtended);
    stateMachine.AddTransition(CONTROLLER_extending, CONTROLLER_no_position, fnExtendingToNoPosition);
    stateMachine.AddTransition(CONTROLLER_extending, CONTROLLER_emergency_stop, fnAnyToEmergencyStop);
    stateMachine.SetOnEntering(CONTROLLER_extending, fnStateExtending);

    stateMachine.AddTransition(CONTROLLER_extended, CONTROLLER_preretracting, fnExtendedToRetracting);
    stateMachine.AddTransition(CONTROLLER_extended, CONTROLLER_precalibrating, fnExtendedToPrecalibrating);
    stateMachine.AddTransition(CONTROLLER_extended, CONTROLLER_emergency_stop, fnAnyToEmergencyStop);
    stateMachine.SetOnEntering(CONTROLLER_extended, fnStateExtended);

    stateMachine.AddTransition(CONTROLLER_calibrating, CONTROLLER_no_position, fnCalibratingToNoPosition);
    stateMachine.AddTransition(CONTROLLER_calibrating, CONTROLLER_emergency_stop, fnAnyToEmergencyStop);
    stateMachine.SetOnEntering(CONTROLLER_calibrating, fnStateCalibrating);

    stateMachine.AddTransition(CONTROLLER_precalibrating, CONTROLLER_extended, fnPrecalibratingToExtended);
    stateMachine.AddTransition(CONTROLLER_precalibrating, CONTROLLER_calibrating, fnPrecalibratingToCalibrating);
    stateMachine.SetOnEntering(CONTROLLER_precalibrating, fnStateCalibrating);

    stateMachine.AddTransition(CONTROLLER_no_position, CONTROLLER_extended, fnNoPositionToExtended);
    stateMachine.AddTransition(CONTROLLER_no_position, CONTROLLER_extending, fnNoPositionToExtending);
    stateMachine.AddTransition(CONTROLLER_no_position, CONTROLLER_retracted, fnNoPositionToRetracted);
    stateMachine.AddTransition(CONTROLLER_no_position, CONTROLLER_preretracting, fnNoPositionToRetracting);
    stateMachine.AddTransition(CONTROLLER_no_position, CONTROLLER_emergency_stop, fnAnyToEmergencyStop);
    stateMachine.SetOnEntering(CONTROLLER_no_position, fnStateNoPosition);

    stateMachine.AddTransition(CONTROLLER_emergency_stop, CONTROLLER_calibrating, fnEmergencyStopToCalibrating);
    stateMachine.SetOnEntering(CONTROLLER_emergency_stop, fnStateEmergencyStop);

    // Initial state
    stateMachine.SetState(CONTROLLER_init, false, true);
}
