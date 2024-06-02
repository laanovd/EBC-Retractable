#include <Arduino.h>
#include "Storage.h"
#include "GPIO.cpp"

#define DMC_ENABLE_DELAY 3000

int dmc_enable_timer = -1;
bool DMC_is_enabled = false;

void DMC_enable()
{
    DMC_is_enabled = true;
    dmc_enable_timer = millis();
}

void DMC_disable()
{
    DMC_is_enabled = false;
    DMC_set_low();
}

void DMC_update()
{
    if (dmc_enable_timer != -1 && millis() - dmc_enable_timer > DMC_ENABLE_DELAY)
    {
        dmc_enable_timer = -1;
        DMC_set_high();
    }
}