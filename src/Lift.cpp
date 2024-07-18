/*******************************************************************
 * Lift.cpp
 *
 * EBC Retractable/lift control
 *
 *******************************************************************/
#include "Lift.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WebSerial.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

#include "CLI.h"
#include "Config.h"
#include "EBC_IOLib.h"
#include "GPIO.h"
#include "Storage.h"

/*******************************************************************
 * Definitions
 *******************************************************************/
#define DEBUG_LIFT

#define BUTTON_BOTH_DELAY 30;

/*******************************************************************
 * Storage keys and defaults
 *******************************************************************/
#define JSON_LIFT_MOVE_TIMEOUT "lift_move_timeout"
#define DELFAULT_LIFT_MOVE_TIMEOUT 30

#define JSON_RETRACTED_COUNT "retracted"
#define JSON_EXTENDED_COUNT "extended"

#define DOUBLE_PRESS_HOLD_TIME 5000

/*******************************************************************
 * Globals
 *******************************************************************/
static JsonDocument LIFT_data;

static int LIFT_error_mask = 0;

/********************************************************************
 * Create initial JSON data
 *******************************************************************/
static JsonDocument LIFT_json(void) {
  int value;

  LIFT_data[JSON_LIFT_ENABLED] = LIFT_enabled();

  LIFT_data[JSON_LIFT_MOTOR_UP] = LIFT_UP_moving();
  LIFT_data[JSON_LIFT_MOTOR_DOWN] = LIFT_DOWN_moving();

  LIFT_data[JSON_LIFT_SENSOR_UP] = LIFT_UP_sensor();
  LIFT_data[JSON_LIFT_SENSOR_DOWN] = LIFT_DOWN_sensor();

  STORAGE_get_int(JSON_RETRACTED_COUNT, value);
  LIFT_data[JSON_RETRACTED_COUNT] = value;

  STORAGE_get_int(JSON_EXTENDED_COUNT, value);
  LIFT_data[JSON_EXTENDED_COUNT] = value;

  return LIFT_data;
}

/********************************************************************
 * Create string
 *******************************************************************/
static String LIFT_info_str(void) {
  JsonDocument doc = LIFT_json();  // Update

  String text = "--- LIFT ---";

  text.concat("\r\nEnabled: ");
  text.concat(doc[JSON_LIFT_ENABLED].as<boolean>());

  text.concat("\r\nMoving UP: ");
  text.concat(doc[JSON_LIFT_MOTOR_UP].as<boolean>());
  text.concat(", DOWN: ");
  text.concat(doc[JSON_LIFT_MOTOR_DOWN].as<boolean>());

  text.concat("\r\nSensors UP: ");
  text.concat(doc[JSON_LIFT_SENSOR_UP].as<boolean>());
  text.concat(", DOWN: ");
  text.concat(doc[JSON_LIFT_SENSOR_DOWN].as<boolean>());

  text.concat("\r\nMoving up/down timeout: ");
  text.concat(doc[JSON_LIFT_MOVE_TIMEOUT].as<int>());

  text.concat("\r\nTimes retracted: ");
  text.concat(doc[JSON_RETRACTED_COUNT].as<int>());

  text.concat("\r\nTimes extended: ");
  text.concat(doc[JSON_EXTENDED_COUNT].as<int>());

  text.concat("\r\n");
  return text;
}

/*******************************************************************
 * LIFT getters
 *******************************************************************/
int LIFT_move_timeout(void) {
  return LIFT_data[JSON_LIFT_MOVE_TIMEOUT].as<int>();
}

bool LIFT_error(void) {
  // return (LIFT_error_mask != 0);
  return false;
}

/*******************************************************************
 * LIFT enable
 *******************************************************************/
void LIFT_enable(void) {
  PCF8574_write(PCF8574_address, LIFT_ENABLE_PIN, IO_ON);

#ifdef DEBUG_LIFT
  Serial.println("LIFT enable");
#endif
}
void LIFT_disable(void) {
  PCF8574_write(PCF8574_address, LIFT_ENABLE_PIN, IO_OFF);

#ifdef DEBUG_LIFT
  Serial.println("LIFT disable");
#endif
}

bool LIFT_enabled(void) {
  return PCF8574_read(PCF8574_address, LIFT_ENABLE_PIN) == IO_ON;
}

/*******************************************************************
 * Lift motor
 *******************************************************************/
void LIFT_UP_on(void) {
  PCF8574_write(PCF8574_address, LIFT_MOTOR_UP_PIN, IO_ON);

#ifdef DEBUG_LIFT
  Serial.println("Lift motor up ON");
#endif
}

void LIFT_UP_off(void) {
  PCF8574_write(PCF8574_address, LIFT_MOTOR_UP_PIN, IO_OFF);

#ifdef DEBUG_LIFT
  Serial.println("Lift motor up OFF");
#endif
}

bool LIFT_UP_moving(void) {
  return PCF8574_read(PCF8574_address, LIFT_MOTOR_UP_PIN);
}

bool LIFT_DOWN_moving(void) {
  return PCF8574_read(PCF8574_address, LIFT_MOTOR_DOWN_PIN);
}

void LIFT_DOWN_on(void) {
  PCF8574_write(PCF8574_address, LIFT_MOTOR_DOWN_PIN, IO_ON);

#ifdef DEBUG_LIFT
  Serial.println("Lift motor down ON");
#endif
}

void LIFT_DOWN_off(void) {
  PCF8574_write(PCF8574_address, LIFT_MOTOR_DOWN_PIN, IO_OFF);

#ifdef DEBUG_LIFT
  Serial.println("Lift motor down OFF");
#endif
}

void LIFT_start_homing() {
  PCF8574_write(PCF8574_address, LIFT_START_HOMING_PIN, IO_ON);
  vTaskDelay(500 / portTICK_PERIOD_MS);
  PCF8574_write(PCF8574_address, LIFT_START_HOMING_PIN, IO_OFF);
}

/*******************************************************************
 * Lift position
 *******************************************************************/
bool LIFT_UP_sensor(void) {
  return digitalRead(LIFT_SENSOR_UP_PIN) == HIGH ? true : false;
}

bool LIFT_DOWN_sensor(void) {
  return digitalRead(LIFT_SENSOR_DOWN_PIN) == HIGH ? true : false;
}

bool LIFT_HOME_sensor(void) {
  return digitalRead(LIFT_SENSOR_HOME_PIN) == HIGH ? true : false;
}

static void LIFT_position_check(void) {
  if (!LIFT_UP_moving() && LIFT_DOWN_moving()) {
    if (!LIFT_UP_sensor() && !LIFT_DOWN_sensor()) {
      LIFT_error_mask |= 0x0001;  // No position error
      return;
    }
  }

  LIFT_error_mask &= ~0x0001;  // Clear error
}

/*******************************************************************
 * Lift buttons
 *******************************************************************/
static bool BUTTON_UP_pushed = false;
static bool BUTTON_DOWN_pushed = false;
static bool BUTTON_BOTH_pushed = false;

bool LIFT_UP_button(void) {
  if (BUTTON_UP_pushed) {
    BUTTON_UP_pushed = false;
#ifdef DEBUG_LIFT
    Serial.println("LIFT UP button pushed");
#endif
    return true;
  }
  return false;
}

bool LIFT_DOWN_button(void) {
  if (BUTTON_DOWN_pushed) {
    BUTTON_DOWN_pushed = false;
#ifdef DEBUG_LIFT
    Serial.println("LIFT DOWN button pushed");
#endif
    return true;
  }
  return false;
}

bool LIFT_BOTH_button(void) {
  if (BUTTON_BOTH_pushed) {
    BUTTON_BOTH_pushed = false;
    return true;
  }
  return false;
}

// * Edit this for active high/low
static void LIFT_button_update(void) {
  static bool BUTTON_UP_state, BUTTON_UP_memo = false;
  static bool BUTTON_DOWN_state, BUTTON_DOWN_memo = false;
  static int delay = BUTTON_BOTH_DELAY;

  // Button up flag
  BUTTON_UP_state = digitalRead(LIFT_BUTTON_UP_PIN);
  if (BUTTON_UP_memo && !BUTTON_UP_state && !BUTTON_DOWN_state)
    BUTTON_UP_pushed = true;
  BUTTON_UP_memo = BUTTON_UP_state;

  // Button down flag
  BUTTON_DOWN_state = digitalRead(LIFT_BUTTON_DOWN_PIN);
  if (BUTTON_DOWN_memo && !BUTTON_DOWN_state && !BUTTON_UP_state)
    BUTTON_DOWN_pushed = true;
  BUTTON_DOWN_memo = BUTTON_DOWN_state;

  // Both buttons pushed
  if (BUTTON_UP_state && BUTTON_DOWN_state) {
    if (delay >= 0) {
      delay--;
      if (delay == 0) {
        BUTTON_UP_memo = BUTTON_DOWN_memo = false;
        BUTTON_UP_pushed = BUTTON_DOWN_pushed = false;
        BUTTON_BOTH_pushed = true;
      }
    }
    return;
  }
  delay = BUTTON_BOTH_DELAY;  // Run every 100ms.
}

/*******************************************************************
 * LIFT counters
 *******************************************************************/
void LIFT_retected_increment(void) {
  int value;
  STORAGE_get_int(JSON_RETRACTED_COUNT, value);
  value++;
  STORAGE_set_int(JSON_RETRACTED_COUNT, value);
}

void LIFT_extended_increment(void) {
  int value;
  STORAGE_get_int(JSON_EXTENDED_COUNT, value);
  value++;
  STORAGE_set_int(JSON_EXTENDED_COUNT, value);
}

/*******************************************************************
 * Lift LED
 *******************************************************************/
static void LIFT_LED_up(void) {
  if (LIFT_error()) {
    digitalWrite(LIFT_LED_UP_PIN, !LED_error_takt());
    return;
  } 
  
  if (!LIFT_DOWN_moving()) {
    if (LIFT_UP_moving()) {
      digitalWrite(LIFT_LED_UP_PIN, !LED_blink_takt()); 
      return;
    }

    if (!LIFT_UP_sensor() && !LIFT_DOWN_sensor()) {
      digitalWrite(LIFT_LED_UP_PIN, !LED_blink_takt()); 
      return;
    }
  }
 
  digitalWrite(LIFT_LED_UP_PIN, (LIFT_UP_sensor()) ? HIGH : LOW); // Inverted logic.
}

static void LIFT_LED_down(void) {
  if (LIFT_error()) {
    digitalWrite(LIFT_LED_DOWN_PIN, !LED_error_takt());
    return;
  } 
  
  if (!LIFT_UP_moving()) {
    if (LIFT_DOWN_moving()) {
      digitalWrite(LIFT_LED_DOWN_PIN, !LED_blink_takt()); 
      return;
    }

    if (!LIFT_DOWN_sensor() && !LIFT_UP_sensor()) {
      digitalWrite(LIFT_LED_DOWN_PIN, !LED_blink_takt()); 
      return;
    }
  }
 
  digitalWrite(LIFT_LED_DOWN_PIN, (LIFT_DOWN_sensor()) ? HIGH : LOW); // Inverted logic.
}

/*******************************************************************
 * Main task
 *******************************************************************/
static void LIFT_main_task(void *parameter) {
  (void)parameter;

  while (true) {
    LIFT_LED_up();
    LIFT_LED_down();

    LIFT_position_check();

    LIFT_button_update();

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

/********************************************************************
 * REST API
 *********************************************************************/
static void LIFT_rest_read(AsyncWebServerRequest *request) {
  String str;
  serializeJson(LIFT_json(), str);
  request->send(200, "application/json", str.c_str());
}

static rest_api_t LIFT_api_handlers = {
    /* uri */ "/api/v1/lift",
    /* comment */ "LIFT module",
    /* instances */ 1,
    /* fn_create */ nullptr,
    /* fn_read */ LIFT_rest_read,
    /* fn_update */ nullptr,
    /* fn_delete */ nullptr,
};

/********************************************************************
 * CLI handler
 *******************************************************************/
static void clicb_handler(cmd *c) {
  Command cmd(c);
  Argument arg = cmd.getArg(0);
  String strArg = arg.getValue();

  /* List settings */
  if (strArg.isEmpty()) {
    CLI_println(LIFT_info_str());
    return;
  }

  arg = cmd.getArg(0);
  strArg = arg.getValue();
  int val = cmd.getArg(1).getValue().toInt();

  if (strArg.equalsIgnoreCase("timeout")) {
    if ((val < 0) || (val > 120)) {
      CLI_println("Illegal value, range: 0 ... 120s.");
      return;
    }
    STORAGE_set_int(JSON_LIFT_MOVE_TIMEOUT, val);
    CLI_println("Retractable timeout has been set to: " + String(val) + "sec.");
  }

  CLI_println("Invalid command: LIFT (timeout <n>).");
}

/********************************************************************
 * Command Line handler(s)
 *********************************************************************/
static void LIFT_setup_cli(void) {
  cli.addCommand("lift", clicb_handler);
}

/*******************************************************************
 * GPIO setup
 *******************************************************************/
static void LIFT_setup_gpio(void) {
  pinMode(LIFT_SENSOR_UP_PIN, INPUT);
  pinMode(LIFT_SENSOR_DOWN_PIN, INPUT);
  pinMode(LIFT_BUTTON_UP_PIN, INPUT);
  pinMode(LIFT_BUTTON_DOWN_PIN, INPUT);
  pinMode(LIFT_LED_UP_PIN, OUTPUT);
  pinMode(LIFT_LED_DOWN_PIN, OUTPUT);

  /* LED startup cycle */
  digitalWrite(LIFT_LED_UP_PIN, LOW);     // Inverted
  digitalWrite(LIFT_LED_DOWN_PIN, LOW);   // Inverted
  vTaskDelay(1000 / portTICK_PERIOD_MS);  // Wait one second
  digitalWrite(LIFT_LED_UP_PIN, HIGH);    // Inverted
  digitalWrite(LIFT_LED_DOWN_PIN, HIGH);  // Inverted
}

/*******************************************************************
  Setup tasks
 *******************************************************************/
static void LIFT_setup_tasks(void) {
  xTaskCreate(LIFT_main_task, "Lift main", 2048, NULL, 10, NULL);
}

/*******************************************************************
  Setup variables
 *******************************************************************/
static void LIFT_setup_variables(void) {
  int value;

  // Maximum lift move up/down time in seconds
  if (STORAGE_get_int(JSON_LIFT_MOVE_TIMEOUT, value)) {
    value = DELFAULT_LIFT_MOVE_TIMEOUT;
    STORAGE_set_int(JSON_LIFT_MOVE_TIMEOUT, value);
  }
  LIFT_data[JSON_LIFT_MOVE_TIMEOUT] = value;

  /* Initialize if not exist */
  if (STORAGE_get_int(JSON_RETRACTED_COUNT, value)) {
    STORAGE_set_int(JSON_LIFT_MOVE_TIMEOUT, 0);
  }

  if (STORAGE_get_int(JSON_EXTENDED_COUNT, value)) {
    STORAGE_set_int(JSON_EXTENDED_COUNT, 0);
  }
}

/*******************************************************************
 * Lift general
 *******************************************************************/
void LIFT_setup(void) {
  LIFT_setup_variables();
  LIFT_setup_gpio();

  LIFT_disable();
  LIFT_UP_off();
  LIFT_DOWN_off();

  Serial.println(F("Lift setup completed..."));
}

void LIFT_start(void) {
  LIFT_setup_tasks();
  LIFT_setup_cli();
  setup_uri(&LIFT_api_handlers);

  LIFT_error_mask = 1;  // TODO: Remove after testing

  Serial.println(F("Lift started..."));
}
