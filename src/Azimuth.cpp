/*******************************************************************
 * Azimuth.cpp
 *
 * EBC azimuth control
 *
 *******************************************************************/
#include "Azimuth.h"

#include <Arduino.h>
#include <ArduinoJson.h>

#include "CLI.h"
#include "Config.h"
#include "EBC_IOLib.h"
#include "GPIO.h"
#include "Maintenance.h"
#include "Storage.h"

/*******************************************************************
 * Definitions
 *******************************************************************/
#define DEBUG_AZIMUTH

#define ADC_MIN 0
#define ADC_MAX 4096
#define DAC_MIN 0
#define DAC_MAX 4996

#undef ENABLE_LEFT_OUTPUT  // No left output use yet

/*******************************************************************
 * Storage keys and defaults
 *******************************************************************/
#define JSON_AZIMUTH_LEFT_DEFAULT 0.0
#define JSON_AZIMUTH_RIGHT_DEFAULT 5.0
#define DELAY_TO_MIDDLE_DEFAULT 5

/*******************************************************************
 * Global variables
 *******************************************************************/
static JsonDocument AZIMUTH_data;

/********************************************************************
 * Create initial JSON data
 *******************************************************************/
static JsonDocument AZIMUTH_json(void) {
  float f;
  int i;

  AZIMUTH_data[JSON_AZIMUTH_ENABLED] = AZIMUTH_enabled();
  AZIMUTH_data[JSON_AZIMUTH_HOME] = AZIMUTH_home();

  AZIMUTH_data[JSON_AZIMUTH_OUTPUT_ENABLED] = AZIMUTH_output_enabled();

  AZIMUTH_data[JSON_AZIMUTH_LEFT_V] = AZIMTUH_get_left();
  AZIMUTH_data[JSON_AZIMUTH_RIGHT_V] = AZIMTUH_get_right();
  AZIMUTH_data[JSON_AZIMUTH_ACTUAL_V] = AZIMTUH_get_actual();

  AZIMUTH_data[JSON_DELAY_TO_MIDDLE] = AZIMUTH_to_the_middle_delay();

  AZIMUTH_data[JSON_AZIMUTH_STEERING] = AZIMUTH_get_wheel();  // 0 ... 100%

  AZIMUTH_data[JSON_DELAY_TO_MIDDLE] = AZIMUTH_to_the_middle_delay();

  return AZIMUTH_data;
}

/********************************************************************
 * Create azimuth string
 *******************************************************************/
String AZIMUTH_info(void) {
  static JsonDocument doc = AZIMUTH_json();

  String text = "--- Azimuth ---";

  text.concat("\r\nAzimuth enabled: ");
  text.concat(doc[JSON_AZIMUTH_ENABLED].as<bool>());

  text.concat("\r\nAzimuth left: ");
  text.concat(doc[JSON_AZIMUTH_LEFT_V].as<float>());

  text.concat("\r\nAzimuth right: ");
  text.concat(doc[JSON_AZIMUTH_RIGHT_V].as<float>());

  text.concat("\r\nAzimuth right: ");
  text.concat(doc[JSON_AZIMUTH_RIGHT_V].as<float>());

  text.concat("\r\nAzimuth actual: ");
  text.concat(doc[JSON_AZIMUTH_ACTUAL_V].as<bool>());

  text.concat("\r\nAzimuth delay: ");
  text.concat(doc[JSON_DELAY_TO_MIDDLE].as<int>());

  text.concat("\r\n");
  return text;
}

/*******************************************************************
 * Scale functions
 *******************************************************************/
float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
  if ((in_max - in_min) != 0) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  }
  return 0.0;
}

long mapl(long x, long in_min, long in_max, long out_min, long out_max) {
  if ((in_max - in_min) != 0) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  }
  return 0L;
}

/*******************************************************************
 * AZIMUTH analog
 *******************************************************************/
static void AZIMUTH_set_right_output(int value) {
  value = max(min(DAC_MIN, value), DAC_MAX);
  MCP4725_write(MCP4725_R_address, (int)value);

#ifdef DEBUG_AZIMUTH
  Serial.println("Azimuth set right analog out: " + String(value));
#endif
}

static int AZIMUTH_get_right_output(void) {
  int value = MCP4725_read_status(MCP4725_R_address);
  return (value != DAC_READ_ERROR) ? value : 0;
}

#ifdef ENABLE_LEFT_OUTPUT
static void AZIMUTH_set_left_output(int value) {
  value = max(min(DAC_MIN, value), DAC_MAX);
  MCP4725_write(MCP4725_L_address, (int)value);

#ifdef DEBUG_AZIMUTH
  Serial.println("Azimuth set left analog out: " + String(value));
#endif
}
#endif

#ifdef ENABLE_LEFT_OUTPUT
static int AZIMUTH_get_left_output(void) {
  int value = MCP4725_read_status(MCP4725_L_address);
  return (value != DAC_READ_ERROR) ? value : 0;
}
#endif

static void AZIMUTH_right_set(int value) {
  MCP4725_write(MCP4725_R_address, value);

#ifdef DEBUG_AZIMUTH
  Serial.println("Azimuth set analog out: " + String(value));
#endif
}

/*******************************************************************
    Steering wheel
 *******************************************************************/
static int STEERING_WHEEL_read(void) {
  return analogRead(STEER_WHEEL_PIN);
}

/*******************************************************************
 * Azimuth enable/disable
 *******************************************************************/
void AZIMUTH_enable(void) {
  PCF8574_write(PCF8574_address, AZIMUTH_ENABLE_PIN, IO_ON);

#ifdef DEBUG_AZIMUTH
  Serial.println("Azimuth enable");
#endif
}

void AZIMUTH_disable(void) {
  PCF8574_write(PCF8574_address, AZIMUTH_ENABLE_PIN, IO_OFF);

#ifdef DEBUG_AZIMUTH
  Serial.println("Azimuth disable");
#endif
}

bool AZIMUTH_enabled() {
  return PCF8574_read(PCF8574_address, AZIMUTH_ENABLE_PIN) == IO_ON;
}

bool AZIMUTH_home() {
  return PCF8574_read(PCF8574_address, AZIMUTH_HOME_PIN) == IO_ON;
}

void AZIMUTH_start_homing() {
  PCF8574_write(PCF8574_address, AZIMUTH_START_HOMING_PIN, IO_ON);
  vTaskDelay(500 / portTICK_PERIOD_MS);
  PCF8574_write(PCF8574_address, AZIMUTH_START_HOMING_PIN, IO_ON);
}

bool AZIMUTH_output_enabled() {
  return digitalRead(AZIMUTH_ANALOG_ENABLE_PIN) == HIGH;
}

void AZIMUTH_output_enable() {
  digitalWrite(AZIMUTH_ANALOG_ENABLE_PIN, IO_ON);
}

void AZIMUTH_output_disable() {
  digitalWrite(AZIMUTH_ANALOG_ENABLE_PIN, IO_OFF);
}

/*******************************************************************
 * Get/Set steering
 *******************************************************************/
int AZIMUTH_get_wheel(void) {
  /* Scale */
  int position = map(
      STEERING_WHEEL_read(),
      ADC_MIN,
      ADC_MAX,
      0,
      100);

  return position;  // %
}

void AZIMUTH_set_steering(int value) {
  value = max(min(1024, value), 0);

  float output = mapf(
      (float)value,
      ADC_MIN,
      ADC_MAX,
      0.0,
      5.0);

  AZIMUTH_set_right_output((int)round(output));
#ifdef ENABLE_LEFT_OUTPUT
  AZIMUTH_set_left_output((int)round(output));
#endif
}

static void AZIMUTH_set_output_manual(int value) {
  value = max(min(100, value), 0);

  int output = map(
      value,
      0, 
      100,
      DAC_MIN,
      DAC_MAX);

  AZIMUTH_set_steering(output);
}

float AZIMUTH_get_steering(void) {
  int value = AZIMUTH_get_right_output();

  float output = mapf(
      (float)value,
      0.0,
      1024.0,
      0.0,
      5.0);

  return output;
}

float AZIMTUH_get_actual(void) {
  int value = AZIMUTH_get_right_output();

  float output = mapf(
      (float)value,
      ADC_MAX,
      ADC_MAX,
      0.0,
      5.0);

  return output;
}

/*******************************************************************
 * Output settings
 *******************************************************************/
float AZIMTUH_get_left(void) {
  float value;
  STORAGE_get_float(JSON_AZIMUTH_LEFT_V, value);
  return value;
}

void AZIMTUH_set_left(float value) {
  if ((value >= 0.0) && (value <= 5.0)) {
    STORAGE_set_float(JSON_AZIMUTH_LEFT_V, value);
  }
}

float AZIMTUH_get_right(void) {
  float value;
  STORAGE_get_float(JSON_AZIMUTH_RIGHT_V, value);
  return value;
}

void AZIMTUH_set_right(float value) {
  if ((value >= 0.0) && (value <= 5.0)) {
    STORAGE_set_float(JSON_AZIMUTH_RIGHT_V, value);
  }
}

void AZIMUTH_set_manual(int value) {
  if ((value >= 0) && (value <= 100)) {
    AZIMUTH_data[JSON_AZIMUTH_MANUAL] = value;
    AZIMUTH_set_output_manual(value);
  }
}

int AZIMUTH_get_manual(void) {
  return AZIMUTH_data[JSON_AZIMUTH_MANUAL];
}

int AZIMUTH_to_the_middle_delay(void) {
  int value;
  STORAGE_set_int(JSON_DELAY_TO_MIDDLE, value);
  return value;
}

static void AZIMUTH_set_delay_to_the_middle(int value) {
  STORAGE_set_int(JSON_DELAY_TO_MIDDLE, value);
  AZIMUTH_data[JSON_DELAY_TO_MIDDLE] = value;
}

/*******************************************************************
 * Azimuth main loop
 *******************************************************************/
void AZIMUTH_update(void *parameter) {
  (void)parameter;
  int i;

  while (true) {
    if (AZIMUTH_enabled() && AZIMUTH_output_enabled()) {
      i = STEERING_WHEEL_read();  // 0...1024
      AZIMUTH_set_steering(i);    // 0...1024
    }

    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

/********************************************************************
 * CLI handler
 *******************************************************************/
// TODO: move to azimuth file
// TODO: update local json variable
// TODO: check if right > left on setting left/right
static void clicb_handler(cmd *c) {
  Command cmd(c);
  Argument arg = cmd.getArg(0);
  String strArg = arg.getValue();

  /* List settings */
  if (strArg.isEmpty()) {
    CLI_println(AZIMUTH_info());
    return;
  }

  if (strArg.equalsIgnoreCase("left")) {
    float val = cmd.getArg(1).getValue().toFloat();
    if ((val < 0.0) || (val > 5.0)) {
      CLI_println("Illegal value, range: 0.0V ... 5.0V");
      return;
    }
    STORAGE_set_float(JSON_AZIMUTH_LEFT_V, val);
    CLI_println("Azimuth left limit has been set to " + String(val) + " Volt");
  }

  if (strArg.equalsIgnoreCase("right")) {
    float val = cmd.getArg(1).getValue().toFloat();
    if ((val < 0.0) || (val > 5.0)) {
      CLI_println("Illegal value, range: 0.0V ... 5.0V");
      return;
    }
    STORAGE_set_float(JSON_AZIMUTH_RIGHT_V, val);
    CLI_println("Azimuth right limit has been set to " + String(val) + " Volt");
  }

  if (strArg.equalsIgnoreCase("delay")) {
    int val = cmd.getArg(1).getValue().toInt();
    if ((val < 0) || (val > 60)) {
      CLI_println("Illegal value, range: 0 ... 60s");
      return;
    }
    AZIMUTH_set_delay_to_the_middle(val);
    CLI_println("Azimuth delay-to-middle has been set to " + String(val) + " seconds");
  }

  if (strArg.equalsIgnoreCase("move") && is_calibrating()) {
    // AZIMUTH_set_position(val);
  }

  CLI_println("Invalid command: AZIMUTH (left <n>, right <n>, delay <n>, move).");
}

static void cli_setup(void) {
  cli.addBoundlessCmd("azimuth", clicb_handler);
}

/*******************************************************************
 * Setup variables
 *******************************************************************/
static void AZIMUTH_setup_variables(void) {
  int value;

  if (STORAGE_get_int(JSON_DELAY_TO_MIDDLE, value)) {
    value = DELAY_TO_MIDDLE_DEFAULT;
    STORAGE_set_int(JSON_DELAY_TO_MIDDLE, value);
  }
  AZIMUTH_data[JSON_DELAY_TO_MIDDLE] = value;
}

/*******************************************************************
 * Azimuth general
 *******************************************************************/
void AZIMUTH_setup() {
  AZIMUTH_setup_variables();
  AZIMUTH_disable();

  Serial.println(F("Azimuth setup completed..."));
}

void AZIMUTH_start() {
  cli_setup();

  Serial.println(F("Azimuth started..."));
}
