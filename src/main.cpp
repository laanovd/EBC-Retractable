
#include <Arduino.h>

#include "CLI.h"
#include "WiFiCom.h"
#include "WebServer.h"
#include "Config.h"
#include "Debug.h"
#include "MCPCom.h"
#include "Storage.h"
#include "Controller.h"
#include "CLI.h"
#include "GPIO.h"
#include "DMC.h"
#include "Azimuth.h"
#include "lift.h"
#include "Maintenance.h"

/*******************************************************************
 * CLI: Show all cli commands and descriptions
 *******************************************************************/
static void clicb_help(cmd *c) {
  (void)c;

  CLI_println(F("--- Help commands ---"));
  CLI_println(F("[?] ~ Help information."));
  CLI_println(F("[!] ~ System information."));
  CLI_println(F("[web] ~ Web-server information."));
  CLI_println(F("[wifi] ~ WiFi information."));
  CLI_println(F("[storage] ~ List settings."));
  CLI_println(F("[restart] ~ Restart the system."));
  CLI_println(F("[factory...] ~ Factory reset settings (yes)"));
  CLI_println(F("[dmc] ~ DMC information."));
  CLI_println(F("[lift] ~ Retractable information (timeout n)."));
  CLI_println(F("[azimuth] ~ Azimuth information (left <n>, right <n>, delay <n>, move)."));
}

/*******************************************************************
 * CLI: Show all cli commands and descriptions
 *******************************************************************/
static void clicb_system(cmd *c) {
  (void)c;
  String text;

  text = "--- System ---";

  text.concat("\r\nProgram title: ");
  text.concat(ProgramTitle);

  text.concat("\r\nProgram name: ");
  text.concat(ProgramName);
  text.concat(", version: ");
  text.concat(ProgramVersion);

  text.concat("\r\nWiFi ssid: ");
  text.concat(WiFi_ssid());
  text.concat(", MAC: ");
  text.concat(WiFi_mac());

  text.concat("\r\nHardware id: ");
  text.concat(ChipIds());
  text.concat(", PCB version: ");
  text.concat(PCBVersion);

  text.concat("\r\nFlash size(MB): ");
  text.concat(ESP.getFlashChipSize() / (1024 * 1024));
  text.concat(", Free memory(kB): ");
  text.concat((ESP.getFreeHeap() / 1024));

  text.concat("\r\n");
  CLI_println(text);
}

/********************************************************************
 * Command Line handler(s)
 *********************************************************************/
static void MAIN_CLI_handlers(void) {
  cli.addCommand("?,help", clicb_help);
  cli.addCommand("!,system", clicb_system);
}

/*******************************************************************
 * loop
 *******************************************************************/
void loop()
{
  // Do main task things...
}


/*******************************************************************
 * MAIN setup
 *******************************************************************/
static void MAIN_setup(void) {
  STORAGE_setup();
  CLI_setup();
  WiFi_setup();
  WEBSERVER_setup();
  // GPIO_setup();
  AZIMUTH_setup();
  LIFT_setup();
  DMC_setup();
  // CONTROLLER_setup();
  MAINTENANCE_setup();

  Serial.println(F("Main setup completed."));
}

/*******************************************************************
 * MAIN start
 *******************************************************************/
static void MAIN_start(void) {
  // GPIO_start();
  AZIMUTH_start();
  LIFT_start();
  DMC_start();
  // CONTROLLER_start();
  MAINTENANCE_start();

  MAIN_CLI_handlers();

  Serial.println(F("Main setup completed."));
}

/*******************************************************************
 * setup
 *******************************************************************/
void setup()
{
  Serial.begin(115200);
  sleep(2); // Startup delay for terminal
  
  MAIN_setup();

  vTaskDelay(1000 / portTICK_PERIOD_MS); // Wait one second for LED's to finish

  MAIN_start();
}
