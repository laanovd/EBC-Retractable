/********************************************************************
 *    Controller.cpp
 *
 *    Retractable statemachine
 *
 ********************************************************************/
#include "Controller.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <StateMachineLib.h>
#include <endian.h>
#include <esp_err.h>

#include "Azimuth.h"
#include "CLI.h"
#include "DMC.h"
#include "GPIO.h"
#include "Storage.h"

#define DEBUG

/********************************************************************
 * Constants
 ********************************************************************/
#define SEC_TO_MS 1000

/********************************************************************
 * Type definitions
 ********************************************************************/
enum VEDOUTStateIds {
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
static JsonDocument controller_data;
static StateMachine stateMachine(10, 27);

/* timers */
static unsigned long preretracting_timer = 0;
static unsigned long retracting_timer = 0;
static unsigned long extending_timer = 0;
static unsigned long precalibrating_timer = 0;
static unsigned long calibrating_timer = 0;

/********************************************************************
 * Setup variables
 ********************************************************************/
static void CONTROLLER_init_float(const char *key, int default_value) {
  float flt;
  if (STORAGE_get_float(key, flt)) {
    flt = default_value;
    STORAGE_set_float(key, flt);
  }
  controller_data[key] = flt;
}

static void CONTROLLER_setup_variables(void) {
  CONTROLLER_init_float(JSON_RETRACTED_COUNT, 0);
  CONTROLLER_init_float(JSON_EXTENDED_COUNT, 0);
  CONTROLLER_init_float(JSON_MOVE_TIMEOUT, JSON_MOVE_TIMEOUT_DEFAULT);
  CONTROLLER_init_float(JSON_DELAY_TO_MIDDLE, JSON_DELAY_TO_MIDDLE_DEFAULT);
}

/********************************************************************
 * Timers
 ********************************************************************/
static void TIMER_start(unsigned long &timer, int timeout) {
  timer = millis() + (timeout * SEC_TO_MS);
}

static void TIMER_stop(unsigned long &timer) {
  timer = 0;
}

static bool TIMER_finished(unsigned long &timer) {
  if (timer && (millis() >= timer)) {
    TIMER_stop(timer);
    return true;
  }
  return false;
}

/********************************************************************
 * Controller Initializiation State
 ********************************************************************/
static void fnStateInit() {
#ifdef DEBUG
  Serial.println("State INIT");
#endif
  DMC_disable();
  RETRACTABLE_disable();
  MOTOR_UP_off();
  MOTOR_DOWN_off();
  AZIMUTH_disable();
}

static bool fnInitToCalibrating() {
  return true;
}

/********************************************************************
 * Controller Retracted State
 ********************************************************************/
static void fnStateRetracted() {
#ifdef DEBUG
  Serial.println("State RETRACTED");
#endif
  LED_UP_on();
  LED_DOWN_off();

  MOTOR_UP_off();

  controller_data[JSON_EXTENDED_COUNT] = controller_data[JSON_EXTENDED_COUNT].as<int>() + 1;
  STORAGE_set_int(JSON_EXTENDED_COUNT, controller_data[JSON_EXTENDED_COUNT].as<int>());
}

static bool fnRetractedToExtending() {
  if (BUTTON_DOWN_is_pressed())
    return true;

  return false;
}

static bool fnRetractedToNoPosition() {
  if (!RETRACTABLE_is_retracted()) {
    return true;
  }

  return false;
}

/********************************************************************
 * Controller Retracting Aligning State
 ********************************************************************/
static void fnStatePreretracting() {
#ifdef DEBUG
  Serial.println("State PRERETRACTING");
#endif
  LED_UP_set_interval(BLINK_INTERVAL_MOVING);
  LED_DOWN_off();

  TIMER_start(preretracting_timer, controller_data[JSON_DELAY_TO_MIDDLE].as<int>());
}

static bool fnPreretractingToNoPosition() {
  if (BUTTON_UP_is_pressed() || BUTTON_DOWN_is_pressed())
    return true;

  return false;
}

static bool fnPreretractingToRetracting() {
  if (TIMER_finished(preretracting_timer))
    return true;

  return false;
}

/********************************************************************
 * Controller Retracting State
 ********************************************************************/
static void fnStateRetracting() {
#ifdef DEBUG
  Serial.println("State RETRACTING");
#endif
  RETRACTABLE_enable();
  MOTOR_DOWN_on();

  TIMER_start(retracting_timer, controller_data[JSON_MOVE_TIMEOUT].as<int>());
}

static bool fnRetractingToNoPosition() {
  if (BUTTON_UP_is_pressed() || BUTTON_DOWN_is_pressed()) {
    TIMER_stop(retracting_timer);
    return true;
  }

  if (TIMER_finished(retracting_timer))
    return true;

  return false;
}

static bool fnRetractingToRetracted() {
  if (RETRACTABLE_is_retracted()) {
    MOTOR_DOWN_off();
    TIMER_stop(retracting_timer);
    return true;
  }

  return false;
}

/********************************************************************
 * Controller Extended State
 ********************************************************************/
static void fnStateExtended() {
#ifdef DEBUG
  Serial.println("State EXTENDED");
#endif
  LED_UP_off();
  LED_DOWN_on();

  MOTOR_DOWN_off();
  DMC_enable();
  AZIMUTH_enable();

  controller_data[JSON_RETRACTED_COUNT] = controller_data[JSON_RETRACTED_COUNT].as<int>() + 1;
  STORAGE_set_int(JSON_RETRACTED_COUNT, controller_data[JSON_RETRACTED_COUNT].as<int>());
}

static bool fnExtendedToPrecalibrating() {
  if (BUTTON_UP_is_pressed() && BUTTON_DOWN_is_pressed()) {
    AZIMUTH_disable();
    DMC_disable();
    return true;
  }

  return false;
}

static bool fnExtendedToRetracting() {
  if (BUTTON_UP_is_pressed() && !BUTTON_DOWN_is_pressed()) {
    DMC_disable();
    AZIMUTH_disable();
    return true;
  }

  return false;
}

static bool fnExtendedToNoPosition() {
  if (!RETRACTABLE_is_extended()) 
    return true;

  return false;
}

/********************************************************************
 * Controller Extending State
 ********************************************************************/
static void fnStateExtending() {
#ifdef DEBUG
  Serial.println("State EXTENDING");
#endif
  RETRACTABLE_enable();
  MOTOR_DOWN_on();

  LED_UP_off();
  LED_DOWN_set_interval(BLINK_INTERVAL_MOVING);

  TIMER_start(extending_timer, controller_data[JSON_MOVE_TIMEOUT].as<int>());
}

static bool fnExtendingToNoPosition() {
  if (BUTTON_UP_is_pressed() || BUTTON_DOWN_is_pressed()) {
    TIMER_stop(extending_timer);
    return true;
  }

  if (TIMER_finished(extending_timer))
    return true;

  return false;
}

static bool fnExtendingToExtended() {
  if (RETRACTABLE_is_extended()) {
    MOTOR_DOWN_off();
    TIMER_stop(extending_timer);
    return true;
  }

  return false;
}

/********************************************************************
 * Controller Pre-Calibration State
 ********************************************************************/
static void fnStatePrecalibrating() {
#ifdef DEBUG
  Serial.println("State PRECALIBRATING");
#endif
  DMC_disable();
  RETRACTABLE_disable();

  LED_UP_set_interval(BLINK_INTERVAL_CALIBRATING);
  LED_DOWN_set_interval(BLINK_INTERVAL_CALIBRATING);

  TIMER_start(precalibrating_timer, 5);
}

static bool fnPrecalibratingToExtended() {
  if (!BUTTON_UP_is_pressed() || !BUTTON_DOWN_is_pressed()) {
    TIMER_stop(precalibrating_timer);
    return true;
  }

  return false;
}

static bool fnPrecalibratingToCalibrating() {
  if (TIMER_finished(precalibrating_timer))
    return true;

  return false;
}

/********************************************************************
 * Controller Calibration State
 ********************************************************************/
static void fnStateCalibrating() {
#ifdef DEBUG
  Serial.println("State CALIBRATING");
#endif
  DMC_disable();
  RETRACTABLE_disable();

  LED_DOWN_set_interval(BLINK_INTERVAL_CALIBRATING);
  LED_UP_set_interval(BLINK_INTERVAL_CALIBRATING);

  set_calibrating(true);
  TIMER_start(calibrating_timer, 3);
}

static bool fnCalibratingToNoPosition() {
  if (BUTTON_UP_is_pressed() || BUTTON_DOWN_is_pressed()) {
    TIMER_stop(calibrating_timer);
    set_calibrating(false);
    return true;
  }

  if (TIMER_finished(calibrating_timer)) {
    set_calibrating(false);
    return true;
  }

  return false;
}

/********************************************************************
 * Controller No Position State
 ********************************************************************/
static void fnStateNoPosition() {
#ifdef DEBUG
  Serial.println("State NO POSITION");
#endif
  MOTOR_UP_off();
  MOTOR_DOWN_off();
  AZIMUTH_disable();
  DMC_disable();

  LED_UP_set_interval(BLINK_INTERVAL_NO_POSITION);
  LED_DOWN_set_interval(BLINK_INTERVAL_NO_POSITION);
  // TODO: Set error
}
static bool fnNoPositionToExtended() {
  if (RETRACTABLE_is_extended() && !RETRACTABLE_is_retracted())
    return true;

  return false;
}

static bool fnNoPositionToExtending() {
  if (BUTTON_DOWN_is_pressed())
    return true;

  return false;
}

static bool fnNoPositionToRetracted() {
  if (RETRACTABLE_is_retracted() && !RETRACTABLE_is_extended())
    return true;

  return false;
}

static bool fnNoPositionToRetracting() {
  if (BUTTON_UP_is_pressed())
    return true;

  return false;
}

/********************************************************************
 * Controller No Position State
 ********************************************************************/
static void fnStateEmergencyStop() {
#ifdef DEBUG
  Serial.println("State EMERGENCY STOP");
#endif
  DMC_disable();
  RETRACTABLE_disable();
  MOTOR_UP_off();
  MOTOR_DOWN_off();
  AZIMUTH_disable();

  LED_UP_set_interval(BLINK_INTERVAL_EMERGENCY);
  LED_DOWN_set_interval(BLINK_INTERVAL_EMERGENCY);

  // TODO: Set error emergency stop
}

static bool fnAnyToEmergencyStop() {
  if (EMERGENCY_STOP_active()) 
    return true;

  return false;
}

static bool fnEmergencyStopToCalibrating() {
  if (!EMERGENCY_STOP_active()) {
    // TODO: Clear error not calibrated
    return true;
  }

  return false;
}

/********************************************************************
 * Controller main task
 ********************************************************************/
static void CONTROLLER_main_task(void *parameter) {
  (void)parameter;
  while (true) {
    stateMachine.Update();

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

/********************************************************************
 * Setup Controller task(s)
 ********************************************************************/
static void CONTROLLER_setup_tasks() {
  xTaskCreate(CONTROLLER_main_task, "Controller debug task", 4096, NULL, 15, NULL);
}

/********************************************************************
 * Setup Controller State Machine
 ********************************************************************/
static void CONTROLLER_setup_statemachine() {
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
void CONTROLLER_setup() {
  CONTROLLER_setup_variables();
  CONTROLLER_setup_statemachine();
  CONTROLLER_setup_tasks();
}