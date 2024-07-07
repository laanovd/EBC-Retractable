/*******************************************************************
 *    Maintenace.cpp
 *
 *    Retractable maintenance mode
 *
 *******************************************************************/
#include "Maintenance.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Endian.h>
#include <Math.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

#include "Azimuth.h"
#include "CLI.h"
#include "Config.h"
#include "Debug.h"
#include "GPIO.h"
#include "StateMachineLib.h"
#include "Storage.h"
#include "WebServer.h"
#include "Lift.h"
#include "DMC.h"

/*******************************************************************
 * Definitions
 *******************************************************************/
#define DEBUG
#define DEBUG_API

/*******************************************************************
 * Local definitions
 *******************************************************************/
#define JSON_MAINTENANCE_ACTIVATE "maintenance_activate"
#define JSON_MAINTENANCE_ACTIVE "maintenance_active"
#define JSON_EMERGENCY_STOP "emergency_stop"
#define JSON_LIFT_ENABLE "lift_enable"
#define JSON_LIFT_ENABLED "lift_enabled"
#define JSON_LIFT_HOMING "lift_homing"
#define JSON_LIFT_RETRACT "lift_retract"
#define JSON_LIFT_RETRACTED "lift_retracted"
#define JSON_LIFT_EXTEND "lift_extend"
#define JSON_LIFT_EXTENDED "lift_extended"

#define JSON_DMC_ENABLE "dmc_enable"
#define JSON_DMC_ENABLED "dmc_enabled"

#define JSON_STEERING_ENABLE "steering_enable"
#define JSON_STEERING_ENABLED "steering_enabled"
#define JSON_STEERING_LEFT_V "steering_left_volt"
#define JSON_STEERING_RIGHT_V "steering_right_volt"
#define JSON_STEERING_ACTUAL_V "steering_actual_volt"
#define JSON_STEERING_MANUAL "steering_manual"
#define JSON_STEERING_CONTROL_PERC "steering_control_percentage"
#define JSON_STEERING_OUTPUT_ENABLE "steering_analog_out_enable"
#define JSON_STEERING_OUTPUT_ENABLED "steering_analog_out_enabled"

/*******************************************************************
 * Local variables
 *******************************************************************/
static JsonDocument maintenance_data;

/********************************************************************
 * Internal JSON data
 *******************************************************************/
static JsonDocument MAINTENANCE_defaults(void) {
  JsonDocument data;

  data[JSON_MAINTENANCE_ACTIVE] = false;

  // Emergency stop
  data[JSON_EMERGENCY_STOP] = EMERGENCY_STOP_active();

  // Lift
  data[JSON_LIFT_ENABLED] = LIFT_enabled();
  data[JSON_LIFT_RETRACTED] = LIFT_UP_sensor();
  data[JSON_LIFT_EXTENDED] = LIFT_DOWN_sensor();
  data[JSON_LIFT_HOMING] = false;

  // DMC
  data[JSON_DMC_ENABLED] = DMC_enabled();

  // Steering
  data[JSON_STEERING_ENABLED] = AZIMUTH_enabled();
  data[JSON_STEERING_LEFT_V] = AZIMUH_get_left();
  data[JSON_STEERING_RIGHT_V] = AZIMUH_get_right();
  data[JSON_STEERING_ACTUAL_V] = AZIMUH_get_actual();
  data[JSON_STEERING_MANUAL] = AZIMUTH_get_steering();
  data[JSON_STEERING_CONTROL_PERC] = STEERING_WHEEL_get_position();

  return data;
}

static void MAINTENANCE_json_update(void) {
  maintenance_data[JSON_EMERGENCY_STOP] = EMERGENCY_STOP_active();

  // Lift
  maintenance_data[JSON_LIFT_ENABLED] = LIFT_enabled();
  maintenance_data[JSON_LIFT_RETRACTED] = LIFT_UP_sensor();
  maintenance_data[JSON_LIFT_EXTENDED] = LIFT_DOWN_sensor();

  // DMC
  maintenance_data[JSON_DMC_ENABLED] = DMC_enabled();

  // Steering
  maintenance_data[JSON_STEERING_LEFT_V] = AZIMUH_get_left();
  maintenance_data[JSON_STEERING_RIGHT_V] = AZIMUH_get_right();
  maintenance_data[JSON_STEERING_ACTUAL_V] = AZIMUH_get_actual();
  maintenance_data[JSON_STEERING_MANUAL] = AZIMUTH_get_steering();
  maintenance_data[JSON_STEERING_CONTROL_PERC] = STEERING_WHEEL_get_position();
  maintenance_data[JSON_STEERING_OUTPUT_ENABLED] = AZIMUTH_enabled();
}

static JsonDocument MAINTENANCE_json(void) {
  MAINTENANCE_json_update();
  return maintenance_data;
}

/********************************************************************
 * Create string
 *******************************************************************/
String MAINTENANCE_string(void) {
  JsonDocument doc = MAINTENANCE_json();

  String text = "--- MAINTENANCE ---";

  text.concat("\r\nMAINTENANCE...: ");
  text.concat(doc[JSON_MAINTENANCE_ACTIVE].as<bool>() ? "ON" : "OFF");

  text.concat("\r\n");
  return text;
}

/********************************************************************
 * Getters and setters
 *******************************************************************/
bool MAINTENANCE_enabled(void) {
  return maintenance_data[JSON_MAINTENANCE_ACTIVE].as<bool>();
}

bool MAINTENANCE_activate(void) {
  return maintenance_data[JSON_MAINTENANCE_ACTIVATE].as<bool>();
}

static bool MAINTENANCE_dmc_active(void) {
  return DMC_enabled();
}

static void MAINTENANCE_dmc_enable(void) {
  if (MAINTENANCE_enabled()) {
    DMC_enable();

#ifdef DEBUG_API
    Serial.println("MAINTENANCE DMC enable...");
#endif
  }
}

static void MAINTENANCE_dmc_disable(void) {
  DMC_disable();

#ifdef DEBUG_API
  Serial.println("MAINTENANCE DMC disable...");
#endif
}

static bool MAINTENANCE_azimuth_active(void) {
  return AZIMUTH_enabled();
}

static void MAINTENANCE_azimuth_disable(void) {
  AZIMUTH_disable();

#ifdef DEBUG_API
  Serial.println("MAINTENANCE AZIMUTH disable...");
#endif
}

static void MAINTENANCE_azimuth_enable(void) {
  if (MAINTENANCE_enabled()) {
    AZIMUTH_enable();

#ifdef DEBUG_API
    Serial.println("MAINTENANCE AZIMUTH enable...");
#endif
  }
}

static bool MAINTENANCE_steering_output_enabled(void) {
  return AZIMUTH_enabled();

#ifdef DEBUG_API
  Serial.println("MAINTENANCE AZIMUTH output disable...");
#endif
}

static void MAINTENANCE_steering_output_disable(void) {
  MAINTENANCE_azimuth_disable();

#ifdef DEBUG_API
  Serial.println("MAINTENANCE AZIMUTH output disable...");
#endif
}

static void MAINTENANCE_steering_output_enable(void) {
  MAINTENANCE_azimuth_enable();

#ifdef DEBUG_API
  Serial.println("MAINTENANCE AZIMUTH enable...");
#endif
}

static void MAINTENANCE_lift_disable(void) {
  LIFT_disable();
  maintenance_data[JSON_LIFT_EXTEND] = false;
  maintenance_data[JSON_LIFT_RETRACT] = false;
  maintenance_data[JSON_LIFT_HOMING] = false;

#ifdef DEBUG_API
  Serial.println("MAINTENANCE LIFT disable...");
#endif
}

static void MAINTENANCE_lift_enable(void) {
  if (MAINTENANCE_enabled()) {
    LIFT_enable();

#ifdef DEBUG_API
    Serial.println("MAINTENANCE LIFT enable...");
#endif
  }
}

static bool MAINTENANCE_lift_enabled(void) {
  return LIFT_enabled();
}

static void MAINTENANCE_lift_extend(void) {
  if (MAINTENANCE_enabled() && MAINTENANCE_lift_enabled()) {
    LIFT_UP_off();
    vTaskDelay(500 / portTICK_PERIOD_MS); // Wait 0.5s
    LIFT_DOWN_on();
    maintenance_data[JSON_LIFT_EXTEND] = true;
    maintenance_data[JSON_LIFT_RETRACT] = false;
    maintenance_data[JSON_LIFT_HOMING] = false;

#ifdef DEBUG_API
    Serial.println("MAINTENANCE LIFT extend...");
#endif
  }
}

static void MAINTENANCE_lift_retract(void) {
  if (MAINTENANCE_enabled() && MAINTENANCE_lift_enabled()) {
    LIFT_DOWN_off();
    vTaskDelay(500 / portTICK_PERIOD_MS); // Wait 0.5s
    LIFT_UP_on();
    maintenance_data[JSON_LIFT_RETRACT] = true;
    maintenance_data[JSON_LIFT_EXTEND] = false;
    maintenance_data[JSON_LIFT_HOMING] = false;

#ifdef DEBUG_API
    Serial.println("MAINTENANCE LIFT retract...");
#endif
  }
}

static void MAINTENANCE_lift_homing(void) {
  if (MAINTENANCE_enabled() && MAINTENANCE_lift_enabled()) {
    maintenance_data[JSON_LIFT_RETRACT] = false;
    maintenance_data[JSON_LIFT_EXTEND] = false;
    maintenance_data[JSON_LIFT_HOMING] = true;

#ifdef DEBUG_API
    Serial.println("MAINTENANCE LIFT start homing...");
#endif
  }
}

static void MAINTENANCE_steering_disable(void) {
  maintenance_data[JSON_STEERING_ENABLED] = false;
  MAINTENANCE_steering_output_disable();

#ifdef DEBUG_API
  Serial.println("MAINTENANCE STEERING disable...");
#endif
}

static void MAINTENANCE_steering_enable(void) {
  if (MAINTENANCE_enabled()) {
    maintenance_data[JSON_STEERING_ENABLED] = true;
#ifdef DEBUG_API
    Serial.println("MAINTENANCE STEERING enable...");
#endif
  }
}

static bool MAINTENANCE_steering_enabled(void) {
  return maintenance_data[JSON_STEERING_ENABLED].as<bool>();
}

void MAINTENANCE_disable(void) {
  maintenance_data[JSON_MAINTENANCE_ACTIVE] = false;
  MAINTENANCE_dmc_disable();
  MAINTENANCE_lift_disable();
  MAINTENANCE_steering_disable();

#ifdef DEBUG_API
  Serial.println("MAINTENANCE deactive...");
#endif
}

void MAINTENANCE_enable(void) {
  if (!EMERGENCY_STOP_active()) {
    maintenance_data[JSON_MAINTENANCE_ACTIVE] = true;
#ifdef DEBUG_API
    Serial.println("MAINTENANCE active...");
#endif
  }
}

/********************************************************************
 * REST API: read handler
 *********************************************************************/
void MAINTENANCE_rest_read(AsyncWebServerRequest *request) {
  String str;
  serializeJson(MAINTENANCE_json(), str);
  request->send(200, "application/json", str.c_str());
}

void MAINTENANCE_rest_update(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  (void)len;
  (void)index;
  (void)total;

  JsonDocument doc;

  DeserializationError error = deserializeJson(doc, (char *)data);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    request->send(204, "text/plain", "204, No content");
    return;
  }

#ifdef DEBUG_API
  String str;
  Serial.print(F("REST API received: "));
  serializeJson(doc, str);
  Serial.println(str);
#endif

  /* Maintenance mode activate */
  if (doc.containsKey(JSON_MAINTENANCE_ACTIVATE)) {
    if (doc[JSON_MAINTENANCE_ACTIVATE].as<bool>() == true)
      MAINTENANCE_enable();
    else
      MAINTENANCE_disable();
  }

  /* Lift enable */
  if (doc.containsKey(JSON_LIFT_ENABLE)) {
    if (doc[JSON_LIFT_ENABLE].as<bool>() == true)
      MAINTENANCE_lift_enable();
    else
      MAINTENANCE_lift_disable();
  }

  /* Lift RETRACT */
  if (doc.containsKey(JSON_LIFT_RETRACT)) {
    if (doc[JSON_LIFT_RETRACT].as<bool>() == true)
      MAINTENANCE_lift_retract();
  }

  /* Lift EXTEND */
  if (doc.containsKey(JSON_LIFT_EXTEND)) {
    if (doc[JSON_LIFT_EXTEND].as<bool>() == true)
      MAINTENANCE_lift_extend();
  }

  /* Lift homing */
  if (doc.containsKey(JSON_LIFT_HOMING)) {
    if (doc[JSON_LIFT_HOMING].as<bool>() == true)
      MAINTENANCE_lift_homing();
  }

  /* DMC enable */
  if (doc.containsKey(JSON_DMC_ENABLE)) {
    if (doc[JSON_DMC_ENABLE].as<bool>() == true)
      MAINTENANCE_dmc_enable();
    else
      MAINTENANCE_dmc_disable();
  }

  /* Steering enable */
  if (doc.containsKey(JSON_STEERING_ENABLE)) {
    if (doc[JSON_STEERING_ENABLE].as<bool>() == true)
      MAINTENANCE_steering_enable();
    else
      MAINTENANCE_steering_disable();
  }

  /* Steering analog output enable */
  if (doc.containsKey(JSON_STEERING_OUTPUT_ENABLE)) {
    if (doc[JSON_STEERING_OUTPUT_ENABLE].as<bool>() == true)
      MAINTENANCE_azimuth_enable();
    else
      MAINTENANCE_azimuth_disable();
  }

  /* Steering set LEFT voltage */
  if (doc.containsKey(JSON_STEERING_LEFT_V)) {
    AZIMUH_set_left(doc[JSON_STEERING_LEFT_V].as<float>());
    maintenance_data[JSON_STEERING_LEFT_V] = doc[JSON_STEERING_LEFT_V].as<float>();

// #ifdef DEBUG_API
//     Serial.printf("MAINTENANCE set left voltage to %2.1fV.\n\r", AZIMUH_get_left());
// #endif
  }

  /* Steering set RIGHT voltage */
  if (doc.containsKey(JSON_STEERING_RIGHT_V)) {
    AZIMUH_set_right(doc[JSON_STEERING_RIGHT_V].as<float>());
    maintenance_data[JSON_STEERING_RIGHT_V] = doc[JSON_STEERING_RIGHT_V].as<float>();

// #ifdef DEBUG_API
//     Serial.printf("MAINTENANCE set right voltage to %2.1fV.\n\r", AZIMUH_get_right());
// #endif
  }

  /* Steering manual control (%) */
  if (doc.containsKey(JSON_STEERING_MANUAL)) {
    AZIMUTH_set_steering(doc[JSON_STEERING_MANUAL].as<int>());

// #ifdef DEBUG_API
//     Serial.printf("MAINTENANCE set manual steering to %d%%.\n\r", AZIMUTH_get_steering());
// #endif
  }

  request->send(200, "text/plain", "200, OK");
}

static rest_api_t MAINTENANCE_api_handlers = {
    /* uri */ "/api/v1/maintenance",
    /* comment */ "MAINETNACE module",
    /* instances */ 1,
    /* fn_create */ nullptr,
    /* fn_read */ MAINTENANCE_rest_read,
    /* fn_update */ MAINTENANCE_rest_update,
    /* fn_delete */ nullptr,
};

/********************************************************************
 * WebSocketsServer task
 *********************************************************************/
static void MAINETNACE_websocket_task(void *parameter) {
  (void)parameter;
  JsonDocument doc;
  String str;

  while (true) {
    vTaskDelay(500 / portTICK_PERIOD_MS);

    MAINTENANCE_json_update();

    WEBSocket_set(maintenance_data);
  }
}

/********************************************************************
 * Maintenance parts
 *******************************************************************/
void MAINTENANCE_dmc_control(void) {
  if (MAINTENANCE_enabled()) {
    DMC_enable();
  } else {
    DMC_disable();
  }
}

void MAINTENANCE_lift_control(void) {
  if (!MAINTENANCE_lift_enabled()) {
    // TODO: LIFT control
    return;
  }

  // TODO: LIFT control
}

void MAINTENANCE_steering_control(void) {
  if (!MAINTENANCE_steering_enabled()) {
    // TODO: STEERING control
    return;
  }

  // TODO: STEERING control
}

void MAINTENANCE_steering_output_control(void) {
  if (!MAINTENANCE_steering_output_enabled()) {
    // TODO: STEERING output control
    return;
  }

  // TODO: STEERING output control
}

/********************************************************************
 * Main task
 *******************************************************************/
void MAINTENANCE_main(void *parameter) {
  (void)parameter;

  while (1) {
    if (MAINTENANCE_activate()) {
      MAINTENANCE_enable();
    } else {
      MAINTENANCE_disable();
    }

    // Start maintenace mode
    if (MAINTENANCE_enabled()) {
      // TODO: Conditions for actvating maintenance mode
      if (EMERGENCY_STOP_active()) {
        MAINTENANCE_disable();
        continue;
      }

      MAINTENANCE_dmc_control();
      MAINTENANCE_lift_control();
      MAINTENANCE_steering_control();
      MAINTENANCE_steering_output_control();

      vTaskDelay(100 / portTICK_PERIOD_MS);
      continue;
    }

    /* Maintenance mode not active */
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

/********************************************************************
 *  Initialize the debug tasks
 *
 *********************************************************************/
static void MAINTENANCE_setup_tasks(void) {
  xTaskCreate(MAINETNACE_websocket_task, "WwebSocketServer task", 8192, NULL, 15, NULL);
}

/********************************************************************
 * Setup
 *******************************************************************/
void MAINTENANCE_setup(void) {
  maintenance_data = MAINTENANCE_defaults();
  setup_uri(&MAINTENANCE_api_handlers);

  Serial.println("MAINTENANCE setup completed...");
}

/********************************************************************
 * Start
 *******************************************************************/
void MAINTENANCE_start(void) {
  MAINTENANCE_disable();
  MAINTENANCE_setup_tasks();

  Serial.println("MAINTENANCE started...");
}
