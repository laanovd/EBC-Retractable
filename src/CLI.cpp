/*******************************************************************
 * Commandline interface
 *
 *******************************************************************/
#include "cli.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SimpleCLI.h>
#include <WebSerial.h>

#include "Config.h"
#include "GPIO.h"
#include "Controller.h"
#include "Azimuth.h"
#include "Storage.h"
#include "WiFiCom.h"
#include "esp_chip_info.h"
/*******************************************************************
 * Global variables
 *******************************************************************/
SimpleCLI cli; /* Command Line Interface */

static cli_output_t cli_output = CLI_SERIAL;

/*******************************************************************
 * Output mode for commandline interface
 *******************************************************************/
void cli_set_output(cli_output_t value)
{
  cli_output = value;
}

/*******************************************************************
 * Calibration commandline handler
 *******************************************************************/
static bool calibrating = false;
void set_calibrating(bool value) { calibrating = value; }
bool is_calibrating() { return calibrating; }

/*******************************************************************
 * Bytes to String
 *******************************************************************/
static String bytes2string(const uint8_t *data, size_t len)
{
  String text = "";

  for (int i = 0; i < len; i++)
    text += char(data[i]);

  return text;
}

/*******************************************************************
 * Output ComandlIne interface data
 *******************************************************************/
void CLI_print(String txt)
{
  if (cli_output == CLI_WEBSERIAL)
  {
    WebSerial.print(txt);
    return;
  }

  // Default output channel (CLI_SERIAL)
  Serial.print(txt);
}

void CLI_println(String txt)
{
  if (cli_output == CLI_SERIAL)
    txt += "\n";
  txt += "\r";

  CLI_print(txt);
}

/*******************************************************************
 * System reboot
 *******************************************************************/
void system_restart(void)
{
  CLI_println("System wil restart in 3 sec...");
  CLI_println("");

  vTaskDelay(3000 / portTICK_PERIOD_MS);
  ESP.restart();
}

/********************************************************************
 * Create initial JSON data
 *******************************************************************/
static JsonDocument CLI_data(void)
{
  JsonDocument doc;

  doc["program-name"] = ProgramName;
  doc["program-version"] = ProgramVersion;
  doc["hardware-mode"] = get_model_info();
  doc["hardware-revision"] = get_silicon_revision();
  doc["cpu-speed-mhz"] = ESP.getCpuFreqMHz();
  doc["chip-id"] = ChipIds();
  doc["flash-size-mb"] = (ESP.getFlashChipSize() / (1024 * 1024));
  doc["heap-size-kb"] = (ESP.getHeapSize() / 1024);
  doc["heep-free-kb"] = (ESP.getFreeHeap() / 1024);
  doc["sketch-size-kb"] = (ESP.getSketchSize() / 1024);
  doc["esp32-sdk-version"] = ESP.getSdkVersion();

  return doc;
}

/*******************************************************************
 *  WebSerial Command line interface
 *
 *******************************************************************/
void CLI_webserial_task(uint8_t *data, size_t len)
{
  String input = bytes2string(data, len);

  cli_set_output(CLI_WEBSERIAL);

  // Parse the user input into the CLI
  cli.parse(input);

  if (cli.errored())
  {
    CommandError cmdError = cli.getError();
    CLI_println(String("ERROR: " + cmdError.toString() + "\r\n"));

    if (cmdError.hasCommand())
    {
      CLI_println(String("Did you mean: '" + cmdError.getCommand().toString() + "'?"));
    }
  }
}

/*******************************************************************
 *  Serial Command line interval
 *
 *******************************************************************/
static void CLI_serial_task(void *parameter)
{
  static String input = "";
  static uint8_t inChar = 0;
  (void)parameter;

  while (1)
  {
    vTaskDelay(200 / portTICK_PERIOD_MS);

    while (Serial.available())
    {
      inChar = Serial.read();
      switch (inChar)
      {
      case '\n':
        cli_set_output(CLI_SERIAL);
        Serial.println();

        // Parse the user input into the CLI
        cli.parse(input);

        if (cli.errored())
        {
          CommandError cmdError = cli.getError();
          CLI_println(String("ERROR: " + cmdError.toString() + "\r\n"));

          if (cmdError.hasCommand())
          {
            CLI_println(String("Did you mean: '" + cmdError.getCommand().toString() + "'?"));
          }
        }

        Serial.print("\n\rcli > ");
        input = "";
        break;

      case '\r': // Ignore
        break;

      default:
        input += char(inChar);
        Serial.print(char(inChar));
        break;
      }
    }
  }
}

/*******************************************************************
 * CLI: Reboot the system
 *******************************************************************/
static void clicb_reboot(cmd *c)
{
  (void)c;
  system_restart();
}

/*******************************************************************
 * Command line interface error callback
 *
 *******************************************************************/
void errorCallback(cmd_error *e)
{
  CommandError cmdError(e); // Create wrapper object

  CLI_print("\n\rERROR: " + cmdError.toString());

  if (cmdError.hasCommand())
  {
    CLI_print("\n\rDid you mean \"" + cmdError.getCommand().toString() + "\"?");
  }
}

/*******************************************************************
 *  Initialize the debug tasks
 *
 *******************************************************************/
static void CLI_setup_tasks(void)
{
  xTaskCreate(CLI_serial_task, "Serial commandline interface task", 4096, NULL, 15, NULL); // Needs large stack
}

/*******************************************************************
 *  Initialize the command line handlers
 *
 *******************************************************************/
static void CLI_handlers(void)
{
  static Command cli_cmd_help, cli_cmd_system_info, cli_cmd_reboot;

  cli.setOnError(errorCallback); // Set error Callback

  cli.addCommand("restart", clicb_reboot);
}

/*******************************************************************
 *  Initialize the command line interface
 *
 *******************************************************************/
void CLI_setup(void)
{
  CLI_setup_tasks();
  CLI_handlers();

  Serial.println(F("CLI handlers setup completed..."));
}