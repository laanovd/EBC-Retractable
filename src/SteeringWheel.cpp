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

#undef STEERWHEEL_CALIBRATION_SIMULATE

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

static TaskHandle_t steerwheel_calibrate_task = NULL;
static bool calibration_abort = false;

/********************************************************************
 * Create initial JSON data
 *******************************************************************/
static JsonDocument STEERINGWHEEL_json(void) {
  STEERWHEEL_data[JSON_STEERWHEEL_LEFT] = STEERWHEEL_get_left();
  STEERWHEEL_data[JSON_STEERWHEEL_RIGHT] = STEERWHEEL_get_right();
  STEERWHEEL_data[JSON_STEERWHEEL_MIDDLE] = STEERWHEEL_get_middle();

  STEERWHEEL_data[JSON_STEERWHEEL_ACTUAL] = STEERWHEEL_get_actual();

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
  text.concat(doc[JSON_STEERWHEEL_LEFT].as<float>());

  text.concat(", right: ");
  text.concat(doc[JSON_STEERWHEEL_RIGHT].as<float>());

  text.concat(", middle: ");
  text.concat(doc[JSON_STEERWHEEL_MIDDLE].as<float>());

  text.concat("\r\nSteering wheel actual: ");
  text.concat(doc[JSON_STEERWHEEL_MIDDLE].as<float>());

  text.concat("\r\n");
  return text;
}

/*******************************************************************
 * Steering wheel read analog input
 *******************************************************************/
#define MAX_AVERAGE 10
static int STEERWHEEL_read(void) {
  static int ndx = 0, value;
  static int array[MAX_AVERAGE] = {0};
  static long sum = 0;

  int left = STEERWHEEL_get_left();
  int right = STEERWHEEL_get_right();

  ndx = (ndx + 1) % MAX_AVERAGE;

  sum -= array[ndx];
  array[ndx] = analogRead(STEER_WHEEL_ANALOG_CHANNEL);
  sum += array[ndx];

  // Serial.printf("\r\nSteering wheel: %d", array[ndx]);

  value = (int)sum / MAX_AVERAGE;

  if (left != right) {
    value = map(value, left, right, ADC_MIN, ADC_MAX);
  }
  value = max(min(value, ADC_MAX), ADC_MIN);

  STEERWHEEL_data[JSON_STEERWHEEL_ACTUAL] = value;
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
    STORAGE_set_int(JSON_STEERWHEEL_LEFT, value);
  }
}

int STEERWHEEL_get_right(void) {
  return STEERWHEEL_data[JSON_STEERWHEEL_RIGHT].as<int>();
}

void STEERWHEEL_set_right(int value) {
  if ((value >= ADC_MIN) && (value <= ADC_MAX)) {
    STEERWHEEL_data[JSON_STEERWHEEL_RIGHT] = value;
    STORAGE_set_int(JSON_STEERWHEEL_RIGHT, value);
  }
}

int STEERWHEEL_get_middle(void) {
  return STEERWHEEL_data[JSON_STEERWHEEL_MIDDLE].as<int>();
}

void STEERWHEEL_set_middle(int value) {
  if ((value >= ADC_MIN) && (value <= ADC_MAX)) {
    STEERWHEEL_data[JSON_STEERWHEEL_MIDDLE] = value;
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

int STEERWHEEL_get_actual(void) {
  if (!steerwheel_calibrate_task) 
    return STEERWHEEL_read();

  return STEERWHEEL_data[JSON_STEERWHEEL_ACTUAL].as<int>();
}

/********************************************************************
 * Steerwheel calibration
 *******************************************************************/
void STEERWHEEL_calibrate(void *parameter) {
  (void)parameter;
  int low = ADC_MAX, high = ADC_MIN, middle = 0;
  
  while (steerwheel_calibrate_task) {
#ifdef STEERWHEEL_CALIBRATION_SIMULATE
    int value = rand() % (4095 + 1 - 400) + 400;
#else
    int value = STEERWHEEL_read();
#endif

    STEERWHEEL_data[JSON_STEERWHEEL_ACTUAL] = value;

    low = max(min(low, value), ADC_MIN);
    STEERWHEEL_data[JSON_STEERWHEEL_LEFT] = low;

    high = min(max(high, value), ADC_MAX);
    STEERWHEEL_data[JSON_STEERWHEEL_RIGHT] = high;
    
    middle = value;
    STEERWHEEL_data[JSON_STEERWHEEL_MIDDLE] = middle;

    // Serial.printf("\r\nSteering wheel calibration: %04d, %04d, %04d.", low, high, middle);

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
  
  /* Store values */
  if (!calibration_abort) {
    STEERWHEEL_set_left(high);
    STEERWHEEL_set_right(low);	
    STEERWHEEL_set_middle(middle);
  }

  /* End task */
  vTaskDelete( NULL );
}

/********************************************************************
 * Starts the calibration process for the steering wheel.
 * 
 * If the calibration task is not already running, it creates 
 * a new task to perform the calibration.
 *******************************************************************/
void STEERWHEEL_calibration_start(void) {
  if (!steerwheel_calibrate_task) {
    calibration_abort = false;
    xTaskCreate(STEERWHEEL_calibrate, "Steerwheel calibration", 4096, NULL, 5, &steerwheel_calibrate_task);
    Serial.println(F("Steering wheel calibration started..."));
  }
}

/********************************************************************
 * @brief Ends the calibration process for the steering wheel.
 *
 * This function sets the `calibration_abort` flag to false and 
 * clears the `steerwheel_calibrate_task` pointer. It is called to 
 * indicate the completion or termination of the calibration process.
 *******************************************************************/
void STEERWHEEL_calibration_end(void) {
  calibration_abort = false;
  steerwheel_calibrate_task = NULL;
  Serial.println(F("Steering wheel calibration stoped..."));
}

/********************************************************************
 * Aborts the calibration process for the steering wheel.
 *******************************************************************/
void STEERWHEEL_calibration_abort(void) {
  calibration_abort = true;
  if (steerwheel_calibrate_task) {
    steerwheel_calibrate_task = NULL;
    Serial.println(F("Steering wheel calibration aborted..."));
  }
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
    float val = cmd.getArg(1).getValue().toInt();
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
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

/********************************************************************
 *  Initialize tasks
 *********************************************************************/
static void STEERINGWHEEL_setup_tasks(void) {
  // xTaskCreate(STEERINGWHEEL_main_task, "Steer wheel main", 2048, NULL, 10, NULL);
}

/*******************************************************************
 * GPIO setup
 *******************************************************************/
static void STEERINGWHEEL_setup_gpio(void) {
  // analogReadResolution(12);
  pinMode(STEER_WHEEL_ANALOG_CHANNEL, INPUT);
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

  if (STORAGE_get_int(JSON_STEERWHEEL_RIGHT, value)) {
    value = ADC_MIN;
    STORAGE_set_int(JSON_STEERWHEEL_RIGHT, value);
  }
  STEERWHEEL_data[JSON_STEERWHEEL_RIGHT] = value;

  if (STORAGE_get_int(JSON_STEERWHEEL_MIDDLE, value)) {
    value = 0;
    STORAGE_set_int(JSON_STEERWHEEL_MIDDLE, value);
  }
  STEERWHEEL_data[JSON_STEERWHEEL_MIDDLE] = value;

  if (STORAGE_get_int(JSON_STEERWHEEL_DEADBAND, value)) {
    value = 0;
    STORAGE_set_int(JSON_STEERWHEEL_DEADBAND, value);
  }
  STEERWHEEL_data[JSON_STEERWHEEL_DEADBAND] = value;
}

/********************************************************************
 * Stops the steering wheel and aborts any ongoing calibration process.
 *******************************************************************/
void STEERINGWHEEL_stop() {
  STEERWHEEL_calibration_abort();

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
