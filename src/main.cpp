
#include <Arduino.h>

#include "CLI.h"
#include "Config.h"
#include "Debug.h"
#include "MCPCom.h"
#include "Storage.h"
#include "TWAICom.h"
#include "WebServer.h"
#include "WiFiCom.h"


/********************************************************************
 *  Initialize the command line handlers
 *
 ********************************************************************/
static void MAIN_handlers(void) {}

/*******************************************************************
 *  Setup tasks
 *******************************************************************/
void MAIN_setup_tasks() {}

/********************************************************************
 * setup
 ********************************************************************/
void setup(){
  Serial.begin(115200);

  STORAGE_setup();

  /* Main program */
  MAIN_handlers();
  MAIN_setup_tasks();

  Serial.println(F("Main setup completed."));
}


/********************************************************************
 * loop
 ********************************************************************/
void loop() {

}