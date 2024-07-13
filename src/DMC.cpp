/*******************************************************************
 * DMC.cpp
 *
 * EBC Dynamic Motor Control
 *
 *******************************************************************/
#include "DMC.h"  

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WebSerial.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

#include "Config.h"
#include "Storage.h"
#include "CLI.h"
#include "GPIO.h"
#include "EBC_IOLib.h" 

/*******************************************************************
 * Definitions
 *******************************************************************/
#define DEBUG_DMC

/*******************************************************************
 * Storage keys and defaults
 *******************************************************************/

/*******************************************************************
 * Globals
 *******************************************************************/
static JsonDocument DMC_data;

static int dmc_enable_timer = -1;
static bool DMC_is_enabled = false;

/********************************************************************
 * Create initial JSON data
 *******************************************************************/
static JsonDocument DMC_json(void) {

  DMC_data[JSON_DMC_ENABLE] = DMC_enabled();

  return DMC_data;
}

/********************************************************************
 * Create string
 *******************************************************************/
static String DMC_info_str(void) {
  JsonDocument doc = DMC_json();  // Update

  String text = "--- DMC ---";

  text.concat("\r\nEnabled: ");
  text.concat(doc[JSON_DMC_ENABLE].as<boolean>());

  text.concat("\r\n");
  return text;
}

/*******************************************************************
 * DMC Enable
 *******************************************************************/
void DMC_enable(void) {
  PCF8574_write(PCF8574_address, DMC_ENABLE_PIN, IO_ON);
#ifdef DEBUG_DMC
  Serial.println("DMC enable");
#endif
}

void DMC_disable(void) {
  PCF8574_write(PCF8574_address, DMC_ENABLE_PIN, IO_OFF);

#ifdef DEBUG_DMC
  Serial.println("DMC disable");
#endif
}

bool DMC_enabled(void) {
  return PCF8574_read(PCF8574_address, DMC_ENABLE_PIN) == IO_ON;
}

/********************************************************************
 * REST API
 *********************************************************************/
static void DMC_rest_read(AsyncWebServerRequest *request) {
  String str;
  serializeJson(DMC_json(), str);
  request->send(200, "application/json", str.c_str());
}

static rest_api_t DMC_api_handlers = {
    /* uri */ "/api/v1/dmc",
    /* comment */ "DMC module",
    /* instances */ 1,
    /* fn_create */ nullptr,
    /* fn_read */ DMC_rest_read,
    /* fn_update */ nullptr,
    /* fn_delete */ nullptr,
};

/********************************************************************
 * CLI handler
 *******************************************************************/
static void clicb_handler(cmd *c)
{
  Command cmd(c);
  Argument arg = cmd.getArg(0);
  String strArg = arg.getValue();

  /* List settings */
  if (strArg.isEmpty())
  {
    CLI_println(DMC_info_str());
    return;
  }

  CLI_println("Invalid command: DMC.");
}

/********************************************************************
 * Setup Command Line Handler (CLI)
 *******************************************************************/
static void DMC_setup_cli(void) {
  cli.addCommand("dmc", clicb_handler);
}

/*******************************************************************
 * GPIO setup
 *******************************************************************/
static void DMC_setup_gpio(void) {
  pinMode(DMC_ENABLE_PIN, OUTPUT);
}

/*******************************************************************
  Setup variables
 *******************************************************************/
static void DMC_setup_variables(void) {
  // Add persistent data
}

/*******************************************************************
 * DMC general
 *******************************************************************/
void DMC_setup(void) {
  DMC_setup_variables();
  DMC_setup_gpio();
  DMC_disable();

  Serial.println(F("DMC setup completed..."));
}

void DMC_start(void) {
  DMC_setup_cli();
  setup_uri(&DMC_api_handlers);

  Serial.println(F("DMC started..."));
}
