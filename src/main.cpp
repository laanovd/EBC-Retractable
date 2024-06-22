
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

/********************************************************************
 *  Initialize the command line handlers
 ********************************************************************/
static void MAIN_handlers(void) {}

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

  /* Main program */
  MAIN_handlers();
  MAIN_setup_tasks();

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