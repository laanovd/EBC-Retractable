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
#include "DMC.h"
#include "Debug.h"
#include "GPIO.h"
#include "Lift.h"
#include "StateMachineLib.h"
#include "Storage.h"
#include "WebServer.h"

/*******************************************************************
 * Definitions
 *******************************************************************/
#define DEBUG

/*******************************************************************
 * RESTfull API keys
 *******************************************************************/
#define JSON_MAINTENANCE_ENABLE "maintenance_active"

/*******************************************************************
 * Local variables
 *******************************************************************/
static JsonDocument maintenance_data;

/********************************************************************
 * Internal JSON data
 *******************************************************************/
static void MAINTENANCE_json_init(void) {
  maintenance_data[JSON_MAINTENANCE_ENABLE] = false;

  maintenance_data[JSON_EMERGENCY_STOP] = true;

  maintenance_data[JSON_LIFT_ENABLED] = false;
  maintenance_data[JSON_LIFT_MOTOR_UP] = false;
  maintenance_data[JSON_LIFT_MOTOR_DOWN] = false;
  maintenance_data[JSON_LIFT_SENSOR_UP] = false;
  maintenance_data[JSON_LIFT_SENSOR_DOWN] = false;
  maintenance_data[JSON_LIFT_HOMING] = false;

  maintenance_data[JSON_DMC_ENABLE] = false;

  maintenance_data[JSON_AZIMUTH_ENABLE] = false;
  maintenance_data[JSON_AZIMUTH_HOME] = false;
  maintenance_data[JSON_AZIMUTH_LEFT_V] = 0.0;
  maintenance_data[JSON_AZIMUTH_RIGHT_V] = 0.0;
  maintenance_data[JSON_AZIMUTH_ACTUAL_V] = 0.0;
  maintenance_data[JSON_AZIMUTH_MANUAL] = false;
  maintenance_data[JSON_AZIMUTH_STEERING] = 0;
  maintenance_data[JSON_AZIMUTH_OUTPUT_ENABLE] = false;
  maintenance_data[JSON_DELAY_TO_MIDDLE] = 0;

  WEBSOCKET_send_doc(maintenance_data);
}

static JsonDocument MAINTENANCE_json(void) {
  maintenance_data[JSON_EMERGENCY_STOP] = EMERGENCY_STOP_active();

  // Lift
  maintenance_data[JSON_LIFT_ENABLED] = LIFT_enabled();
  maintenance_data[JSON_LIFT_SENSOR_UP] = LIFT_UP_sensor();
  maintenance_data[JSON_LIFT_SENSOR_DOWN] = LIFT_DOWN_sensor();

  // DMC
  maintenance_data[JSON_DMC_ENABLE] = DMC_enabled();

  // Steering
  maintenance_data[JSON_AZIMUTH_LEFT_V] = AZIMTUH_get_left();
  maintenance_data[JSON_AZIMUTH_RIGHT_V] = AZIMTUH_get_right();
  maintenance_data[JSON_AZIMUTH_ACTUAL_V] = AZIMTUH_get_actual();
  maintenance_data[JSON_AZIMUTH_MANUAL] = AZIMUTH_get_manual();
  maintenance_data[JSON_AZIMUTH_STEERING] = AZIMUTH_get_wheel();
  maintenance_data[JSON_AZIMUTH_OUTPUT_ENABLED] = AZIMUTH_output_enabled();

  return maintenance_data;
}

/********************************************************************
 * Create string
 *******************************************************************/
String MAINTENANCE_string(void) {
  JsonDocument doc = MAINTENANCE_json();

  String text = "--- MAINTENANCE ---";

  text.concat("\r\nMAINTENANCE...: ");
  text.concat(doc[JSON_MAINTENANCE_ENABLE].as<bool>() ? "ON" : "OFF");

  text.concat("\r\n");
  return text;
}

/********************************************************************
 * Getters and setters
 *******************************************************************/
bool MAINTENANCE_enabled(void) {
  return maintenance_data[JSON_MAINTENANCE_ENABLE].as<bool>();
}

static void MAINTENANCE_dmc_enable(void) {
  if (MAINTENANCE_enabled()) {
    DMC_enable();
    maintenance_data[JSON_DMC_ENABLE] = true;
  }
}

static void MAINTENANCE_azimuth_enable(void) {
  if (MAINTENANCE_enabled()) {
    AZIMUTH_enable();
    maintenance_data[JSON_AZIMUTH_ENABLE] = true;
  }
}

static void MAINTENANCE_lift_disable(void) {
  LIFT_disable();
}

static void MAINTENANCE_lift_enable(void) {
  if (MAINTENANCE_enabled()) {
    LIFT_enable();
  }
}

static void MAINTENANCE_lift_extend(void) {
  if (MAINTENANCE_enabled() && LIFT_enabled()) {
    LIFT_UP_off();
    vTaskDelay(500 / portTICK_PERIOD_MS);  // Wait 0.5s
    LIFT_DOWN_on();
  }
}

static void MAINTENANCE_lift_retract(void) {
  if (MAINTENANCE_enabled() && LIFT_enabled()) {
    LIFT_DOWN_off();
    vTaskDelay(500 / portTICK_PERIOD_MS);  // Wait 0.5s
    LIFT_UP_on();
  }
}

static void MAINTENANCE_lift_homing(void) {
  if (MAINTENANCE_enabled() && LIFT_enabled()) {
  }
}

static void MAINTENANCE_steering_disable(void) {
  AZIMUTH_disable();
}

static void MAINTENANCE_steering_enable(void) {
  if (MAINTENANCE_enabled()) {
    maintenance_data[JSON_AZIMUTH_ENABLE] = true;
  }
}

static bool MAINTENANCE_steering_enabled(void) {
  return maintenance_data[JSON_AZIMUTH_ENABLE].as<bool>();
}

void MAINTENANCE_disable(void) {
  maintenance_data[JSON_MAINTENANCE_ENABLE] = false;
  DMC_disable();
  LIFT_disable();
  AZIMUTH_disable();
}

void MAINTENANCE_enable(void) {
  if (!EMERGENCY_STOP_active()) {
    maintenance_data[JSON_MAINTENANCE_ENABLE] = true;
  }
}

/********************************************************************
 * JSON string command handler
 *********************************************************************/
DeserializationError MAINTENANCE_command_handler(char *data) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, data);

#ifdef DEBUG_API
  String str;
  Serial.print(F("MAINETNACE COMMAND HANDLER received: "));
  serializeJson(doc, str);
  Serial.println(str);
#endif

  /* Maintenance mode active */
  if (doc.containsKey(JSON_MAINTENANCE_ENABLE)) {
    if (doc[JSON_MAINTENANCE_ENABLE].as<bool>() == true)
      MAINTENANCE_enable();
    else
      MAINTENANCE_disable();
  }

  /* Lift enable */
  if (doc.containsKey(JSON_LIFT_ENABLED)) {
    if (doc[JSON_LIFT_ENABLED].as<bool>() == true)
      MAINTENANCE_lift_enable();
    else
      MAINTENANCE_lift_disable();
  }

  /* Lift RETRACT */
  if (doc.containsKey(JSON_LIFT_SENSOR_UP)) {
    if (doc[JSON_LIFT_SENSOR_UP].as<bool>() == true)
      MAINTENANCE_lift_retract();
  }

  /* Lift EXTEND */
  if (doc.containsKey(JSON_LIFT_SENSOR_DOWN)) {
    if (doc[JSON_LIFT_SENSOR_DOWN].as<bool>() == true)
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
      DMC_disable();
  }

  /* Steering enable */
  if (doc.containsKey(JSON_AZIMUTH_ENABLE)) {
    if (doc[JSON_AZIMUTH_ENABLE].as<bool>() == true)
      MAINTENANCE_steering_enable();
    else
      MAINTENANCE_steering_disable();
  }

  /* Steering analog output enable */
  if (doc.containsKey(JSON_AZIMUTH_OUTPUT_ENABLE)) {
    if (doc[JSON_AZIMUTH_OUTPUT_ENABLE].as<bool>() == true)
      MAINTENANCE_azimuth_enable();
    else
      AZIMUTH_disable();
  }

  /* Steering set LEFT voltage */
  if (doc.containsKey(JSON_AZIMUTH_LEFT_V)) {
    AZIMTUH_set_left(doc[JSON_AZIMUTH_LEFT_V].as<float>());
    // maintenance_data[JSON_AZIMUTH_LEFT_V] = doc[JSON_AZIMUTH_LEFT_V].as<float>();
  }

  /* Steering set RIGHT voltage */
  if (doc.containsKey(JSON_AZIMUTH_RIGHT_V)) {
    AZIMTUH_set_right(doc[JSON_AZIMUTH_RIGHT_V].as<float>());
    // maintenance_data[JSON_AZIMUTH_RIGHT_V] = doc[JSON_AZIMUTH_RIGHT_V].as<float>();
  }

  /* Steering manual control (%) */
  if (doc.containsKey(JSON_AZIMUTH_MANUAL)) {
    AZIMUTH_set_manual(doc[JSON_AZIMUTH_MANUAL].as<int>());
  }

  return error;
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

  DeserializationError error = MAINTENANCE_command_handler((char *)data);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    request->send(204, "text/plain", "204, No content");
    return;
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
static void MAINTENACE_websocket_task(void *parameter) {
  (void)parameter;

  while (true) {
    vTaskDelay(500 / portTICK_PERIOD_MS);

    WEBSOCKET_update_doc(MAINTENANCE_json());
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
  if (!LIFT_enabled()) {
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
  if (!AZIMUTH_enabled()) {
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
    if (MAINTENANCE_enabled()) {
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
  xTaskCreate(MAINTENACE_websocket_task, "WwebSocketServer task", 8192, NULL, 15, NULL);
}

/********************************************************************
 * Setup
 *******************************************************************/
void MAINTENANCE_setup(void) {
  MAINTENANCE_json_init();
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
