/*******************************************************************
 * Azimuth.cpp
 *
 * EBC azimuth control
 *
 *******************************************************************/
#include "Azimuth.h"

#include <Arduino.h>
#include <ArduinoJson.h>

#include "CLI.h"
#include "Config.h"
#include "EBC_IOLib.h"
#include "EBC_Utils.h"
#include "GPIO.h"
#include "Maintenance.h"
#include "Storage.h"

/*******************************************************************
 * Definitions
 *******************************************************************/
#define DEBUG_AZIMUTH

#undef ENABLE_LEFT_OUTPUT  // No left output use yet

/*******************************************************************
 * Storage keys and defaults
 *******************************************************************/
#define JSON_STEERWHEEL_LEFT_DEFAULT 0.0
#define JSON_STEERWHEEL_RIGHT_DEFAULT 5.0

#define DELAY_TO_MIDDLE_DEFAULT 120

/*******************************************************************
 * Type definitions
 *******************************************************************/
enum AzmuthCalibrateStates {
  ACalibStart,
  ACalibSeekHomeBegin,
  ACalibSeekHomeEnd,
  ACalibFinished,
  ACalibAbort
};

/*******************************************************************
 * Global variables
 *******************************************************************/
static JsonDocument AZIMUTH_data;

/********************************************************************
 * Create initial JSON data
 *******************************************************************/
static JsonDocument AZIMUTH_json(void) {
  AZIMUTH_data[JSON_AZIMUTH_ENABLED] = AZIMUTH_enabled();
  AZIMUTH_data[JSON_AZIMUTH_HOME] = AZIMUTH_home();

  AZIMUTH_data[JSON_AZIMUTH_OUTPUT_ENABLED] = AZIMUTH_analog_enabled();

  AZIMUTH_data[JSON_AZIMUTH_LOW] = AZIMUTH_get_low();
  AZIMUTH_data[JSON_AZIMUTH_HIGH] = AZIMUTH_get_high();
  AZIMUTH_data[JSON_AZIMUTH_MIDDLE] = AZIMUTH_get_middle();
  AZIMUTH_data[JSON_AZIMUTH_ACTUAL] = AZIMUTH_get_actual();

  AZIMUTH_data[JSON_AZIMUTH_TIMEOUT_TO_MIDDLE] = AZIMUTH_get_timeout();

  return AZIMUTH_data;
}

/********************************************************************
 * Create azimuth string
 *******************************************************************/
String AZIMUTH_info(void) {
  static JsonDocument doc = AZIMUTH_json();

  String text = "--- Azimuth ---";

  text.concat("\r\nAzimuth enabled: ");
  text.concat(doc[JSON_AZIMUTH_ENABLED].as<bool>());

  text.concat("\r\nAzimuth low: ");
  text.concat(doc[JSON_AZIMUTH_LOW].as<int>());

  text.concat(", middle: ");
  text.concat(doc[JSON_AZIMUTH_MIDDLE].as<int>());

  text.concat(", high: ");
  text.concat(doc[JSON_AZIMUTH_HIGH].as<int>());

  text.concat("\r\nAzimuth actual: ");
  text.concat(doc[JSON_AZIMUTH_ACTUAL].as<int>());

  text.concat("\r\nAzimuth delay: ");
  text.concat(doc[JSON_AZIMUTH_TIMEOUT_TO_MIDDLE].as<int>());

  text.concat("\r\n");
  return text;
}

/*******************************************************************
 * AZIMUTH analog
 *******************************************************************/
static void AZIMUTH_set_right_output(int value) {
  static int memo = -1;

  value = max(min(DAC_MAX, value), DAC_MIN);  // range from 0...4095
  if (abs(value - memo) > 4) {
    MCP4725_write(MCP4725_R_address, (int)value);
    memo = value;
  }
}

static int AZIMUTH_get_right_output(void) {
  int value = MCP4725_read_status(MCP4725_R_address);
  return (value != DAC_READ_ERROR) ? value : 0;
}

#ifdef ENABLE_LEFT_OUTPUT
static void AZIMUTH_set_left_output(int value) {
  static int memo = -1;

  if (value != memo) {
    memo = max(min(DAC_MAX, value), DAC_MIN);  // range from 0...4095
    MCP4725_write(MCP4725_L_address, (int)memo);

#ifdef DEBUG_AZIMUTH
    Serial.printf("Azimuth set left analog out: %d\n", memo);
#endif
  }
}
#endif

#ifdef ENABLE_LEFT_OUTPUT
static int AZIMUTH_get_left_output(void) {
  int value = MCP4725_read_status(MCP4725_L_address);
  return (value != DAC_READ_ERROR) ? value : 0;
}
#endif

/*******************************************************************
 * Azimuth enable/disable
 *******************************************************************/
void AZIMUTH_enable(void) {
  PCF8574_write(PCF8574_address, AZIMUTH_ENABLE_PIN, IO_ON);

#ifdef DEBUG_AZIMUTH
  Serial.println("Azimuth enable");
#endif
}

void AZIMUTH_disable(void) {
  PCF8574_write(PCF8574_address, AZIMUTH_ENABLE_PIN, IO_OFF);

#ifdef DEBUG_AZIMUTH
  Serial.println("Azimuth disable");
#endif
}

bool AZIMUTH_enabled(void) {
  return PCF8574_read(PCF8574_address, AZIMUTH_ENABLE_PIN);
}

bool AZIMUTH_home(void) {
  return digitalRead(AZIMUTH_HOME_PIN) == HIGH;
}

void AZIMUTH_start_homing(void) {
  PCF8574_write(PCF8574_address, AZIMUTH_START_HOMING_PIN, IO_ON);
  vTaskDelay(500 / portTICK_PERIOD_MS);
  PCF8574_write(PCF8574_address, AZIMUTH_START_HOMING_PIN, IO_OFF);
}

bool AZIMUTH_analog_enabled(void) {
  return digitalRead(AZIMUTH_ANALOG_ENABLE_PIN) == HIGH;
}

void AZIMUTH_analog_enable(void) {
  digitalWrite(AZIMUTH_ANALOG_ENABLE_PIN, IO_ON);

#ifdef DEBUG_AZIMUTH
  Serial.println("Azimuth analog enable");
#endif
}

void AZIMUTH_analog_disable(void) {
  digitalWrite(AZIMUTH_ANALOG_ENABLE_PIN, IO_OFF);

#ifdef DEBUG_AZIMUTH
  Serial.println("Azimuth analog output disable");
#endif
}

void AZIMUTH_go_to_middle(void) {
  AZIMUTH_analog_enable();
  AZIMUTH_set_steering(4096/2); // 50%
}

/*******************************************************************
 * Get/Set steering
 *******************************************************************/
int AZIMUTH_get_low(void) {
  return AZIMUTH_data[JSON_AZIMUTH_LOW].as<int>();
}

void AZIMUTH_set_low(int value) {
  if ((value >= DAC_MIN) && (value <= DAC_MAX)) {
    AZIMUTH_data[JSON_AZIMUTH_LOW] = value;
    AZIMUTH_set_steering(AZIMUTH_get_manual());  // Recalculate
  }
}

void AZIMUTH_store_low(int value) {
  if ((value >= DAC_MIN) && (value <= DAC_MAX)) {
    STORAGE_set_int(JSON_AZIMUTH_LOW, value);
  }
}

int AZIMUTH_get_high(void) {
  return AZIMUTH_data[JSON_AZIMUTH_HIGH].as<int>();
}

void AZIMUTH_set_high(int value) {
  if ((value >= DAC_MIN) && (value <= DAC_MAX)) {
    AZIMUTH_data[JSON_AZIMUTH_HIGH] = value;
    AZIMUTH_set_steering(AZIMUTH_get_manual());  // Recalculate
  }
}

void AZIMUTH_store_high(int value) {
  if ((value >= DAC_MIN) && (value <= DAC_MAX)) {
    STORAGE_set_int(JSON_AZIMUTH_HIGH, value);
  }
}

int AZIMUTH_get_middle(void) {
  return AZIMUTH_data[JSON_AZIMUTH_MIDDLE].as<int>();
}

void AZIMUTH_set_middle(int value) {
  if ((value >= DAC_MIN) && (value <= DAC_MAX)) {
    AZIMUTH_data[JSON_AZIMUTH_MIDDLE] = value;
    AZIMUTH_set_steering(value);  // Recalculate
  }
}

void AZIMUTH_store_middle(int value) {
  if ((value >= DAC_MIN) && (value <= DAC_MAX)) {
    STORAGE_set_int(JSON_AZIMUTH_MIDDLE, value);
  }
}

int AZIMUTH_get_actual(void) {
  return AZIMUTH_data[JSON_AZIMUTH_ACTUAL].as<int>();
}

void AZIMUTH_set_actual(int value) {
  if ((value >= DAC_MIN) && (value <= DAC_MAX)) {
    AZIMUTH_data[JSON_AZIMUTH_ACTUAL] = value;
  }
}

void AZIMUTH_set_manual(int value) {
  if ((value >= DAC_MIN) && (value <= DAC_MAX)) {
    AZIMUTH_data[JSON_AZIMUTH_MANUAL] = value;
  }
}

int AZIMUTH_get_manual(void) {
  return AZIMUTH_data[JSON_AZIMUTH_MANUAL].as<int>();
}

int AZIMUTH_get_timeout(void) {
  int value;
  STORAGE_get_int(JSON_AZIMUTH_TIMEOUT_TO_MIDDLE, value);
  return value;
}

void AZIMUTH_set_timeout(int value) {
  if ((value >= 0) && (value <= 120)) {
    AZIMUTH_data[JSON_AZIMUTH_TIMEOUT_TO_MIDDLE] = value;
    STORAGE_set_int(JSON_AZIMUTH_TIMEOUT_TO_MIDDLE, value);
  }
}

/********************************************************************
 * @brief Saves the calibration values for the azimuth.
 *
 * This function retrieves the calibration values from the `doc` JSON object
 * and stores them using the `STORAGE_get_int` function.
 *******************************************************************/
void AZIMUTH_calibration_save(void) {
  int value = AZIMUTH_get_low();
  AZIMUTH_store_low(value);

  value = AZIMUTH_get_middle();
  AZIMUTH_store_middle(value);

  // Store middle position in Eeprom
  Serial.printf("Write middle to Eeprom: %d\n", value);
  if (MCP4725_write_eeprom(MCP4725_R_address, value) == -1) {
    Serial.println(F("Azimuth write middle to Eeprom failed."));
  }

  value = AZIMUTH_get_high();
  AZIMUTH_store_high(value);
}

/********************************************************************
 * @brief Restores the calibration values for the azimuth.
 *
 * This function retrieves the calibration values for the low, high, and middle positions
 * of the azimuth from the storage and sets them using the corresponding setter functions.
 *
 * @note This function assumes that the calibration values have been previously stored in the storage.
 *
 * @see AZIMUTH_set_left
 * @see AZIMUTH_set_right
 * @see AZIMUTH_set_middle
 *******************************************************************/
void AZIMUTH_calibration_restore(void) {
  int value;

  STORAGE_get_int(JSON_AZIMUTH_LOW, value);
  AZIMUTH_set_low(value);

  STORAGE_get_int(JSON_AZIMUTH_HIGH, value);
  AZIMUTH_set_high(value);

  STORAGE_get_int(JSON_AZIMUTH_MIDDLE, value);
  AZIMUTH_set_middle(value);
}

/********************************************************************
 * Sets the steering value for the azimuth control.
 *
 * This function maps the input value to the range of
 * the azimuth control's left and right limits,
 * and then sets the output value accordingly.
 * The input value is expected to be in the range of LINEAR_MIN,
 * LINEAR_MIDDLE and LINEAR_MAX.
 *
 * The output value is constrained to the range of DAC_MIN to DAC_MAX.
 *
 * @param value The input value representing the desired steering position.
 *******************************************************************/
void AZIMUTH_set_steering_direct(int value) {
  value = constrain(value, DAC_MIN, DAC_MAX);  // range from 0...4095
  AZIMUTH_set_right_output(value);

#ifdef ENABLE_LEFT_OUTPUT
  AZIMUTH_set_left_output(value);
#endif
}

void AZIMUTH_set_steering(int value) {
  long left = AZIMUTH_get_low();
  long middle = AZIMUTH_get_middle();
  long right = AZIMUTH_get_high();

  if (((left < middle) && (middle < right)) ||
      ((left > middle) && (middle > right))) {
    if (value < LINEAR_MIDDLE) {
      value = map(value, LINEAR_MIN, LINEAR_MIDDLE, left, middle);
    } else {
      value = map(value, LINEAR_MIDDLE, LINEAR_MAX, middle, right);
    }
  }

  AZIMUTH_set_steering_direct(value);
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
    CLI_println(AZIMUTH_info());
    return;
  }

  if (strArg.equalsIgnoreCase("low")) {
    int val = cmd.getArg(1).getValue().toInt();
    if ((val < DAC_MIN) || (val > DAC_MAX)) {
      CLI_println("Illegal value, range: 0 ... 4096 counts.");
      return;
    }
    STORAGE_set_int(JSON_AZIMUTH_LOW, val);
    CLI_println("Azimuth low limit has been set to " + String(val) + " counts.");
  }

  if (strArg.equalsIgnoreCase("high")) {
    int val = cmd.getArg(1).getValue().toInt();
    if ((val < DAC_MIN) || (val > DAC_MAX)) {
      CLI_println("Illegal value, range: 0 ... 4096 counts.");
      return;
    }
    STORAGE_set_int(JSON_AZIMUTH_HIGH, val);
    CLI_println("Azimuth high limit has been set to " + String(val) + " counts.");
  }

  if (strArg.equalsIgnoreCase("middle")) {
    int val = cmd.getArg(1).getValue().toInt();
    if ((val < DAC_MIN) || (val > DAC_MAX)) {
      CLI_println("Illegal value, range: 0 ... 4096 counts.");
      return;
    }
    STORAGE_set_int(JSON_AZIMUTH_MIDDLE, val);
    CLI_println("Azimuth middle has been set to " + String(val) + " counts.");
  }

  if (strArg.equalsIgnoreCase("timeout")) {
    int val = cmd.getArg(1).getValue().toInt();
    if ((val < 0) || (val > 60)) {
      CLI_println("Illegal value, range: 0 ... 60s");
      return;
    }
    AZIMUTH_set_timeout(val);
    CLI_println("Azimuth timeout-to-middle has been set to " + String(val) + " seconds");
    return;
  }

  if (strArg.equalsIgnoreCase("move") && is_calibrating()) {
    // AZIMUTH_set_position(val);
    return;
  }

  CLI_println("Invalid command: AZIMUTH (left <n>, right <n>, middle <n> timeout <n>).");
}

static void cli_setup(void) {
  cli.addBoundlessCmd("azimuth", clicb_handler);
}

/********************************************************************
 * Update LED status
 *********************************************************************/
static void AZIMUTH_led_update(void) {
  if (AZIMUTH_home()) {
    digitalWrite(LED_TWAI_PIN, HIGH);
  } else {
    digitalWrite(LED_TWAI_PIN, LOW);
  }
}

/********************************************************************
 * Main task
 *********************************************************************/
static void AZIMUTH_main_task(void *parameter) {
  (void)parameter;

  while (true) {
    AZIMUTH_led_update();

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

/********************************************************************
 *  Initialize tasks
 *********************************************************************/
static void AZIMUTH_setup_tasks(void) {
  xTaskCreate(AZIMUTH_main_task, "AZIMUTH main", 2048, NULL, 10, NULL);
}

/*******************************************************************
 * GPIO setup
 *******************************************************************/
static void AZIMUTH_setup_gpio(void) {
  pinMode(AZIMUTH_ANALOG_ENABLE_PIN, OUTPUT);
  pinMode(AZIMUTH_HOME_PIN, INPUT);
  pinMode(LED_TWAI_PIN, OUTPUT);  // Used for AZIMUTH home signal
}

/*******************************************************************
 * Setup variables
 *******************************************************************/
static void AZIMUTH_setup_variables(void) {
  int value;

  if (STORAGE_get_int(JSON_AZIMUTH_LOW, value)) {
    value = DAC_MIN;
    STORAGE_set_int(JSON_AZIMUTH_LOW, value);
  }
  AZIMUTH_data[JSON_AZIMUTH_LOW] = value;

  if (STORAGE_get_int(JSON_AZIMUTH_MIDDLE, value)) {
    value = (DAC_MIN + DAC_MAX) / 2;
    STORAGE_set_int(JSON_AZIMUTH_MIDDLE, value);
  }
  AZIMUTH_data[JSON_AZIMUTH_MIDDLE] = value;

  if (STORAGE_get_int(JSON_AZIMUTH_HIGH, value)) {
    value = DAC_MAX;
    STORAGE_set_int(JSON_AZIMUTH_HIGH, value);
  }
  AZIMUTH_data[JSON_AZIMUTH_HIGH] = value;

  if (STORAGE_get_int(JSON_AZIMUTH_TIMEOUT_TO_MIDDLE, value)) {
    value = DELAY_TO_MIDDLE_DEFAULT;
    STORAGE_set_int(JSON_AZIMUTH_TIMEOUT_TO_MIDDLE, value);
  }
  AZIMUTH_data[JSON_AZIMUTH_TIMEOUT_TO_MIDDLE] = value;
}

/*******************************************************************
 * Stops the azimuth movement.
 *
 * This function disables the azimuth control and analog output.
 *******************************************************************/
void AZIMUTH_stop(void) {
  AZIMUTH_disable();
  AZIMUTH_analog_disable();

  Serial.println(F("Azimuth stopped..."));
}

/*******************************************************************
 * @brief Initializes the Azimuth module.
 *
 * This function sets up the necessary configurations and resources
 * for the Azimuth module. It should be called before using any
 * other functions related to the Azimuth module.
 *******************************************************************/
void AZIMUTH_setup(void) {
  AZIMUTH_setup_variables();
  AZIMUTH_setup_gpio();
  AZIMUTH_disable();

  Serial.println(F("Azimuth setup completed..."));
}

/*******************************************************************
 * @brief Starts the azimuth process.
 *
 * This function initializes and starts the azimuth process.
 * It should be called before any other azimuth-related functions are used.
 *******************************************************************/
void AZIMUTH_start(void) {
  AZIMUTH_setup_tasks();
  cli_setup();

  Serial.println(F("Azimuth started..."));
}
