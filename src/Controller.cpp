/*******************************************************************
 *    Controller.cpp
 *
 *    Retractable/lift statemachine
 *
 *******************************************************************/
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
#include "Lift.h"
#include "Maintenance.h"
#include "Storage.h"

/*******************************************************************
 * Constants
 *******************************************************************/
#define DEBUG_CONTROLLER

#define SEC_TO_MS 1000

/*******************************************************************
 * Storage keys and defaults
 *******************************************************************/
#define JSON_MOVE_TIMEOUT "move_timeout"
#define JSON_MOVE_TIMEOUT_DEFAULT 20

/*******************************************************************
 * Type definitions
 *******************************************************************/
enum VEDOUTStateIds {
  CONTROLLER_init,
  CONTROLLER_retracted,
  CONTROLLER_retracting,
  CONTROLLER_homing,
  CONTROLLER_extended,
  CONTROLLER_extending,
  CONTROLLER_no_position,
  CONTROLLER_emergency_stop,
  CONTROLLER_maintenance
};

/*******************************************************************
 * Global variables
 *******************************************************************/
static JsonDocument controller_data;
static StateMachine stateMachine(9, 26);

/* timers */
static unsigned long Homing_timer = 0;
static unsigned long retracting_timer = 0;
static unsigned long extending_timer = 0;
static unsigned long precalibrating_timer = 0;
static unsigned long calibrating_timer = 0;

static bool request_maintenance_enable = false;

/*******************************************************************
 * Timers
 *******************************************************************/
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

/*******************************************************************
 * Maintenance mode
 *******************************************************************/
void CONTROLLER_request_maintenance(void) {
  request_maintenance_enable = true;
}

static bool CONTROLLER_maintenance_requested(void) {
  if (request_maintenance_enable) {
    request_maintenance_enable = false;
    return true;
  }
  return false;
}

/*******************************************************************
 * Controller Initializiation State
 *******************************************************************/
static void fnStateInit() {
#ifdef DEBUG_CONTROLLER
  Serial.println("Enter state: INIT.");
#endif
  DMC_disable();
  AZIMUTH_disable();
  AZIMUTH_analog_disable();

  LIFT_disable();
  LIFT_UP_off();
  LIFT_DOWN_off();
}

static bool fnInitToNoPosition() {
  return true;
}

/*******************************************************************
 * Controller Retracting Aligning State
 *******************************************************************/
static void fnStateHoming() {
#ifdef DEBUG_CONTROLLER
  Serial.println("Enter state: HOMING");
#endif
  DMC_disable();
  AZIMUTH_disable();
  AZIMUTH_analog_disable();
  TIMER_start(Homing_timer, AZIMUTH_get_to_middle_timeout());
}

static bool fnHomingToNoPosition() {
  if (TIMER_finished(Homing_timer)) {
    return true;
  }
  return false;
}

static bool fnHomingToRetracting() {
#ifdef DEBUG_CONTROLLER
  Serial.printf("Azimuth at home position: %s.\n", AZIMUTH_home() ? "yes" : "no");
#endif

  if (AZIMUTH_home()) {
    TIMER_stop(Homing_timer);
    return true;
  }
  return false;
}

/*******************************************************************
 * Azimuth homing
 *******************************************************************/
static void fnStateRetracting() {
#ifdef DEBUG_CONTROLLER
  Serial.println("Enter state: RETRACTING");
#endif
  DMC_disable();
  AZIMUTH_disable();
  AZIMUTH_analog_disable();

  LIFT_enable();
  LIFT_UP_on();
  LIFT_DOWN_off();

  TIMER_start(retracting_timer, LIFT_move_timeout());
}

static bool fnRetractingToNoPosition() {
  /* LIFT not UP in time */
  if (TIMER_finished(retracting_timer)) {
    LIFT_UP_off();
    return true;
  }
  return false;
}

static bool fnRetractingToRetracted() {
  if (LIFT_UP_sensor()) {
    LIFT_UP_off();
    TIMER_stop(retracting_timer);
    return true;
  }
  return false;
}

/*******************************************************************
 * Controller Retracted State
 *******************************************************************/
static void fnStateRetracted() {
#ifdef DEBUG_CONTROLLER
  Serial.println("Enter state: RETRACTED");
#endif
  DMC_disable();
  LIFT_disable();
  LIFT_UP_off();
  LIFT_DOWN_off();

  LIFT_retected_increment();
}

static bool fnRetractedToExtending() {
  if (LIFT_DOWN_button()) {
    return true;
  }
  return false;
}

static bool fnRetractedToNoPosition() {
  if (!LIFT_UP_sensor()) {
    return true;
  }
  return false;
}

/*******************************************************************
 * Controller Extending State
 *******************************************************************/
static void fnStateExtending() {
#ifdef DEBUG_CONTROLLER
  Serial.println("Enter state: EXTENDING");
#endif
  DMC_disable();
  AZIMUTH_disable();
  AZIMUTH_analog_disable();

  LIFT_enable();
  LIFT_UP_off();
  LIFT_DOWN_on();

  TIMER_start(extending_timer, LIFT_move_timeout());
}

static bool fnExtendingToNoPosition() {
  /* LIFT not down in time */
  if (TIMER_finished(extending_timer)) {
    LIFT_DOWN_off();
    return true;
  }
  return false;
}

static bool fnExtendingToExtended() {
  if (LIFT_DOWN_sensor()) {
    LIFT_DOWN_off();
    TIMER_stop(extending_timer);
    return true;
  }
  return false;
}

/*******************************************************************
 * Controller Extended State
 *******************************************************************/
static void fnStateExtended() {
#ifdef DEBUG_CONTROLLER
  Serial.println("Enter state: EXTENDED");
#endif
  LIFT_UP_off();
  LIFT_DOWN_off();

  DMC_enable();

  AZIMUTH_analog_enable();
  vTaskDelay(500 / portTICK_PERIOD_MS);
  AZIMUTH_enable();

  LIFT_extended_increment();
}

static bool fnExtendedToRetracting() {
  if (LIFT_UP_button()) {
    return true;
  }
  return false;
}

static bool fnExtendedToNoPosition() {
  if (!LIFT_DOWN_sensor()) {
    return true;
  }
  return false;
}

/*******************************************************************
 * Controller No Position State
 *******************************************************************/
static void fnStateNoPosition() {
#ifdef DEBUG_CONTROLLER
  Serial.println("Enter state: NO-POSITION");
#endif
  LIFT_UP_off();
  LIFT_DOWN_off();

  /* If lift not down the block DMC and AZIMUTH */
  if (!LIFT_DOWN_sensor()) {
      DMC_disable();
      AZIMUTH_disable();
      AZIMUTH_analog_disable();
  }
}

static bool fnNoPositionToExtended() {
  if (LIFT_DOWN_sensor() && !LIFT_UP_sensor())
    return true;

  return false;
}

static bool fnNoPositionToExtending() {
  if (LIFT_DOWN_button())
    return true;

  return false;
}

static bool fnNoPositionToRetracted() {
  if (LIFT_UP_sensor() && !LIFT_DOWN_sensor())
    return true;

  return false;
}

static bool fnNoPositionToHoming() {
  if (LIFT_UP_button())
    return true;

  return false;
}

/*******************************************************************
 * Controller Emergency stop
 *******************************************************************/
static void fnStateEmergencyStop() {
#ifdef DEBUG_CONTROLLER
  Serial.println("Enter state: EMERGENCY-STOP");
#endif
  DMC_disable();
  AZIMUTH_disable();
  AZIMUTH_analog_disable();

  LIFT_disable();
  LIFT_UP_off();
  LIFT_DOWN_off();

  // TODO: Set error emergency stop
}

static bool fnAnyToEmergencyStop() {
  return EMERGENCY_STOP_active();
}

static bool fnEmergencyStopToNoPosition() {
  if (!EMERGENCY_STOP_active()) {
    // TODO: Clear error not calibrated
    return true;
  }
  return false;
}

/*******************************************************************
 * Controller Emergency stop
 *******************************************************************/
static void fnStateMaintenace() {
#ifdef DEBUG_CONTROLLER
  Serial.println("Enter state: MAINTENANCE-MODE");
#endif
  DMC_disable();
  AZIMUTH_disable();
  AZIMUTH_analog_disable();

  LIFT_disable();
  LIFT_UP_off();
  LIFT_DOWN_off();

  MAINTENANCE_enable();  // Start maintenance mode
}

static bool fnMaintenanceToNoPosition() {
  if (!MAINTENANCE_enabled()) {
    return true;
  }
  return false;
}

static bool fnAnyToMantenance() {
  return CONTROLLER_maintenance_requested();
}

/*******************************************************************
 * Update steering
 *******************************************************************/
static void CONTROLLER_update_steering() {
  if (LIFT_DOWN_sensor()) {
    if (AZIMUTH_enabled() && AZIMUTH_analog_enabled()) {
      int value = STEERWHEEL_get_actual();
      AZIMUTH_set_steering(value);
    }
  }
}

/*******************************************************************
 * Controller main task
 *******************************************************************/
static void CONTROLLER_main_task(void *parameter) {
  (void)parameter;

  while (true) {
    stateMachine.Update();

    if (!MAINTENANCE_enabled()) {
      CONTROLLER_update_steering();
    }

    vTaskDelay(250 / portTICK_PERIOD_MS);
  }
}

/*******************************************************************
 * Setup Controller task(s)
 *******************************************************************/
static void CONTROLLER_setup_tasks() {
  xTaskCreate(CONTROLLER_main_task, "Controller debug task", 4096, NULL, 15, NULL);
}

/*******************************************************************
 * Setup Controller State Machine
 *******************************************************************/
static void CONTROLLER_setup_statemachine() {
  /* ---------------- */
  /* STATE(s) initial */
  /* ---------------- */
  stateMachine.SetOnEntering(CONTROLLER_init, fnStateInit);
  stateMachine.AddTransition(CONTROLLER_init, CONTROLLER_no_position, fnInitToNoPosition);

  /* -------------------- */
  /* STATE(s) No position */
  /* -------------------- */
  stateMachine.SetOnEntering(CONTROLLER_no_position, fnStateNoPosition);
  stateMachine.AddTransition(CONTROLLER_no_position, CONTROLLER_extended, fnNoPositionToExtended);
  stateMachine.AddTransition(CONTROLLER_no_position, CONTROLLER_extending, fnNoPositionToExtending);
  stateMachine.AddTransition(CONTROLLER_no_position, CONTROLLER_homing, fnNoPositionToHoming);
  stateMachine.AddTransition(CONTROLLER_no_position, CONTROLLER_retracted, fnNoPositionToRetracted);
  stateMachine.AddTransition(CONTROLLER_no_position, CONTROLLER_emergency_stop, fnAnyToEmergencyStop);
  stateMachine.AddTransition(CONTROLLER_no_position, CONTROLLER_maintenance, fnAnyToMantenance);

  /* ----------------------------------------- */
  /* STATE(s) Homing, retracting and retracted */
  /* ----------------------------------------- */
  stateMachine.SetOnEntering(CONTROLLER_homing, fnStateHoming);
  stateMachine.AddTransition(CONTROLLER_homing, CONTROLLER_retracting, fnHomingToRetracting);
  stateMachine.AddTransition(CONTROLLER_homing, CONTROLLER_no_position, fnHomingToNoPosition);
  stateMachine.AddTransition(CONTROLLER_homing, CONTROLLER_emergency_stop, fnAnyToEmergencyStop);

  stateMachine.SetOnEntering(CONTROLLER_retracting, fnStateRetracting);
  stateMachine.AddTransition(CONTROLLER_retracting, CONTROLLER_retracted, fnRetractingToRetracted);
  stateMachine.AddTransition(CONTROLLER_retracting, CONTROLLER_no_position, fnRetractingToNoPosition);
  stateMachine.AddTransition(CONTROLLER_retracting, CONTROLLER_emergency_stop, fnAnyToEmergencyStop);

  stateMachine.SetOnEntering(CONTROLLER_retracted, fnStateRetracted);
  stateMachine.AddTransition(CONTROLLER_retracted, CONTROLLER_extending, fnRetractedToExtending);
  stateMachine.AddTransition(CONTROLLER_retracted, CONTROLLER_no_position, fnRetractedToNoPosition);
  stateMachine.AddTransition(CONTROLLER_retracted, CONTROLLER_emergency_stop, fnAnyToEmergencyStop);
  stateMachine.AddTransition(CONTROLLER_retracted, CONTROLLER_maintenance, fnAnyToMantenance);

  /* ------------------------------- */
  /* STATE(s) Extending and extended */
  /* ------------------------------- */
  stateMachine.SetOnEntering(CONTROLLER_extending, fnStateExtending);
  stateMachine.AddTransition(CONTROLLER_extending, CONTROLLER_extended, fnExtendingToExtended);
  stateMachine.AddTransition(CONTROLLER_extending, CONTROLLER_no_position, fnExtendingToNoPosition);
  stateMachine.AddTransition(CONTROLLER_extending, CONTROLLER_emergency_stop, fnAnyToEmergencyStop);

  stateMachine.SetOnEntering(CONTROLLER_extended, fnStateExtended);
  stateMachine.AddTransition(CONTROLLER_extended, CONTROLLER_homing, fnExtendedToRetracting);
  stateMachine.AddTransition(CONTROLLER_extended, CONTROLLER_no_position, fnExtendedToNoPosition);
  stateMachine.AddTransition(CONTROLLER_extended, CONTROLLER_emergency_stop, fnAnyToEmergencyStop);
  stateMachine.AddTransition(CONTROLLER_extended, CONTROLLER_maintenance, fnAnyToMantenance);

  /* ----------------------- */
  /* STATE(s) Emergency stop */
  /* ----------------------- */
  stateMachine.SetOnEntering(CONTROLLER_emergency_stop, fnStateEmergencyStop);
  stateMachine.AddTransition(CONTROLLER_emergency_stop, CONTROLLER_no_position, fnEmergencyStopToNoPosition);

  /* ------------------------- */
  /* STATE(s) Maintenance mode */
  /* ------------------------- */
  stateMachine.SetOnEntering(CONTROLLER_maintenance, fnStateMaintenace);
  stateMachine.AddTransition(CONTROLLER_maintenance, CONTROLLER_no_position, fnMaintenanceToNoPosition);

  // Initial state
  stateMachine.SetState(CONTROLLER_init, false, true);
}

/*******************************************************************
 * Setup Controller
 *******************************************************************/
void CONTROLLER_setup() {
  CONTROLLER_setup_statemachine();

  Serial.println(F("Controller setup completed..."));
}

void CONTROLLER_start() {
  CONTROLLER_setup_tasks();

  Serial.println(F("Controller started..."));
}