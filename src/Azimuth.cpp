#include "Azimuth.h"

#include <Arduino.h>
#include <ArduinoJson.h>

#include "CLI.h"
#include "Storage.h"
#include "GPIO.cpp"

/********************************************************************
 * Setup variables
 ********************************************************************/
static JsonDocument azimuth_data;

static void AZIMUTH_init_int(const char *key, int default_value)
{
    int integer;
    if (STORAGE_get_int(key, integer))
    {
        integer = default_value;
        STORAGE_set_int(key, integer);
    }
    azimuth_data[key] = integer;
}

static void AZIMUTH_setup_variables(void)
{
    AZIMUTH_init_int(JSON_AZIMUTH_LEFT, JSON_AZIMUTH_LEFT_DEFAULT);
    AZIMUTH_init_int(JSON_AZIMUTH_RIGHT, JSON_AZIMUTH_RIGHT_DEFAULT);
}

/********************************************************************
 * Azimuth main loop
 ********************************************************************/
static bool AZIMUTH_ENABLED = false;
void AZIMUTH_update()
{
    if (AZIMUTH_ENABLED)
    {
        float wheel_position = WHEEL_get_position();
        float azimuth_position = wheel_position * (float(azimuth_data[JSON_AZIMUTH_RIGHT]) - float(azimuth_data[JSON_AZIMUTH_LEFT])) + azimuth_data[JSON_AZIMUTH_LEFT];
        AZIMUTH_set_position(azimuth_position / 9.9);
    }
}

/********************************************************************
 * Azimuth enable/disable
 ********************************************************************/
void AZIMUTH_enable()
{
    AZIMUTH_ENABLED = true;
    ANALOG_OUT_enable();
}

void AZIMUTH_disable()
{
    AZIMUTH_ENABLED = false;
    ANALOG_OUT_enable();
}

/********************************************************************
 * Azimuth setup
 ********************************************************************/
void AZIMUTH_setup()
{
    AZIMUTH_setup_variables();
}