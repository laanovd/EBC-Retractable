/*******************************************************************
 * Azimuth.cpp
 *    
 * EBC azimuth control
 * 
 *******************************************************************/
#include "Azimuth.h"

#include <Arduino.h>
#include <ArduinoJson.h>

#include "Config.h"
#include "Storage.h"
#include "CLI.h"
#include "GPIO.h"
#include "EBC_IOlib.h" 

/*******************************************************************
 * Definitions
 *******************************************************************/
#define DEBUG_AZIMUTH

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
 * Create azimuth string
 *******************************************************************/
String AZIMUTH_info(void)
{
  float tmp;
  String text = "--- Azimuth ---";

  text.concat("\r\nAzimuth enabled: ");
  text.concat(AZIMUTH_enabled());

  text.concat("\r\nAzimuth left: ");
  STORAGE_get_float(JSON_AZIMUTH_LEFT_V, tmp);
  text.concat(String(tmp));

  text.concat("\r\nAzimuth right: ");
  STORAGE_get_float(JSON_AZIMUTH_RIGHT_V, tmp);
  text.concat(String(tmp));

  text.concat("\r\nAzimuth delay: ");
  STORAGE_get_float(JSON_DELAY_TO_MIDDLE, tmp);
  text.concat(String(tmp));

  text.concat("\r\n");
  return text;
}

/*******************************************************************
 * AZIMUTH analog 
 *******************************************************************/
static void AZIMUTH_right_set(int value) {
  MCP4725_write(MCP4725_R_address, value);

#ifdef DEBUG_AZIMUTH
  Serial.println("Azimuth set analog out: " + String(value));
#endif
}

/*******************************************************************
    Steering wheel
 *******************************************************************/
int STEERING_WHEEL_get_position(void) {
  return analogRead(WHEEL_PIN);
}

/*******************************************************************
 * Azimuth enable/disable
 *******************************************************************/
static void AZIMUTH_set_left(float value) {
  if ((value >= 0.0) && (value <= 5.0)) {
    AZIMUTH_data[JSON_AZIMUTH_LEFT_V] = value;
  }
}

static void AZIMUTH_set_right(float value) {
  if ((value >= 0.0) && (value <= 5.0)) {
    AZIMUTH_data[JSON_AZIMUTH_RIGHT_V] = value;
  }
}

void AZIMUTH_enable(void) {
  PCF8574_write(PCF8574_address,AZIMUTH_ENABLE_PIN, IO_ON);

#ifdef DEBUG_AZIMUTH
  Serial.println("Azimuth enable");
#endif
}

void AZIMUTH_disable(void) {
  PCF8574_write(PCF8574_address,AZIMUTH_ENABLE_PIN, IO_OFF);

#ifdef DEBUG_AZIMUTH
  Serial.println("Azimuth disable");
#endif
}

bool AZIMUTH_enabled() {
  return PCF8574_read(PCF8574_address, AZIMUTH_ENABLE_PIN) == IO_ON;
}

/*******************************************************************
 * Output settings
 *******************************************************************/
float AZIMUH_get_actual(void) {
  return AZIMUTH_data[JSON_AZIMUTH_ACTUAL_V].as<float>();
}

float AZIMUH_get_left(void) {
  return AZIMUTH_data[JSON_AZIMUTH_LEFT_V].as<float>();
}

void AZIMUH_set_left(float value) {
  if ((value >= 0.0) && (value <= 10.0)) {
    AZIMUTH_data[JSON_AZIMUTH_LEFT_V] = value;
  }
}

float AZIMUH_get_right(void) {
  return AZIMUTH_data[JSON_AZIMUTH_RIGHT_V].as<float>();
}

void AZIMUH_set_right(float value) {
  if ((value >= 0.0) && (value <= 10.0)) {
    AZIMUTH_data[JSON_AZIMUTH_RIGHT_V] = value;
  }
}

int AZIMUTH_to_the_middle_delay(void) {
  return AZIMUTH_data[JSON_DELAY_TO_MIDDLE].as<int>();
}

static void AZIMUTH_set_delay_to_the_middle(int value) {
  STORAGE_set_int(JSON_DELAY_TO_MIDDLE, value);
  AZIMUTH_data[JSON_DELAY_TO_MIDDLE] = value;
}

/*******************************************************************
 * Scale (float)
 *******************************************************************/
float scalef(float A, float A1, float A2, float Min, float Max)
{
    long double percentage = (A-A1)/(A1-A2);
    return (percentage) * (Min-Max)+Min;
}

/*******************************************************************
 * Set steering percentage
 *******************************************************************/
void AZIMUTH_set_steering(int value) {
  if ((value >= 0) && (value <= 100)) {

    AZIMUTH_data[JSON_AZIMUTH_STEERING] = value;

    /* Scale */
    float azimuth_position = scalef(
        (float)value, 
        0.0, 
        100.0, 
        AZIMUTH_data[JSON_AZIMUTH_LEFT_V].as<float>(), 
        AZIMUTH_data[JSON_AZIMUTH_RIGHT_V].as<float>()
    );

    AZIMUTH_set_right(azimuth_position);
  }
}

int AZIMUTH_get_steering(void) {
  return AZIMUTH_data[JSON_AZIMUTH_STEERING].as<int>();
}

/*******************************************************************
 * Azimuth main loop
 *******************************************************************/
void AZIMUTH_update() {
  if (AZIMUTH_enabled()) {
    int wheel_position = STEERING_WHEEL_get_position();

    // 0 ... 100%
    AZIMUTH_data[JSON_AZIMUTH_STEERING] = ((wheel_position * 100) / 4096);

    /* Scale */
    float azimuth_position = scalef(
        (float)wheel_position, 
        0.0, 
        4096.0, 
        AZIMUTH_data[JSON_AZIMUTH_LEFT_V].as<float>(), 
        AZIMUTH_data[JSON_AZIMUTH_RIGHT_V].as<float>()
    );

    AZIMUTH_right_set(azimuth_position);
  }
}

/********************************************************************
 * CLI handler
 *******************************************************************/
// TODO: move to azimuth file
// TODO: update local json variable
// TODO: check if right > left on setting left/right
static void clicb_handler(cmd *c)
{
  Command cmd(c);
  Argument arg = cmd.getArg(0);
  String strArg = arg.getValue();

  /* List settings */
  if (strArg.isEmpty())
  {
    CLI_println(AZIMUTH_info());
    return;
  }

  if (strArg.equalsIgnoreCase("left"))
  {
    float val = cmd.getArg(1).getValue().toFloat();
    if ((val < 0.0) || (val > 5.0)) {
      CLI_println("Illegal value, range: 0.0V ... 5.0V");
      return;
    }
    STORAGE_set_float(JSON_AZIMUTH_LEFT_V, val);
    CLI_println("Azimuth left limit has been set to " + String(val) + " Volt");
  }

  if (strArg.equalsIgnoreCase("right"))
  {
    float val = cmd.getArg(1).getValue().toFloat();
    if ((val < 0.0) || (val > 5.0)) {
      CLI_println("Illegal value, range: 0.0V ... 5.0V");
      return;
    }
    STORAGE_set_float(JSON_AZIMUTH_RIGHT_V, val);
    CLI_println("Azimuth right limit has been set to " + String(val) + " Volt");
  }

  if (strArg.equalsIgnoreCase("delay"))
  {
    int val = cmd.getArg(1).getValue().toInt();
    if ((val < 0) || (val > 60)) {
      CLI_println("Illegal value, range: 0 ... 60s");
      return;
    }
    AZIMUTH_set_delay_to_the_middle(val);
    CLI_println("Azimuth delay-to-middle has been set to " + String(val) + " seconds");
  }

  if (strArg.equalsIgnoreCase("move") && is_calibrating())
  {
    // AZIMUTH_set_position(val);
  }

  CLI_println("Invalid command: AZIMUTH (left <n>, right <n>, delay <n>, move).");
}

static void cli_setup(void)
{
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
