/********************************************************************
 *    Debug.ino
 *
 *    Debugging tools
 *
 *********************************************************************/
#include <Arduino.h>
#include <ElegantOTAPro.h>
#include <WebSerial.h>
#include <esp_err.h>

#include "Config.h"
#include "Storage.h"
#include "WiFiCom.h"
#include "WebServer.h"
#include "CLI.h"

#include "Debug.h"

/********************************************************************
 * Constants
 *********************************************************************/
#define DEBUG_THROTTLE_MS 200   // Delay for displaying debug lines in WebSerial

#define MAX_DEBUG_LINE_SIZE 96  // Maximum debug line length
#define DEBUG_QUEUE_SIZE 30     // Maximum debug messages in queue
#define MAX_TRANSMIT_LENGTH 512 // Max. tx buffer WebSerial

#define MAX_COMMAND_HANLERS 25  // Maximum list with command handlers

/********************************************************************
 * Globals
 *********************************************************************/
// Debugging
static bool debug_frames_active = false;    // Show general info
static bool debug_data_active = false;      // Show data
static bool debug_bus_active = false;       // Show low-level bus communication
static bool debug_setup_completed = false;  // Setup of debug module done

static QueueHandle_t _debug_queue;
static int _debug_dropped = 0;

/********************************************************************
 * Debug logging unconditional
 *********************************************************************/
static void DEBUG_direct(String txt) {
  if (!debug_setup_completed)
    return;  // Drop message

  if (txt.length() > MAX_DEBUG_LINE_SIZE - 3) {
    txt = txt.substring(0, MAX_DEBUG_LINE_SIZE - 3);
    txt += "...";
  }

  if (xQueueSend(_debug_queue, txt.c_str(), 0) != pdPASS) {
    _debug_dropped += 1;
  }
}

/********************************************************************
 * Show information unconditional
 *********************************************************************/
void DEBUG_info(String info) {
  DEBUG_direct(info);
}

/********************************************************************
 * Debugging frames active
 *********************************************************************/
bool DEBUG_frames_active(void) {
  return debug_frames_active;
}

/********************************************************************
 * Show debug logging conditional...
 *********************************************************************/
void DEBUG_frames(String frame) {
  if (!DEBUG_frames_active())
    return;

  DEBUG_direct(frame);
}

/********************************************************************
 * Debugging data active
 *********************************************************************/
bool DEBUG_data_active(void) {
  return debug_data_active;
}

/********************************************************************
 * Show data conditional...
 *********************************************************************/
void DEBUG_data(String txt) {
  if (!DEBUG_data_active())
    return;

  DEBUG_direct(txt);
}

/********************************************************************
 * Convert CAN frame to string
 *********************************************************************/
static String CAN2String(const int length, const int id, const int rtr, const uint8_t buffer[]) {
  char msg[96];
  snprintf(msg, sizeof(msg), "Id=0x%04x, length=%d, [%02x %02x %02x %02x %02x %02x %02x %02x].",
           (int)id,
           (int)length,
           buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);
  return msg;
}

/********************************************************************
 * Show CAN commnication conditional...
 *********************************************************************/
void DEBUG_can(String label, const int length, const int id, const int rtr, const uint8_t buffer[]) {
  if (!debug_bus_active)
    return;

  String txt = CAN2String(length, id, rtr, buffer);
  DEBUG_direct(label + txt);
}

/********************************************************************
 * Show data conditional...
 *********************************************************************/
void DEBUG_bus(String txt) {
  if (!debug_bus_active)
    return;

  DEBUG_direct(txt);
}

/********************************************************************
 * CLI: Show low-level communication
 *********************************************************************/
static void clicb_toggle_bus(cmd *c) {
  (void)c;
  
  debug_bus_active = !debug_bus_active;

  char msg[32];
  snprintf(msg, sizeof(msg), "Show bus communication is %s.", debug_bus_active ? "ON" : "OFF");
  CLI_println(msg);
}

/********************************************************************
 * CLI: Show communication frames
 *********************************************************************/
static void clicb_toggle_frames(cmd *c) {
  (void)c;

  debug_frames_active = !debug_frames_active;

  char msg[32];
  snprintf(msg, sizeof(msg), "Show frames is %s.", debug_frames_active ? "ON" : "OFF");
  CLI_println(msg);
}

/********************************************************************
 * CLI: Show internal data
 *********************************************************************/
static void clicb_toggle_data(cmd *c) {
  (void)c;

  debug_data_active = !debug_data_active;

  char msg[32];
  snprintf(msg, sizeof(msg), "Show data is %s.", debug_data_active ? "ON" : "OFF");
  CLI_println(msg);
}

/********************************************************************
 *  DEBUG main task
 *   
 *********************************************************************/
static void DEBUG_main_task(void *parameter) {
  char msg[MAX_DEBUG_LINE_SIZE + 1];
  String txt;
  (void)parameter;

  vTaskDelay(1500 / portTICK_PERIOD_MS); // Startup delay

  while (1) {
    // Throttle debugging via WebSerial
    vTaskDelay(DEBUG_THROTTLE_MS / portTICK_PERIOD_MS);

    if (!uxQueueMessagesWaiting(_debug_queue))
      continue;  // To start of loop

    txt = "";
    while (txt.length() < MAX_TRANSMIT_LENGTH) {
      if (xQueueReceive(_debug_queue, msg, 0) != pdPASS)
        break;

      if (!txt.isEmpty())
        txt = txt + "\r\n";

      txt = txt + String(msg);
    }
  
    if (_debug_dropped) {
      txt = txt + "(#drop:" + String(_debug_dropped) + ")";
      _debug_dropped = 0;
    }

    CLI_println(txt.c_str());
  }
}

/********************************************************************
 *  Initialize tasks
 *   
 *********************************************************************/
static void DEBUG_setup_tasks(void) {
  xTaskCreate(DEBUG_main_task, "Debug system task", 4096, NULL, 15, NULL);
}

/********************************************************************
 *  Initialize the debug queue
 *   
 *********************************************************************/
static void DEBUG_setup_queue(void) {
  _debug_queue = xQueueCreate(DEBUG_QUEUE_SIZE, MAX_DEBUG_LINE_SIZE + 1);
}

/********************************************************************
 * Command Line handler(s)
 *********************************************************************/
static void CLI_handlers(void) {
  cli.addCommand("bus", clicb_toggle_bus);
  cli.addCommand("frames", clicb_toggle_frames);
  cli.addCommand("data", clicb_toggle_data);
}

/********************************************************************
 *  Setup debugging tools
 *
 *********************************************************************/
void DEBUG_setup(void) {
  DEBUG_setup_queue();
  DEBUG_setup_tasks();

  CLI_handlers();

  debug_setup_completed = true;  // Setup completed.
  Serial.println(F("Debug setup completed..."));
}