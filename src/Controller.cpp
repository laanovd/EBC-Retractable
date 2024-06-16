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
#include "GPIO.h"
#include "DMC.h"
#include "Azimuth.h"

#define DEBUG

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

static void CONTROLLER_init_float(const char *key, int default_value)
{
    float flt;
    if (STORAGE_get_float(key, flt))
    {
        flt = default_value;
        STORAGE_set_float(key, flt);
    }
    controller_data[key] = flt;
}

static void CONTROLLER_setup_variables(void)
{
    CONTROLLER_init_float(JSON_RETRACTED_COUNT, 0);
    CONTROLLER_init_float(JSON_EXTENDED_COUNT, 0);
    CONTROLLER_init_float(JSON_MOVE_TIMEOUT, JSON_MOVE_TIMEOUT_DEFAULT);
    CONTROLLER_init_float(JSON_DELAY_TO_MIDDLE, JSON_DELAY_TO_MIDDLE_DEFAULT);
}

/********************************************************************
 * Global data
 ********************************************************************/
static StateMachine stateMachine(10, 27);

/********************************************************************
 * Controller Initializiation State
 ********************************************************************/
static void fnStateInit()
{
#ifdef DEBUG
    Serial.println("State INIT");
#endif
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
#ifdef DEBUG
    Serial.println("State RETRACTED");
#endif
    LED_UP_on();
    LED_DOWN_off();

    controller_data[JSON_EXTENDED_COUNT] = int(controller_data[JSON_EXTENDED_COUNT]) + 1;
    STORAGE_set_int(JSON_EXTENDED_COUNT, controller_data[JSON_EXTENDED_COUNT]);
}

static bool fnRetractedToExtending()
{
    if (BUTTON_DOWN_is_pressed())
        return true;

    return false;
}

static bool fnRetractedToNoPosition()
{
    if (!RETRACTABLE_is_retracted())
    {
        return true;
    }

    return false;
}

/********************************************************************
 * Controller Retracting Aligning State
 ********************************************************************/
static int preretracting_timer = 0;
static void fnStatePreretracting()
{
#ifdef DEBUG
    Serial.println("State PRERETRACTING");
#endif
    LED_UP_set_interval(BLINK_INTERVAL_MOVING);
    LED_DOWN_off();
    preretracting_timer = millis();
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
    if (millis() - preretracting_timer >=  (int) controller_data[JSON_DELAY_TO_MIDDLE] * 1000)
        return true;

    return false;
}

/********************************************************************
 * Controller Retracting State
 ********************************************************************/
static void fnStateRetracting()
{
#ifdef DEBUG
    Serial.println("State RETRACTING");
#endif
    timer_millis = millis();
}

static bool fnRetractingToNoPosition()
{
    if (BUTTON_UP_is_pressed())
        return true;
    if (BUTTON_DOWN_is_pressed())
        return true;
    if (millis() - timer_millis >= int(controller_data[JSON_MOVE_TIMEOUT]) * 1000)
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
#ifdef DEBUG
    Serial.println("State EXTENDED");
#endif
    LED_UP_off();
    LED_DOWN_on();

    controller_data[JSON_RETRACTED_COUNT] = int(controller_data[JSON_RETRACTED_COUNT]) + 1;
    STORAGE_set_int(JSON_RETRACTED_COUNT, controller_data[JSON_RETRACTED_COUNT]);

    DMC_enable();
    AZIMUTH_enable();
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

static bool fnExtendedToRetracting()
{
    if (BUTTON_UP_is_pressed() && !BUTTON_DOWN_is_pressed())
    {
        DMC_disable();
        AZIMUTH_disable();
        return true;
    }

    return false;
}

static bool fnExtendedToNoPosition()
{
    if (!RETRACTABLE_is_extended())
    {
        DMC_disable();
        AZIMUTH_disable();
        return true;
    }

    return false;
}

/********************************************************************
 * Controller Extending State
 ********************************************************************/
static void fnStateExtending()
{
#ifdef DEBUG
    Serial.println("State EXTENDING");
#endif
    LED_UP_off();
    LED_DOWN_set_interval(BLINK_INTERVAL_MOVING);
    timer_millis = millis();
}

static bool fnExtendingToNoPosition()
{
    if (BUTTON_UP_is_pressed())
        return true;
    if (BUTTON_DOWN_is_pressed())
        return true;
    if (millis() - timer_millis >= int(controller_data[JSON_MOVE_TIMEOUT]) * 1000)
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
static int precalibrating_timer =0;
static void fnStatePrecalibrating()
{
#ifdef DEBUG
    Serial.println("State PRECALIBRATING");
#endif
    precalibrating_timer = millis();
    LED_UP_set_interval(BLINK_INTERVAL_CALIBRATING);
    LED_DOWN_set_interval(BLINK_INTERVAL_CALIBRATING);
}

static bool fnPrecalibratingToExtended()
{
    if (!BUTTON_UP_is_pressed() && millis() - precalibrating_timer < 5000)
        return true;
    if (!BUTTON_DOWN_is_pressed() && millis() - precalibrating_timer < 5000)
        return true;

    return false;
}

static bool fnPrecalibratingToCalibrating()
{
    if (!BUTTON_UP_is_pressed() && !BUTTON_DOWN_is_pressed() && millis() - precalibrating_timer >= 5000)
        return true;

    return false;
}

/********************************************************************
 * Controller Calibration State
 ********************************************************************/
static int calibrating_timer;
static void fnStateCalibrating()
{
#ifdef DEBUG
    Serial.println("State CALIBRATING");
#endif
    calibrating_timer = millis();
    set_calibrating(true);
    LED_DOWN_set_interval(BLINK_INTERVAL_CALIBRATING);
    LED_UP_set_interval(BLINK_INTERVAL_CALIBRATING);
    LED_ERROR_off();
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
    if (millis() - calibrating_timer >= 3 * 1000)
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
#ifdef DEBUG
    Serial.println("State NO POSITION");
#endif
    LED_UP_set_interval(BLINK_INTERVAL_NO_POSITION);
    LED_DOWN_set_interval(BLINK_INTERVAL_NO_POSITION);
    LED_ERROR_on();
}
static bool fnNoPositionToExtended()
{
    if (RETRACTABLE_is_extended() && !RETRACTABLE_is_retracted())
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
    if (RETRACTABLE_is_retracted() && !RETRACTABLE_is_extended())
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
#ifdef DEBUG
    Serial.println("State EMERGENCY STOP");
#endif
    LED_UP_set_interval(BLINK_INTERVAL_EMERGENCY);
    LED_DOWN_set_interval(BLINK_INTERVAL_EMERGENCY);
    LED_ERROR_on();
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
        LED_ERROR_off();
        return true;
    }

    return false;
}

/********************************************************************
 * Setup Controller State Machine
 ********************************************************************/
static void CONTROLLER_main_task(void *parameter)
{
    (void)parameter;
    while (true)
    {
        stateMachine.Update();

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

static void CONTROLLER_setup_tasks()
{
    xTaskCreate(CONTROLLER_main_task, "Controller debug task", 4096, NULL, 15, NULL);
}

/********************************************************************
 * Setup Controller State Machine
 ********************************************************************/
static void CONTROLLER_setup_statemachine()
{
    stateMachine.AddTransition(CONTROLLER_init, CONTROLLER_calibrating, fnInitToCalibrating);
    stateMachine.SetOnEntering(CONTROLLER_init, fnStateInit);

    stateMachine.AddTransition(CONTROLLER_retracting, CONTROLLER_retracted, fnRetractingToRetracted);
    stateMachine.AddTransition(CONTROLLER_retracting, CONTROLLER_no_position, fnRetractingToNoPosition);
    stateMachine.AddTransition(CONTROLLER_retracting, CONTROLLER_emergency_stop, fnAnyToEmergencyStop);
    stateMachine.SetOnEntering(CONTROLLER_retracting, fnStateRetracting);

    stateMachine.AddTransition(CONTROLLER_preretracting, CONTROLLER_retracting, fnPreretractingToRetracting);
    stateMachine.AddTransition(CONTROLLER_preretracting, CONTROLLER_no_position, fnPreretractingToNoPosition);
    stateMachine.AddTransition(CONTROLLER_preretracting, CONTROLLER_emergency_stop, fnAnyToEmergencyStop);
    stateMachine.SetOnEntering(CONTROLLER_preretracting, fnStatePreretracting);

    stateMachine.AddTransition(CONTROLLER_retracted, CONTROLLER_extending, fnRetractedToExtending);
    stateMachine.AddTransition(CONTROLLER_retracted, CONTROLLER_no_position, fnRetractedToNoPosition);
    stateMachine.AddTransition(CONTROLLER_retracted, CONTROLLER_emergency_stop, fnAnyToEmergencyStop);
    stateMachine.SetOnEntering(CONTROLLER_retracted, fnStateRetracted);

    stateMachine.AddTransition(CONTROLLER_extending, CONTROLLER_extended, fnExtendingToExtended);
    stateMachine.AddTransition(CONTROLLER_extending, CONTROLLER_no_position, fnExtendingToNoPosition);
    stateMachine.AddTransition(CONTROLLER_extending, CONTROLLER_emergency_stop, fnAnyToEmergencyStop);
    stateMachine.SetOnEntering(CONTROLLER_extending, fnStateExtending);

    stateMachine.AddTransition(CONTROLLER_extended, CONTROLLER_preretracting, fnExtendedToRetracting);
    stateMachine.AddTransition(CONTROLLER_extended, CONTROLLER_precalibrating, fnExtendedToPrecalibrating);
    stateMachine.AddTransition(CONTROLLER_extended, CONTROLLER_no_position, fnExtendedToNoPosition);
    stateMachine.AddTransition(CONTROLLER_extended, CONTROLLER_emergency_stop, fnAnyToEmergencyStop);
    stateMachine.SetOnEntering(CONTROLLER_extended, fnStateExtended);

    stateMachine.AddTransition(CONTROLLER_calibrating, CONTROLLER_no_position, fnCalibratingToNoPosition);
    stateMachine.AddTransition(CONTROLLER_calibrating, CONTROLLER_emergency_stop, fnAnyToEmergencyStop);
    stateMachine.SetOnEntering(CONTROLLER_calibrating, fnStateCalibrating);

    stateMachine.AddTransition(CONTROLLER_precalibrating, CONTROLLER_extended, fnPrecalibratingToExtended);
    stateMachine.AddTransition(CONTROLLER_precalibrating, CONTROLLER_calibrating, fnPrecalibratingToCalibrating);
    stateMachine.SetOnEntering(CONTROLLER_precalibrating, fnStatePrecalibrating);

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

/********************************************************************
 * Setup Controller
 ********************************************************************/
void CONTROLLER_setup()
{
    CONTROLLER_setup_variables();
    CONTROLLER_setup_statemachine();
    CONTROLLER_setup_tasks();
}