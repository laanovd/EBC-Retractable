/********************************************************************
 *    Maintenace.cpp
 *
 *    Retractable maintenance mode
 *
 ********************************************************************/
#include "Maintenance.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Endian.h>
#include <Math.h>

#include "Azimuth.h"
#include "CLI.h"
#include "Config.h"
#include "Debug.h"
#include "GPIO.h"
#include "MCPCom.h"
#include "StateMachineLib.h"
#include "Storage.h"
#include "WebServer.h"

#define DEBUG

/********************************************************************
 * Local definitions
 ********************************************************************/
#define JSON_MAINTENANCE_ACTIVE "active"
#define JSON_MAINTENANCE_STATUS "status"
#define JSON_MAINTENANCE_EMERGENCY_STOP "emergencys-top"
#define JSON_MAINTENANCE_LIFT_ENABLE "lift-enable"
#define JSON_MAINTENANCE_LIFT_HOMING "lift-homing"
#define JSON_MAINTENANCE_LIFT_RETRACT "lift-retract"
#define JSON_LIFT_RETRACTED "lift-retracted"
#define JSON_MAINTENANCE_LIFT_EXTEND "lift-extend"
#define JSON_LIFT_EXTENDED "lift-extended"

#define JSON_MAINTENANCE_DMC_ENABLE "dmc-enable"
#define JSON_DMC_ENABLED "dmc-enabled"

#define JSON_MAINTENANCE_STEERING_ENABLE "steering-enable"
#define JSON_STEERING_ENABLED "steering-enabled"
#define JSON_STEERING_LEFT_V "steering-left-volt"
#define JSON_STEERING_RIGHT_V "steering-right-volt"
#define JSON_STEERING_ACTUAL_V "steering-actual-volt"
#define JSON_STEERING_CONTROL_PERC "steering-control-percentage"
#define JSON_MAINTENANCE_STEERING_OUTPUT_ENABLE "steering-analog-out-enable"

/********************************************************************
 * Local variables
 ********************************************************************/
static JsonDocument maintenance_data;

/*********************************************************************
 * Create initial JSON data
 ********************************************************************/
static JsonDocument MAINTENANCE_json(void) {
  JsonDocument data;

  data[JSON_MAINTENANCE_ACTIVE] = false;

  // Emergency stop
  data[JSON_MAINTENANCE_EMERGENCY_STOP] = EMERGENCY_STOP_active();

  // Lift
  data[JSON_MAINTENANCE_LIFT_ENABLE] = false;
  data[JSON_MAINTENANCE_LIFT_RETRACT] = false;
  data[JSON_LIFT_RETRACTED] = RETRACTABLE_is_retracted();
  data[JSON_MAINTENANCE_LIFT_EXTEND] = false;
  data[JSON_LIFT_EXTENDED] = RETRACTABLE_is_extended();
  data[JSON_MAINTENANCE_LIFT_HOMING] = false;

  // DMC
  data[JSON_MAINTENANCE_DMC_ENABLE] = false;
  data[JSON_DMC_ENABLED] = DMC_enabled();

  // Steering
  data[JSON_MAINTENANCE_STEERING_ENABLE] = false;
  data[JSON_STEERING_ENABLED] = ANALOG_OUT_enabled();
  data[JSON_STEERING_LEFT_V] = AZIMUH_get_left();
  data[JSON_STEERING_RIGHT_V] = AZIMUH_get_right();
  data[JSON_STEERING_ACTUAL_V] = AZIMUH_get_actual();
  data[JSON_STEERING_CONTROL_PERC] = STEERING_WHEEL_get_position();
  data[JSON_MAINTENANCE_STEERING_OUTPUT_ENABLE] = false;

  return data;
}

/*********************************************************************
 * Create string
 ********************************************************************/
String MAINTENANCE_string(void) {
  JsonDocument doc = MAINTENANCE_json();

  String text = "--- MAINTENANCE ---";

  text.concat("\r\nMAINTENANCE...: ");
  text.concat(doc[JSON_MAINTENANCE_ACTIVE].as<bool>() ? "ON" : "OFF");

  text.concat("\r\n");
  return text;
}

/*********************************************************************
 * Getters and setters
 ********************************************************************/
bool MAINTENANCE_active(void) {
  return maintenance_data[JSON_MAINTENANCE_ACTIVE].as<bool>();
}

static bool MAINTENANCE_dmc_active(void) {
  return maintenance_data[JSON_MAINTENANCE_DMC_ENABLE].as<bool>();
}

static void MAINTENANCE_dmc_disable(void) {
  maintenance_data[JSON_MAINTENANCE_DMC_ENABLE] = false;
}

static void MAINTENANCE_dmc_enable(void) {
  if (MAINTENANCE_active()) {
    maintenance_data[JSON_MAINTENANCE_DMC_ENABLE] = true;
  } else
    MAINTENANCE_dmc_disable();
}

static void MAINTENANCE_lift_disable(void) {
  maintenance_data[JSON_MAINTENANCE_LIFT_ENABLE] = false;
  maintenance_data[JSON_MAINTENANCE_LIFT_EXTEND] = false;
  maintenance_data[JSON_MAINTENANCE_LIFT_RETRACT] = false;
  maintenance_data[JSON_MAINTENANCE_LIFT_HOMING] = false;
}

static void MAINTENANCE_lift_enable(void) {
  if (MAINTENANCE_active()) {
    maintenance_data[JSON_MAINTENANCE_LIFT_ENABLE] = true;
  } else
    MAINTENANCE_lift_disable();
}

static bool MAINTENANCE_lift_enabled(void) {
  return maintenance_data[JSON_MAINTENANCE_LIFT_ENABLE].as<bool>();
}

static void MAINTENANCE_lift_extend(void) {
  if (MAINTENANCE_active()) {
    maintenance_data[JSON_MAINTENANCE_LIFT_EXTEND] = true;
    maintenance_data[JSON_MAINTENANCE_LIFT_RETRACT] = false;
    maintenance_data[JSON_MAINTENANCE_LIFT_HOMING] = false;
  }
}

static void MAINTENANCE_lift_retract(void) {
  if (MAINTENANCE_active()) {
    maintenance_data[JSON_MAINTENANCE_LIFT_RETRACT] = true;
    maintenance_data[JSON_MAINTENANCE_LIFT_EXTEND] = false;
    maintenance_data[JSON_MAINTENANCE_LIFT_HOMING] = false;
  }
}

static void MAINTENANCE_lift_homing(void) {
  if (MAINTENANCE_active()) {
    maintenance_data[JSON_MAINTENANCE_LIFT_RETRACT] = false;
    maintenance_data[JSON_MAINTENANCE_LIFT_EXTEND] = false;
    maintenance_data[JSON_MAINTENANCE_LIFT_HOMING] = true;
  }
}

static bool MAINTENANCE_steering_output_enabled(void) {
  return maintenance_data[JSON_MAINTENANCE_STEERING_OUTPUT_ENABLE].as<bool>();
}

static void MAINTENANCE_steering_output_disable(void) {
  maintenance_data[JSON_MAINTENANCE_STEERING_OUTPUT_ENABLE] = false;
}

static void MAINTENANCE_steering_output_enable(void) {
  if (MAINTENANCE_active()) {
    maintenance_data[JSON_MAINTENANCE_STEERING_OUTPUT_ENABLE] = true;
  } else
    MAINTENANCE_steering_output_disable();
}

static void MAINTENANCE_steering_disable(void) {
  maintenance_data[JSON_MAINTENANCE_STEERING_ENABLE] = false;
  MAINTENANCE_steering_output_disable();
}

static void MAINTENANCE_steering_enable(void) {
  if (MAINTENANCE_active()) {
    maintenance_data[JSON_MAINTENANCE_STEERING_ENABLE] = true;
  } else
    MAINTENANCE_steering_disable();
}

static bool MAINTENANCE_steering_enabled(void) {
  return maintenance_data[JSON_MAINTENANCE_STEERING_ENABLE].as<bool>();
}

static void MAINTENANCE_deactivate(void) {
  maintenance_data[JSON_MAINTENANCE_ACTIVE] = false;
  MAINTENANCE_deactivate();
  MAINTENANCE_lift_disable();
  MAINTENANCE_steering_disable();
}

static void MAINTENANCE_activate(void) {
  if (!EMERGENCY_STOP_active()) {
    maintenance_data[JSON_MAINTENANCE_ACTIVE] = true;
  } else
    MAINTENANCE_deactivate();
}

/*********************************************************************
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

  // Request activation maintenance mode
  if (doc.containsKey(JSON_MAINTENANCE_ACTIVE)) {
    if (doc[JSON_MAINTENANCE_ACTIVE].as<bool>() == true)
      MAINTENANCE_activate();
    else
      MAINTENANCE_deactivate();
  }

  /* Set steering minimum voltage */
  if (doc.containsKey(JSON_STEERING_LEFT_V)) {
    AZIMUTH_set_left(doc[JSON_STEERING_LEFT_V].as<float>());
  }

  /* Set steering maximum voltage */
  if (doc.containsKey(JSON_STEERING_RIGHT_V)) {
    AZIMUTH_set_right(doc[JSON_STEERING_RIGHT_V].as<float>());
  }

  /* DMC enable */
  if (doc.containsKey(JSON_MAINTENANCE_DMC_ENABLE)) {
    if (doc[JSON_MAINTENANCE_DMC_ENABLE].as<bool>() == true)
      MAINTENANCE_dmc_enable();
    else
      MAINTENANCE_dmc_disable();
  }

  /* Lift enable */
  if (doc.containsKey(JSON_MAINTENANCE_LIFT_ENABLE)) {
    if (doc[JSON_MAINTENANCE_LIFT_ENABLE].as<bool>() == true)
      MAINTENANCE_lift_enable();
    else
      MAINTENANCE_lift_disable();
  }

  /* Lift retract */
  if (doc.containsKey(JSON_MAINTENANCE_LIFT_RETRACT)) {
    if (doc[JSON_MAINTENANCE_LIFT_RETRACT].as<bool>() == true)
      MAINTENANCE_lift_retract();
  }

  /* Lift extend */
  if (doc.containsKey(JSON_MAINTENANCE_LIFT_EXTEND)) {
    if (doc[JSON_MAINTENANCE_LIFT_EXTEND].as<bool>() == true)
      MAINTENANCE_lift_extend();
  }

  /* Lift extend */
  if (doc.containsKey(JSON_MAINTENANCE_LIFT_HOMING)) {
    if (doc[JSON_MAINTENANCE_LIFT_HOMING].as<bool>() == true)
      MAINTENANCE_lift_homing();
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

/*********************************************************************
 * Maintenance parts
 ********************************************************************/
void MAINTENANCE_dmc_control(void) {
  if (!MAINTENANCE_dmc_active()) {
    DMC_set_low();
    return;
  }

  DMC_set_high();
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

/*********************************************************************
 * Main task
 ********************************************************************/
void MAINTENANCE_main(void *parameter) {
  (void)parameter;

  while (1) {
    // Start maintenace mode
    if (MAINTENANCE_active()) {
      // TODO: Conditions for actvating maintenance mode
      if (EMERGENCY_STOP_active()) {
        MAINTENANCE_deactivate();
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

/*********************************************************************
 * Setup
 ********************************************************************/
void MAINTENANCE_setup(void) {
  setup_uri(&MAINTENANCE_api_handlers);
}

/*********************************************************************
 * Start
 ********************************************************************/
void MAINTENANCE_start(void) {
  MAINTENANCE_deactivate();

  Serial.println("MAINTENANCE setup completed...");
}
