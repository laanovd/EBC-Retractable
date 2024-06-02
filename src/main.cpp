
#include <Arduino.h>

#include "CLI.h"
#include "Config.h"
#include "Debug.h"
#include "MCPCom.h"
#include "Storage.h"
#include "CLI.h"
#include "GPIO.cpp"

/********************************************************************
 *  Initialize the command line handlers
 ********************************************************************/
static void MAIN_handlers(void) {}

/*******************************************************************
 *  Setup tasks
 *******************************************************************/
#define LED_UPDATE_FREQUENCY 5

void MAIN_setup_tasks() {}

void LED_main_task(void* parameter)
{
  (void)parameter;
  while(true){
    LED_UP_update();
    LED_DOWN_update();
    LED_HEARTBEAT_update();
    vTaskDelay(1000 / LED_UPDATE_FREQUENCY);
  }
}

/********************************************************************
 * setup
 ********************************************************************/
void setup(){
  Serial.begin(115200);

  STORAGE_setup();
  CLI_setup();
  GPIO_setup();

  /* Main program */
  MAIN_handlers();
  MAIN_setup_tasks();

  Serial.println(F("Main setup completed."));
}


/********************************************************************
 * loop
 ********************************************************************/
void loop() {
  // ? Debounce buttons here?

}