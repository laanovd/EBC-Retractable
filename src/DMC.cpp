#include <Arduino.h>

#include "Config.h"
#include "GPIO.h"
#include "Storage.h"

/*******************************************************************
 * Definitions
 *******************************************************************/
#define DEBUG_DMC

/*******************************************************************
 * Globals
 *******************************************************************/
static int dmc_enable_timer = -1;
static bool DMC_is_enabled = false;

/*******************************************************************
 * DMC Enable
 *******************************************************************/
void DMC_enable(void) {
  PCF8574_write(DMC_ENABLE_PIN, IO_ON);

#ifdef DEBUG_DMC
  Serial.println("DMC enabled");
#endif
}

void DMC_disable(void) {
  PCF8574_write(DMC_ENABLE_PIN, IO_OFF);

#ifdef DEBUG_DMC
  Serial.println("DMC disabled");
#endif
}

bool DMC_enabled(void) {
  return PCF8574_read(DMC_ENABLE_PIN) == IO_ON;
}

/*******************************************************************
 * GPIO setup
 *******************************************************************/
static void DMC_setup_gpio(void) {
  pinMode(DMC_ENABLE_PIN, OUTPUT);
}

/*******************************************************************
 * DMC general
 *******************************************************************/
void DMC_setup(void) {
  DMC_setup_gpio();
  DMC_disable();
}

void DMC_start(void) {
}
