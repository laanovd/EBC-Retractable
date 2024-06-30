
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
#include "Maintenance.h"


/********************************************************************
 * CLI: Show all cli commands and descriptions
 ********************************************************************/
static void clicb_help(cmd *c) {
  (void)c;

  CLI_println(F("--- Help commands ---"));
  CLI_println(F("[?] ~ Help information."));
  CLI_println(F("[!] ~ System information."));
  CLI_println(F("[web] ~ Web-server information."));
  CLI_println(F("[wifi] ~ WiFi information."));
  CLI_println(F("[mcp] ~ MCP information."));
  CLI_println(F("[storage] ~ List settings."));
  CLI_println(F("[restart] ~ Restart the system."));
  CLI_println(F("[factory...] ~ Factory reset settings (yes)"));
}

/********************************************************************
 * CLI: Show all cli commands and descriptions
 ********************************************************************/
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
 *  Initialize the command line handlers
 *
 ********************************************************************/
static void MAIN_handlers(void) {
  cli.addCommand("?,help", clicb_help);
  cli.addCommand("!,system", clicb_system);
}

/*******************************************************************
 *  Setup tasks
 *******************************************************************/
void LED_main_task(void *parameter)
{
  (void)parameter;
  while (true)
  {
    LED_UP_update();
    LED_DOWN_update();

    BUTTON_update();

    DMC_update();

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void AZIMUTH_main_task(void *parameter)
{
  (void)parameter;
  while (true)
  {
    AZIMUTH_update();

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

/********************************************************************
 * setup
 ********************************************************************/
static void MAIN_setup_tasks()
{
  xTaskCreate(LED_main_task, "LED monitor task", 1024, NULL, 10, NULL);
  xTaskCreate(AZIMUTH_main_task, "Azimuth debug task", 2048, NULL, 15, NULL);
}

/********************************************************************
 * setup
 ********************************************************************/
void setup()
{
  Serial.begin(115200);

  STORAGE_setup();
  CLI_setup();
  WiFi_setup();
  WEBSERVER_setup();
  GPIO_setup();
  AZIMUTH_setup();
  CONTROLLER_setup();
  MAINTENANCE_setup();

  /* Main program */
  MAIN_handlers();
  MAIN_setup_tasks();
  MAINTENANCE_start();

  Serial.println(F("Main setup completed."));
}

/********************************************************************
 * loop
 ********************************************************************/
void loop()
{
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  LED_HEARTBEAT_update();
  LED_ERROR_update();
}