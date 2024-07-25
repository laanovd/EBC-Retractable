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
#define DELAY_TO_MIDDLE_DEFAULT 5

/*******************************************************************
 * Global variables
 *******************************************************************/
static JsonDocument AZIMUTH_data;

/********************************************************************
 * Create initial JSON data
 *******************************************************************/
static JsonDocument AZIMUTH_json(void) {
  float f;
  int i;

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

  text.concat(", high: ");
  text.concat(doc[JSON_AZIMUTH_HIGH].as<int>());

  text.concat(", middle: ");
  text.concat(doc[JSON_AZIMUTH_MIDDLE].as<int>());

  text.concat("\r\nAzimuth actual: ");
  text.concat(doc[JSON_AZIMUTH_ACTUAL].as<bool>());

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

bool AZIMUTH_enabled() {
  return PCF8574_read(PCF8574_address, AZIMUTH_ENABLE_PIN);
}

bool AZIMUTH_home() {
  return digitalRead(AZIMUTH_HOME_PIN) == HIGH;
}

void AZIMUTH_start_homing() {
  PCF8574_write(PCF8574_address, AZIMUTH_START_HOMING_PIN, IO_ON);
  vTaskDelay(500 / portTICK_PERIOD_MS);
  PCF8574_write(PCF8574_address, AZIMUTH_START_HOMING_PIN, IO_OFF);
}

bool AZIMUTH_analog_enabled() {
  return digitalRead(AZIMUTH_ANALOG_ENABLE_PIN) == HIGH;
}

void AZIMUTH_analog_enable() {
  digitalWrite(AZIMUTH_ANALOG_ENABLE_PIN, IO_ON);

#ifdef DEBUG_AZIMUTH
  Serial.println("Azimuth analog enable");
#endif
}

void AZIMUTH_analog_disable() {
  digitalWrite(AZIMUTH_ANALOG_ENABLE_PIN, IO_OFF);

#ifdef DEBUG_AZIMUTH
  Serial.println("Azimuth analog output disable");
#endif
}

/*******************************************************************
 * Get/Set steering
 *******************************************************************/
int AZIMUTH_get_low(void) {
  int value;
  STORAGE_get_int(JSON_AZIMUTH_LOW, value);
  return value;
}

void AZIMUTH_set_low(int value) {
  if ((value >= DAC_MIN) && (value <= DAC_MAX)) {
    AZIMUTH_data[JSON_AZIMUTH_LOW] = value;
    STORAGE_set_int(JSON_AZIMUTH_LOW, value);
    AZIMUTH_set_steering(AZIMUTH_get_manual());  // Recalculate
  }
}

int AZIMUTH_get_high(void) {
  int value;
  STORAGE_get_int(JSON_AZIMUTH_HIGH, value);
  return value;
}

void AZIMUTH_set_high(int value) {
  if ((value >= DAC_MIN) && (value <= DAC_MAX)) {
    AZIMUTH_data[JSON_AZIMUTH_HIGH] = value;
    STORAGE_set_int(JSON_AZIMUTH_HIGH, value);
    AZIMUTH_set_steering(AZIMUTH_get_manual());  // Recalculate
  }
}

int AZIMUTH_get_middle(void) {
  int value;
  STORAGE_get_int(JSON_AZIMUTH_MIDDLE, value);
  return value;
}

void AZIMUTH_set_middle(int value) {
  if ((value >= DAC_MIN) && (value <= DAC_MAX)) {
    AZIMUTH_data[JSON_AZIMUTH_MIDDLE] = value;
    STORAGE_set_int(JSON_AZIMUTH_MIDDLE, value);
    AZIMUTH_set_steering(AZIMUTH_get_manual());  // Recalculate
  }
}

int AZIMUTH_get_actual(void) {
  // return AZIMUTH_get_right_output();
  return AZIMUTH_data[JSON_AZIMUTH_ACTUAL].as<int>();
}

void AZIMUTH_set_steering(int value) {
  long left = AZIMUTH_get_low();
  long right = AZIMUTH_get_high();

  int output = mapl(value, 0, 4095, left, right);  // map to DAC_MIN...DAC_MAX
  AZIMUTH_set_right_output(output);

  AZIMUTH_data[JSON_AZIMUTH_ACTUAL] = output;

#ifdef ENABLE_LEFT_OUTPUT
  AZIMUTH_set_left_output(output);
#endif
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

void AZIMUTH_set_manual(int value) {
  if ((value >= 0) && (value <= 4096)) {
    AZIMUTH_data[JSON_AZIMUTH_MANUAL] = value;
    AZIMUTH_set_steering(value);
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
    CLI_println(AZIMUTH_info());
    return;
  }

  if (strArg.equalsIgnoreCase("low")) {
    int val = cmd.getArg(1).getValue().toInt();
    if ((val < DAC_MIN) || (val > DAC_MAX)) {
      CLI_println("Illegal value, range: 400 ... 4096 counts.");
      return;
    }
    STORAGE_set_int(JSON_AZIMUTH_LOW, val);
    CLI_println("Azimuth low limit has been set to " + String(val) + " counts.");
  }

  if (strArg.equalsIgnoreCase("high")) {
    int val = cmd.getArg(1).getValue().toInt();
    if ((val < DAC_MIN) || (val > DAC_MAX)) {
      CLI_println("Illegal value, range: 4000 ... 4096 counts.");
      return;
    }
    STORAGE_set_float(JSON_AZIMUTH_HIGH, val);
    CLI_println("Azimuth high limit has been set to " + String(val) + " counts.");
  }

  if (strArg.equalsIgnoreCase("middle")) {
    int val = cmd.getArg(1).getValue().toInt();
    if ((val < DAC_MIN) || (val > DAC_MAX)) {
      CLI_println("Illegal value, range: 400 ... 4096 counts.");
      return;
    }
    STORAGE_set_float(JSON_AZIMUTH_MIDDLE, val);
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
  pinMode(LED_TWAI_PIN, OUTPUT); // Used for AZIMUTH home signal
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
void AZIMUTH_stop() {
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
void AZIMUTH_setup() {
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
void AZIMUTH_start() {
  AZIMUTH_setup_tasks();
  cli_setup();

  Serial.println(F("Azimuth started..."));
}
