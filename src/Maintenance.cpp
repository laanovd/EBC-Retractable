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
#include "Controller.h"
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
 * JSON and Websocket keys
 *******************************************************************/
#define JSON_MAINTENANCE_ENABLED "maintenance_enabled"

/*******************************************************************
 * Local variables
 *******************************************************************/
static JsonDocument maintenance_data;
static int azimuth_homing_timer = 0;
static int lift_homing_timer = 0;

/********************************************************************
 * Internal JSON data
 *******************************************************************/
static void MAINTENANCE_json_init(void) {
  maintenance_data[JSON_MAINTENANCE_ENABLED] = false;

  maintenance_data[JSON_EMERGENCY_STOP] = true;

  maintenance_data[JSON_LIFT_ENABLED] = false;
  maintenance_data[JSON_LIFT_MOTOR_UP] = false;
  maintenance_data[JSON_LIFT_MOTOR_DOWN] = false;
  maintenance_data[JSON_LIFT_SENSOR_UP] = false;
  maintenance_data[JSON_LIFT_SENSOR_DOWN] = false;
  maintenance_data[JSON_LIFT_HOMING] = false;

  maintenance_data[JSON_DMC_ENABLED] = false;

  maintenance_data[JSON_AZIMUTH_ENABLED] = false;
  maintenance_data[JSON_AZIMUTH_HOME] = false;
  maintenance_data[JSON_AZIMUTH_LEFT_V] = 0.0;
  maintenance_data[JSON_AZIMUTH_RIGHT_V] = 0.0;
  maintenance_data[JSON_AZIMUTH_ACTUAL_V] = 0.0;
  maintenance_data[JSON_AZIMUTH_MANUAL] = 0;
  maintenance_data[JSON_AZIMUTH_STEERING] = 0;
  maintenance_data[JSON_AZIMUTH_OUTPUT_ENABLED] = false;
  maintenance_data[JSON_DELAY_TO_MIDDLE] = 0;
  maintenance_data[JSON_AZIMUTH_HOME] = false;
  maintenance_data[JSON_AZIMUTH_HOMING] = false;

  WEBSOCKET_send_doc(maintenance_data);
}

static JsonDocument MAINTENANCE_json(void) {
  maintenance_data[JSON_EMERGENCY_STOP] = EMERGENCY_STOP_active();

  // Lift
  maintenance_data[JSON_LIFT_ENABLED] = LIFT_enabled();
  maintenance_data[JSON_LIFT_MOTOR_UP] = LIFT_UP_moving();
  maintenance_data[JSON_LIFT_SENSOR_UP] = LIFT_UP_sensor();
  maintenance_data[JSON_LIFT_MOTOR_DOWN] = LIFT_DOWN_moving();
  maintenance_data[JSON_LIFT_SENSOR_DOWN] = LIFT_DOWN_sensor();

  // DMC
  maintenance_data[JSON_DMC_ENABLED] = DMC_enabled();

  // Steering
  maintenance_data[JSON_AZIMUTH_ENABLED] = AZIMUTH_enabled();
  maintenance_data[JSON_AZIMUTH_OUTPUT_ENABLED] = AZIMUTH_analog_enabled();
  maintenance_data[JSON_AZIMUTH_LEFT_V] = AZIMUTH_get_left();
  maintenance_data[JSON_AZIMUTH_RIGHT_V] = AZIMUTH_get_right();
  maintenance_data[JSON_AZIMUTH_ACTUAL_V] = AZIMUTH_get_actual();
  maintenance_data[JSON_AZIMUTH_MANUAL] = AZIMUTH_get_manual();
  maintenance_data[JSON_AZIMUTH_STEERING] = AZIMUTH_get_steerwheel();
  maintenance_data[JSON_AZIMUTH_HOME] = AZIMUTH_home();

  return maintenance_data;
}

/********************************************************************
 * Create string
 *******************************************************************/
String MAINTENANCE_string(void) {
  JsonDocument doc = MAINTENANCE_json();

  String text = "--- MAINTENANCE ---";

  text.concat("\r\nMAINTENANCE mode enabled: ");
  text.concat(doc[JSON_MAINTENANCE_ENABLED].as<bool>() ? "YES" : "NO");

  text.concat("\r\nEmergency stop: ");
  text.concat(doc[JSON_MAINTENANCE_ENABLED].as<bool>() ? "OK" : "STOP");

  text.concat("\r\nLift enabled: ");
  text.concat(doc[JSON_LIFT_ENABLED].as<bool>() ? "YES" : "NO");
  text.concat(", homing: ");
  text.concat(doc[JSON_LIFT_HOMING].as<bool>() ? "YES" : "NO");

  text.concat("\r\nLift UP: motor: ");
  text.concat(doc[JSON_LIFT_MOTOR_UP].as<bool>() ? "ON" : "OFF");
  text.concat(", sensor: ");
  text.concat(doc[JSON_LIFT_SENSOR_UP].as<bool>() ? "ON" : "OFF");

  text.concat("\r\nLift DOWN: motor: ");
  text.concat(doc[JSON_LIFT_MOTOR_DOWN].as<bool>() ? "ON" : "OFF");
  text.concat(", sensor: ");
  text.concat(doc[JSON_LIFT_SENSOR_DOWN].as<bool>() ? "ON" : "OFF");

  text.concat("\r\nDMC enabled: ");
  text.concat(doc[JSON_DMC_ENABLED].as<bool>() ? "YES" : "NO");

  text.concat("\r\nAZIMUTH enabled: ");
  text.concat(doc[JSON_AZIMUTH_ENABLED].as<bool>() ? "YES" : "NO");
  text.concat(", home: ");
  text.concat(doc[JSON_AZIMUTH_HOME].as<bool>() ? "YES" : "NO");

  text.concat("\r\nAZIMUTH voltages: left: ");
  text.concat(doc[JSON_AZIMUTH_LEFT_V].as<float>());
  text.concat(", right: ");
  text.concat(doc[JSON_AZIMUTH_RIGHT_V].as<float>());
  text.concat(", actual: ");
  text.concat(doc[JSON_AZIMUTH_ACTUAL_V].as<float>());

  text.concat("\r\nAZIMUTH steering: maunal: ");
  text.concat(doc[JSON_AZIMUTH_MANUAL].as<int>());
  text.concat(", calculated: ");
  text.concat(doc[JSON_AZIMUTH_STEERING].as<int>());

  text.concat("\r\nAZIMUTH output enabled:  ");
  text.concat(doc[JSON_AZIMUTH_OUTPUT_ENABLED].as<bool>() ? "YES" : "NO");

  text.concat("\r\nAZIMUTH delay-to-the-middle:  ");
  text.concat(doc[JSON_DELAY_TO_MIDDLE].as<int>());

  text.concat("\r\n");
  return text;
}

/********************************************************************
 * Getters and setters
 *******************************************************************/
/********************************************************************
 * Enables maintenance mode if the emergency stop is not active.
 *
 * This function enables maintenance mode if the emergency stop is not active.
 * The maintenance mode allows performing maintenance tasks on the system.
 *
 * @note If the emergency stop is active, the maintenance mode will not be enabled.
 *******************************************************************/
void MAINTENANCE_enable(void) {
  if (!EMERGENCY_STOP_active()) {
    maintenance_data[JSON_MAINTENANCE_ENABLED] = true;

    DMC_disable();
    LIFT_disable();
    AZIMUTH_disable();
    AZIMUTH_analog_disable();
  }
}

/********************************************************************
 * Disables the maintenance mode.
 *
 * This function disables maintenance mode and disables
 * the DMC, LIFT, and AZIMUTH subsystems.
 *******************************************************************/
void MAINTENANCE_disable(void) {
  maintenance_data[JSON_MAINTENANCE_ENABLED] = false;

  DMC_disable();
  LIFT_disable();
  AZIMUTH_disable();
  AZIMUTH_analog_disable();
}

/********************************************************************
 * Check if maintenance mode is enabled.
 *
 * @return true if maintenance mode is enabled, false otherwise.
 *******************************************************************/
bool MAINTENANCE_enabled(void) {
  return maintenance_data[JSON_MAINTENANCE_ENABLED].as<bool>();
}

/********************************************************************
 * DMC
 *******************************************************************/
static void MAINTENANCE_dmc_enable(void) {
  if (MAINTENANCE_enabled()) {
    DMC_enable();
  }
}

/********************************************************************
 * AZIMUTH
 *******************************************************************/
static void MAINTENANCE_azimuth_enable(void) {
  if (MAINTENANCE_enabled()) {
    AZIMUTH_enable();
  }
}

static void MAINTENANCE_analog_enable(void) {
  if (MAINTENANCE_enabled()) {
    AZIMUTH_analog_enable();
  }
}

static void MAINTENANCE_azimuth_homing(void) {
  maintenance_data[JSON_AZIMUTH_HOMING] = true;
  AZIMUTH_start_homing();
  azimuth_homing_timer = 20;
}

/********************************************************************
 * LIFT
 *******************************************************************/
static void MAINTENANCE_lift_enable(void) {
  if (MAINTENANCE_enabled()) {
    LIFT_enable();
  }
}

static void MAINTENANCE_lift_motor_up(void) {
  if (MAINTENANCE_enabled() && LIFT_enabled()) {
    LIFT_UP_on();
    LIFT_DOWN_off();
  }
}

static void MAINTENANCE_lift_motor_down(void) {
  if (MAINTENANCE_enabled() && LIFT_enabled()) {
    LIFT_DOWN_on();
    LIFT_UP_off();
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
    maintenance_data[JSON_LIFT_HOMING] = true;
    LIFT_start_homing();
    lift_homing_timer = 20;
  }
}

/********************************************************************
 * @brief Handles the maintenance commands received from the API.
 *
 * This function parses the JSON data and performs the corresponding actions based on the received commands.
 *
 * @param data The JSON data containing the maintenance commands.
 *
 * @return DeserializationError The error status of the JSON deserialization process.
 *********************************************************************/
int MAINTENANCE_command_handler(const char *data) {
  JsonDocument doc;
  int handeled = 0;

  DeserializationError error = deserializeJson(doc, data);
  if (error != DeserializationError::Ok) {
    return -1;  // DeserializationError
  }

#ifdef DEBUG_API
  String str;
  Serial.print(F("MAINTENACE COMMAND HANDLER received: "));
  serializeJson(doc, str);
  Serial.println(str);
#endif

  /* Maintenance mode active */
  if (doc.containsKey(JSON_MAINTENANCE_ENABLED)) {
    if (doc[JSON_MAINTENANCE_ENABLED].as<bool>() == true)
      CONTROLLER_request_maintenance();
    else
      MAINTENANCE_disable();
    handeled++;
  }

  /* Lift enable */
  if (doc.containsKey(JSON_LIFT_ENABLED)) {
    if (doc[JSON_LIFT_ENABLED].as<bool>() == true)
      MAINTENANCE_lift_enable();
    else
      LIFT_disable();
    handeled++;
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
    handeled++;
  }

  /* Lift UP */
  if (doc.containsKey(JSON_LIFT_MOTOR_UP)) {
    if (doc[JSON_LIFT_MOTOR_UP].as<bool>() == true)
      MAINTENANCE_lift_motor_up();
    handeled++;
  }

  /* Lift DOWN */
  if (doc.containsKey(JSON_LIFT_MOTOR_DOWN)) {
    if (doc[JSON_LIFT_MOTOR_DOWN].as<bool>() == true)
      MAINTENANCE_lift_motor_down();
    handeled++;
  }

  /* DMC enable */
  if (doc.containsKey(JSON_DMC_ENABLED)) {
    if (doc[JSON_DMC_ENABLED].as<bool>() == true)
      MAINTENANCE_dmc_enable();
    else
      DMC_disable();
    handeled++;
  }

  /* Steering enable */
  if (doc.containsKey(JSON_AZIMUTH_ENABLED)) {
    if (doc[JSON_AZIMUTH_ENABLED].as<bool>() == true)
      MAINTENANCE_azimuth_enable();
    else
      AZIMUTH_disable();
    handeled++;
  }

  /* Steering analog output enable */
  if (doc.containsKey(JSON_AZIMUTH_OUTPUT_ENABLED)) {
    if (doc[JSON_AZIMUTH_OUTPUT_ENABLED].as<bool>() == true)
      MAINTENANCE_analog_enable();
    else
      AZIMUTH_analog_disable();
    handeled++;
  }

  /* Steering set LEFT voltage */
  if (doc.containsKey(JSON_AZIMUTH_LEFT_V)) {
    AZIMUTH_set_left(doc[JSON_AZIMUTH_LEFT_V].as<float>());
    handeled++;
  }

  /* Steering set RIGHT voltage */
  if (doc.containsKey(JSON_AZIMUTH_RIGHT_V)) {
    AZIMUTH_set_right(doc[JSON_AZIMUTH_RIGHT_V].as<float>());
    handeled++;
  }

  /* Steering start homing */
  if (doc.containsKey(JSON_AZIMUTH_HOMING)) {
    if (doc[JSON_AZIMUTH_HOMING].as<bool>() == true)
      MAINTENANCE_azimuth_homing();
    handeled++;
  }

  /* Steering manual control (%) */
  if (doc.containsKey(JSON_AZIMUTH_MANUAL)) {
    AZIMUTH_set_manual(doc[JSON_AZIMUTH_MANUAL].as<int>());
    handeled++;
  }

  if (handeled == 0) {
#ifdef DEBUG_WEBSOCKET
    Serail.println(F("MAINTENANCE mode no commands handled"))
#endif
        handeled = -2;  // no command found
  }

  return handeled;  // Ok is handeled > 0
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

  if (MAINTENANCE_command_handler((const char *)data) < 0) {
    Serial.print(F("MAINTENANCE_command_handler failed: "));
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

  vTaskDelay(1000 / portTICK_PERIOD_MS);

  while (true) {
    WEBSOCKET_update_doc(MAINTENANCE_json());

    /* After countdown lift disable status homing */
    if (lift_homing_timer >= 0) {
      lift_homing_timer--;
      if (LIFT_HOME_sensor() || lift_homing_timer == 0) {
        maintenance_data[JSON_LIFT_HOMING] = false;
      }
    }

    /* After countdown azimuth disable status homing */
    if (azimuth_homing_timer >= 0) {
      azimuth_homing_timer--;
      if (azimuth_homing_timer == 0) {
        maintenance_data[JSON_AZIMUTH_HOMING] = false;
      }
    }

    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

/********************************************************************
 * Maintenance parts
 *******************************************************************/
static void MAINTENANCE_dmc_update(void) {
  // TOD: DMC update
}

static void MAINTENANCE_lift_update(void) {
  // TOD: LIFT update
}

static void MAINTENANCE_steering_update(void) {
  if (AZIMUTH_enabled() && AZIMUTH_analog_enabled()) {
    int value = AZIMUTH_get_manual();
    AZIMUTH_set_steering(value);
  }
}

/********************************************************************
 * Main task
 *******************************************************************/
void MAINTENANCE_main_task(void *parameter) {
  (void)parameter;

  while (true) {
    // Start maintenace mode
    if (MAINTENANCE_enabled()) {
      // Stop maintenance mode on emergency stop
      if (!EMERGENCY_STOP_active()) {
        MAINTENANCE_disable();
        continue;
      }

      MAINTENANCE_dmc_update();
      MAINTENANCE_lift_update();
      MAINTENANCE_steering_update();

      vTaskDelay(250 / portTICK_PERIOD_MS);
      continue;  // taks 4 x/sec.
    }

    /* Maintenance mode not active */
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

/********************************************************************
 *  Initialize tasks
 *
 *********************************************************************/
static void MAINTENANCE_setup_tasks(void) {
  xTaskCreate(MAINTENACE_websocket_task, "WebSocketServer task", 8192, NULL, 15, NULL);
  xTaskCreate(MAINTENANCE_main_task, "Maintenance main", 16000, NULL, 15, NULL);
}

/********************************************************************
 * Command Line handler(s)
 *********************************************************************/
static void MAINTENANCE_cli_handlers(void) {
  // cli.addCommand("maintenance", clicb_list_wifi);
  // cli.addCommand("mtc", clicb_list_wifi);
}

/********************************************************************
 * Setup
 *******************************************************************/
void MAINTENANCE_setup(void) {
  MAINTENANCE_json_init();
  MAINTENANCE_cli_handlers();
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
