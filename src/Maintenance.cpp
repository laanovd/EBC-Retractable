/********************************************************************
 *    Maintenace.cpp
 *
 *    Retractable maintenance mode
 *
 ********************************************************************/
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Endian.h>
#include <Math.h>
#include "WebServer.h"

#include "CLI.h"
#include "Config.h"
#include "Debug.h"
#include "MCPCom.h"
#include "StateMachineLib.h"
#include "Storage.h"

#include "Maintenance.h"

#define DEBUG

/********************************************************************
 * Local variables
 ********************************************************************/
#define JSON_MAINTENANCE_MODE "maintenance-mode"
#define JSON_MAINTENANCE_ACTIVE "active"
#define JSON_MAINTENANCE_STATUS "status"
#define JSON_MAINTENANCE_EMERGENCY_STOP "emergencys-top"
#define JSON_MAINTENANCE_LIFT_ENABLE "lift-enable"
#define JSON_MAINTENANCE_LIFT_HOMING "lift-homing"
#define JSON_MAINTENANCE_LIFT_RETRACT "lift-retract"
#define JSON_MAINTENANCE_LIFT_RETRACTED "lift-retracted"
#define JSON_MAINTENANCE_LIFT_EXTEND "lift-extend"
#define JSON_MAINTENANCE_LIFT_EXTENDED "lift-extended"

#define JSON_MAINTENANCE_DMC_ENABLE "dmc-enable"
#define JSON_MAINTENANCE_DMC_ENABLED "dmc-enabled"

#define JSON_MAINTENANCE_STEERING_ENABLE "steering-enable"
#define JSON_MAINTENANCE_STEERING_ENABLED "steering-enabled"
#define JSON_MAINTENANCE_STEERING_MIN_V "steering-min-volt"
#define JSON_MAINTENANCE_STEERING_MAX_V "steering-max-volt"
#define JSON_MAINTENANCE_STEERING_ACTUAL_V "steering-actual-volt"
#define JSON_MAINTENANCE_STEERING_CONTROL_PERC "steering-control-percentage"
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

  data[JSON_MAINTENANCE_MODE] = false;
  data[JSON_MAINTENANCE_ACTIVE] = false;

  // Emergency stop
  data[JSON_MAINTENANCE_EMERGENCY_STOP] = "inactive";

  // Lift
  data[JSON_MAINTENANCE_LIFT_ENABLE] = false;
  data[JSON_MAINTENANCE_LIFT_RETRACT] = false;
  data[JSON_MAINTENANCE_LIFT_RETRACTED] = false;
  data[JSON_MAINTENANCE_LIFT_EXTEND] = false;
  data[JSON_MAINTENANCE_LIFT_EXTENDED] = false;
  data[JSON_MAINTENANCE_LIFT_HOMING] = false;

  // DMC
  data[JSON_MAINTENANCE_DMC_ENABLE] = false;
  data[JSON_MAINTENANCE_DMC_ENABLED] = false;

  // Steering
  data[JSON_MAINTENANCE_STEERING_ENABLE] = false;
  data[JSON_MAINTENANCE_STEERING_ENABLED] = false;
  data[JSON_MAINTENANCE_STEERING_MIN_V] = 0.0;
  data[JSON_MAINTENANCE_STEERING_MAX_V] = 5.0;
  data[JSON_MAINTENANCE_STEERING_ACTUAL_V] = 0.0;
  data[JSON_MAINTENANCE_STEERING_CONTROL_PERC] = 0;
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
  text.concat(doc[JSON_MAINTENANCE_MODE].as<bool>() ? "ON": "OFF");

  text.concat("\r\n");
  return text;
}

/*********************************************************************
 * REST API: read handler
 *********************************************************************/
void MAINTENANCE_rest_read(AsyncWebServerRequest *request) {
  String str;
  serializeJson(MAINTENANCE_json(), str);
  request->send(200, "application/json", str.c_str());
}

static rest_api_t MAINTENANCE_api_handlers = {
    /* uri */ "/api/v1/bms",
    /* comment */ "BMS module",
    /* instances */ 1,
    /* fn_create */ nullptr,
    /* fn_read */ MAINTENANCE_rest_read,
    /* fn_update */ nullptr,
    /* fn_delete */ nullptr,
};

/*********************************************************************
 * Setup MAINTENANCE
 ********************************************************************/
void MAINTENANCE_setup(void) {
  // MAINTENANCE_setup_variables();
  setup_uri(&MAINTENANCE_api_handlers);
}

/*********************************************************************
 * Start MAINTENANCE
 ********************************************************************/
void MAINTENANCE_start(void) {
  if (active) {
    Serial.println("MAINTENANCE mode active.");

  } 

  Serial.println("MAINTENANCE setup completed...");
}

