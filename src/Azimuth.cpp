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
static bool AZIMUTH_is_enabled = false;

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
  AZIMUTH_init_float(JSON_AZIMUTH_LEFT, JSON_AZIMUTH_LEFT_DEFAULT);
  AZIMUTH_init_float(JSON_AZIMUTH_RIGHT, JSON_AZIMUTH_RIGHT_DEFAULT);
}

/********************************************************************
 * Azimuth output
 ********************************************************************/
static void AZIMUTH_set_output(float value) {
    analogWrite(AZIMUTH_PIN, (int)value);
}

/********************************************************************
 * Azimuth enable/disable
 ********************************************************************/
void AZIMUTH_enable() {
 AZIMUTH_is_enabled = true;
}

void AZIMUTH_disable() {
  AZIMUTH_is_enabled = false;
}

bool AZIMUTH_enabled() {
  return AZIMUTH_enabled;
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
 * Azimuth main loop
 ********************************************************************/
void AZIMUTH_update() {
  if (AZIMUTH_enabled()) {
    int wheel_position = STEERING_WHEEL_get_position();

    /* Scale */
    float azimuth_position = scalef(
        (float)wheel_position, 
        0.0, 
        4096.0, 
        azimuth_data[JSON_AZIMUTH_LEFT].as<float>(), 
        azimuth_data[JSON_AZIMUTH_RIGHT].as<float>()
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