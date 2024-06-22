#include <Arduino.h>
#include "Storage.h"
#include "GPIO.h"

#define DMC_ENABLE_DELAY 3000

int dmc_enable_timer = -1;
bool DMC_is_enabled = false;

void DMC_enable()
{
    DMC_set_high();
    dmc_enable_timer = millis();
}

void DMC_disable()
{
    DMC_set_low();
    dmc_enable_timer = -1;
}

void DMC_update()
{
    if (DMC_enabled() && ((millis() - dmc_enable_timer) > DMC_ENABLE_DELAY))
    {
        dmc_enable_timer = -1;
        DMC_set_high();
    }
}