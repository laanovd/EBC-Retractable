#include "Azimuth.h"

#include <Arduino.h>
#include <ArduinoJson.h>

#include "CLI.h"
#include "GPIO.h"
#include "Storage.h"

/********************************************************************
 * Global variables
 ********************************************************************/
static JsonDocument azimuth_data;

/********************************************************************
 * Setup variables
 ********************************************************************/
static void AZIMUTH_init_float(const char *key, int default_value) {
  float flt;
  if (STORAGE_get_float(key, flt)) {
    flt = default_value;
    STORAGE_set_float(key, flt);
  }
  azimuth_data[key] = flt;
}

static void AZIMUTH_setup_variables(void) {
  AZIMUTH_init_float(JSON_AZIMUTH_LEFT_V, JSON_AZIMUTH_LEFT_DEFAULT);
  AZIMUTH_init_float(JSON_AZIMUTH_RIGHT_V, JSON_AZIMUTH_RIGHT_DEFAULT);
}

/********************************************************************
 * Azimuth output
 ********************************************************************/
static void AZIMUTH_set_output(float value) {
    azimuth_data[JSON_AZIMUTH_ACTUAL] = value;
    analogWrite(AZIMUTH_PIN, (int)value);
}

/********************************************************************
 * Azimuth enable/disable
 ********************************************************************/
void AZIMUTH_enable() {
  ANALOG_OUT_enable();
}

void AZIMUTH_disable() {
  ANALOG_OUT_disable();
}

bool AZIMUTH_enabled() {
  return ANALOG_OUT_enabled();
}

void AZIMUTH_set_left(float value) {
  if ((value >= 0.0) && (value <= 5.0)) {
    azimuth_data[JSON_AZIMUTH_LEFT_V] = value;
  }
}

void AZIMUTH_set_right(float value) {
  if ((value >= 0.0) && (value <= 5.0)) {
    azimuth_data[JSON_AZIMUTH_RIGHT_V] = value;
  }
}

/********************************************************************
 * Output settings
 ********************************************************************/
float AZIMUH_get_actual(void) {
  return azimuth_data[JSON_AZIMUTH_ACTUAL].as<float>();
}

float AZIMUH_get_left(void) {
  return azimuth_data[JSON_AZIMUTH_LEFT_V].as<float>();
}

void AZIMUH_set_left(float value) {
  if ((value >= 0.0) && (value <= 10.0)) {
    azimuth_data[JSON_AZIMUTH_LEFT_V] = value;
  }
}

float AZIMUH_get_right(void) {
  return azimuth_data[JSON_AZIMUTH_RIGHT_V].as<float>();
}

void AZIMUH_set_right(float value) {
  if ((value >= 0.0) && (value <= 10.0)) {
    azimuth_data[JSON_AZIMUTH_RIGHT_V] = value;
  }
}

int AZIMUTH_get_steering(void) {
  return azimuth_data[JSON_AZIMUTH_STEERING].as<int>();
}

/********************************************************************
 * Scale (float)
 ********************************************************************/
float scalef(float A, float A1, float A2, float Min, float Max)
{
    long double percentage = (A-A1)/(A1-A2);
    return (percentage) * (Min-Max)+Min;
}

/********************************************************************
 * Set steering percentage
 ********************************************************************/
void AZIMUTH_set_steering(int value) {
  if ((value >= 0) && (value <= 100)) {

    azimuth_data[JSON_AZIMUTH_STEERING] = value;

    /* Scale */
    float azimuth_position = scalef(
        (float)value, 
        0.0, 
        100.0, 
        azimuth_data[JSON_AZIMUTH_LEFT_V].as<float>(), 
        azimuth_data[JSON_AZIMUTH_RIGHT_V].as<float>()
    );

    AZIMUTH_set_output(azimuth_position);
  }
}

/********************************************************************
 * Azimuth main loop
 ********************************************************************/
void AZIMUTH_update() {
  if (AZIMUTH_enabled()) {
    int wheel_position = STEERING_WHEEL_get_position();

    // 0 ... 100%
    azimuth_data[JSON_AZIMUTH_STEERING] = ((wheel_position * 100) / 4096);

    /* Scale */
    float azimuth_position = scalef(
        (float)wheel_position, 
        0.0, 
        4096.0, 
        azimuth_data[JSON_AZIMUTH_LEFT_V].as<float>(), 
        azimuth_data[JSON_AZIMUTH_RIGHT_V].as<float>()
    );

    AZIMUTH_set_output(azimuth_position);
  }
}

/********************************************************************
 * Azimuth setup
 ********************************************************************/
void AZIMUTH_setup() {
  AZIMUTH_setup_variables();
  AZIMUTH_disable();

  Serial.println(F("Azimuth setup completed..."));
}