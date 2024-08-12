/*******************************************************************
 * Azimuth.cpp
 *
 * EBC azimuth control
 *
 *******************************************************************/
#include "SteeringWheel.h"

#include <Arduino.h>
#include <ArduinoJson.h>

#include "Azimuth.h"
#include "CLI.h"
#include "Config.h"
#include "EBC_IOLib.h"
#include "GPIO.h"
#include "Maintenance.h"
#include "Storage.h"

/*******************************************************************
 * Definitions
 *******************************************************************/
#define DEBUG_AZIMUTH

#define STEERWHEEL_CALIBRATION_SIMULATE

#undef ENABLE_LEFT_OUTPUT  // No left output use yet

/*******************************************************************
 * Storage keys and defaults
 *******************************************************************/
#define JSON_STEERWHEEL_LEFT_DEFAULT 0.0
#define JSON_STEERWHEEL_RIGHT_DEFAULT 5.0
#define DELAY_TO_MIDDLE_DEFAULT 5

/*******************************************************************
 * Global variables
 *******************************************************************/
static JsonDocument STEERWHEEL_data;

/********************************************************************
 * Create initial JSON data
 *******************************************************************/
static JsonDocument STEERINGWHEEL_json(void) {
  STEERWHEEL_data[JSON_STEERWHEEL_LEFT] = STEERWHEEL_get_left();
  STEERWHEEL_data[JSON_STEERWHEEL_RIGHT] = STEERWHEEL_get_right();
  STEERWHEEL_data[JSON_STEERWHEEL_MIDDLE] = STEERWHEEL_get_middle();

  STEERWHEEL_data[JSON_STEERWHEEL_DEADBAND] = STEERWHEEL_get_deadband();

  return STEERWHEEL_data;
}

/********************************************************************
 * Create azimuth string
 *******************************************************************/
String STEERINGWHEEL_info(void) {
  static JsonDocument doc = STEERINGWHEEL_json();

  String text = "--- Steering wheel ---";

  text.concat("\r\nSteering wheel:  left: ");
  text.concat(doc[JSON_STEERWHEEL_LEFT].as<int>());

  text.concat(", middle: ");
  text.concat(doc[JSON_STEERWHEEL_MIDDLE].as<int>());

  text.concat(", right: ");
  text.concat(doc[JSON_STEERWHEEL_RIGHT].as<int>());

  text.concat("\r\nSteering wheel actual: ");
  text.concat(doc[JSON_STEERWHEEL_ACTUAL].as<int>());

  text.concat("\r\nMiddle deadband: ");
  text.concat(doc[JSON_STEERWHEEL_DEADBAND].as<int>());

  text.concat("\r\n");
  return text;
}

/*******************************************************************
 * Steering wheel read analog input
 *******************************************************************/
#define MAX_AVERAGE 10
static void STEERWHEEL_update(void) {
  static int ndx = 0, value;
  static int array[MAX_AVERAGE] = {0};
  static long sum = 0;

  ndx = (ndx + 1) % MAX_AVERAGE;

  sum -= array[ndx];
  array[ndx] = analogRead(STEER_WHEEL_ANALOG_CHANNEL);
  sum += array[ndx];

  value = constrain(sum / MAX_AVERAGE, ADC_MIN, ADC_MAX);
  STEERWHEEL_data[JSON_STEERWHEEL_ACTUAL] = value;
}

/********************************************************************
 * @brief Get the actual steering wheel position.
 *
 * This function reads the current steering wheel position and returns it.
 *
 * @return The actual steering wheel position.
 *******************************************************************/
int STEERWHEEL_get_actual(void) {
  return STEERWHEEL_data[JSON_STEERWHEEL_ACTUAL].as<int>();
}

/********************************************************************
 * Calculates the linear value of the steering wheel position.
 *
 * @return The linear value of the steering wheel position.
 *******************************************************************/
int STEERWHEEL_get_linear(void) {
  long value = STEERWHEEL_get_actual();
  long left = STEERWHEEL_get_left();
  long middle = STEERWHEEL_get_middle();
  long right = STEERWHEEL_get_right();

  if (((left < middle) && (middle < right)) ||
      ((left > middle) && (middle > right))) {
    if (value < middle) {
      value = (int)map(value, left, middle, LINEAR_MIN, LINEAR_MIDDLE - 1);
    } else {
      value = (int)map(value, middle, right, LINEAR_MIDDLE, LINEAR_MAX);
    }
  }

  value = constrain(value, LINEAR_MIN, LINEAR_MAX);
  return value;
}

/*******************************************************************
 * Steering wheel get calibration values
 *******************************************************************/
int STEERWHEEL_get_left(void) {
  return STEERWHEEL_data[JSON_STEERWHEEL_LEFT].as<int>();
}

void STEERWHEEL_set_left(int value) {
  if ((value >= ADC_MIN) && (value <= ADC_MAX)) {
    STEERWHEEL_data[JSON_STEERWHEEL_LEFT] = value;
  }
}

void STEERWHEEL_store_left(int value) {
  if ((value >= ADC_MIN) && (value <= ADC_MAX)) {
    STORAGE_set_int(JSON_STEERWHEEL_LEFT, value);
  }
}

int STEERWHEEL_get_right(void) {
  return STEERWHEEL_data[JSON_STEERWHEEL_RIGHT].as<int>();
}

void STEERWHEEL_set_right(int value) {
  if ((value >= ADC_MIN) && (value <= ADC_MAX)) {
    STEERWHEEL_data[JSON_STEERWHEEL_RIGHT] = value;
  }
}

void STEERWHEEL_store_right(int value) {
  if ((value >= ADC_MIN) && (value <= ADC_MAX)) {
    STORAGE_set_int(JSON_STEERWHEEL_RIGHT, value);
  }
}

int STEERWHEEL_get_middle(void) {
  return STEERWHEEL_data[JSON_STEERWHEEL_MIDDLE].as<int>();
}

void STEERWHEEL_set_middle(int value) {
  if ((value >= ADC_MIN) && (value <= ADC_MAX)) {
    STEERWHEEL_data[JSON_STEERWHEEL_MIDDLE] = value;
  }
}

void STEERWHEEL_store_middle(int value) {
  if ((value >= ADC_MIN) && (value <= ADC_MAX)) {
    STORAGE_set_int(JSON_STEERWHEEL_MIDDLE, value);
  }
}

int STEERWHEEL_get_deadband(void) {
  int value;
  STORAGE_get_int(JSON_STEERWHEEL_DEADBAND, value);
  return value;
}

void STEERWHEEL_set_deadband(int value) {
  if ((value >= ADC_MIN) && (value <= ADC_MAX)) {
    STEERWHEEL_data[JSON_STEERWHEEL_DEADBAND] = value;
    STORAGE_set_int(JSON_STEERWHEEL_DEADBAND, value);
  }
}

/********************************************************************
 * @brief Saves the calibration values for the steering wheel.
 *
 * This function retrieves the calibration values from the `doc` JSON object
 * and stores them using the `STORAGE_get_int` function.
 *******************************************************************/
void STEERWHEEL_calibration_save(void) {
  int value = STEERWHEEL_get_left();
  STEERWHEEL_store_left(value);

  value = STEERWHEEL_get_right();
  STEERWHEEL_store_right(value);

  value = STEERWHEEL_get_middle();
  STEERWHEEL_store_middle(value);
}

/********************************************************************
 * @brief Restores the calibration values for the steering wheel.
 *
 * This function retrieves the calibration values for the left, right, and middle positions
 * of the steering wheel from the storage and sets them using the corresponding setter functions.
 *
 * @note This function assumes that the calibration values have been previously stored in the storage.
 *
 * @see STEERWHEEL_set_left
 * @see STEERWHEEL_set_right
 * @see STEERWHEEL_set_middle
 *******************************************************************/
void STEERWHEEL_calibration_restore(void) {
  int value;

  STORAGE_get_int(JSON_STEERWHEEL_LEFT, value);
  STEERWHEEL_set_left(value);

  STORAGE_get_int(JSON_STEERWHEEL_RIGHT, value);
  STEERWHEEL_set_right(value);

  STORAGE_get_int(JSON_STEERWHEEL_MIDDLE, value);
  STEERWHEEL_set_middle(value);
}

/********************************************************************
 * CLI handler
 *******************************************************************/
static void clicb_handler(cmd *c) {
  Command cmd(c);
  Argument arg = cmd.getArg(0);
  String strArg = arg.getValue();

  /* List settings */
  if (strArg.isEmpty()) {
    CLI_println(STEERINGWHEEL_info());
    return;
  }

  if (strArg.equalsIgnoreCase("left")) {
    int val = cmd.getArg(1).getValue().toInt();
    if ((val < ADC_MIN) || (val > ADC_MAX)) {
      CLI_println("Illegal value, range: 0 ... 4095 counts");
      return;
    }
    STORAGE_set_int(JSON_STEERWHEEL_LEFT, val);
    CLI_println("Steering wheel left limit has been set to " + String(val) + " counts.");
  }

  if (strArg.equalsIgnoreCase("right")) {
    int val = cmd.getArg(1).getValue().toInt();
    if ((val < ADC_MIN) || (val > ADC_MAX)) {
      CLI_println("Illegal value, range: 0 ... 4095 counts");
      return;
    }
    STORAGE_set_int(JSON_STEERWHEEL_RIGHT, val);
    CLI_println("Steering wheel right limit has been set to " + String(val) + " counts.");
  }

  if (strArg.equalsIgnoreCase("middle")) {
    int val = cmd.getArg(1).getValue().toInt();
    if ((val < ADC_MIN) || (val > ADC_MAX)) {
      CLI_println("Illegal value, range: 0 ... 4095 counts");
      return;
    }
    STORAGE_set_int(JSON_STEERWHEEL_MIDDLE, val);
    CLI_println("Steering wheel middle has been set to " + String(val) + " counts.");
  }

  if (strArg.equalsIgnoreCase("deadband")) {
    int val = cmd.getArg(1).getValue().toInt();
    if ((val < ADC_MIN) || (val > ADC_MAX)) {
      CLI_println("Illegal value, range: 0 ... 4095 counts");
      return;
    }
    STORAGE_set_int(JSON_STEERWHEEL_DEADBAND, val);
    CLI_println("Steering wheel middle has been set to " + String(val) + " counts.");
  }

  CLI_println("Invalid command: STEER (left <n>, right <n>, middle <n>, deadband <n>).");
}

static void cli_setup(void) {
  cli.addBoundlessCmd("steer", clicb_handler);
}

/********************************************************************
 * Main task
 *********************************************************************/
static void STEERINGWHEEL_main_task(void *parameter) {
  (void)parameter;

  while (true) {
    STEERWHEEL_update();
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

/********************************************************************
 *  Initialize tasks
 *********************************************************************/
static void STEERINGWHEEL_setup_tasks(void) {
  xTaskCreate(STEERINGWHEEL_main_task, "Steer wheel main", 2048, NULL, 10, NULL);
}

/*******************************************************************
 * GPIO setup
 *******************************************************************/
static void STEERINGWHEEL_setup_gpio(void) {
  analogReadResolution(12);
}

/*******************************************************************
 * Setup variables
 *******************************************************************/
static void STEERINGWHEEL_setup_variables(void) {
  int value;

  if (STORAGE_get_int(JSON_STEERWHEEL_LEFT, value)) {
    value = ADC_MAX;
    STORAGE_set_int(JSON_STEERWHEEL_LEFT, value);
  }
  STEERWHEEL_data[JSON_STEERWHEEL_LEFT] = value;

  if (STORAGE_get_int(JSON_STEERWHEEL_MIDDLE, value)) {
    value = (ADC_MIN + ADC_MAX) / 2;
    STORAGE_set_int(JSON_STEERWHEEL_MIDDLE, value);
  }
  STEERWHEEL_data[JSON_STEERWHEEL_MIDDLE] = value;

  if (STORAGE_get_int(JSON_STEERWHEEL_RIGHT, value)) {
    value = ADC_MIN;
    STORAGE_set_int(JSON_STEERWHEEL_RIGHT, value);
  }
  STEERWHEEL_data[JSON_STEERWHEEL_RIGHT] = value;

  if (STORAGE_get_int(JSON_STEERWHEEL_DEADBAND, value)) {
    value = 2;
    STORAGE_set_int(JSON_STEERWHEEL_DEADBAND, value);
  }
  STEERWHEEL_data[JSON_STEERWHEEL_DEADBAND] = value;
}

/********************************************************************
 * Stops the steering wheel and aborts any ongoing calibration process.
 *******************************************************************/
void STEERINGWHEEL_stop() {
  Serial.println(F("Steering wheel stopped."));
}

/********************************************************************
 * @brief Sets up the steering wheel.
 *
 * This function initializes the necessary components and
 * configurations for the steering wheel. It should be called once
 * during the setup phase of the program.
 *******************************************************************/
void STEERINGWHEEL_setup() {
  STEERINGWHEEL_setup_variables();
  STEERINGWHEEL_setup_gpio();

  Serial.println(F("Steering wheel setup completed..."));
}

/********************************************************************
 * @brief Starts the steering wheel functionality.
 *
 * This function initializes and starts the steering wheel
 * functionality. It should be called before using any other
 * steering wheel related functions.
 *******************************************************************/
void STEERINGWHEEL_start() {
  STEERINGWHEEL_setup_tasks();
  cli_setup();

  Serial.println(F("Steering wheel started..."));
}
