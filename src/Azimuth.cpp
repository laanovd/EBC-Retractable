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

  AZIMUTH_data[JSON_AZIMUTH_LEFT] = AZIMUTH_get_left();
  AZIMUTH_data[JSON_AZIMUTH_RIGHT] = AZIMUTH_get_right();
  // AZIMUTH_data[JSON_AZIMUTH_ACTUAL] = AZIMUTH_get_actual();

  AZIMUTH_data[JSON_AZIMUTH_TIMEOUT_TO_MIDDLE] = AZIMUTH_get_to_middle_timeout();

  AZIMUTH_data[JSON_STEERWHEEL_LEFT] = STEERWHEEL_get_left();
  AZIMUTH_data[JSON_STEERWHEEL_RIGHT] = STEERWHEEL_get_right();
  AZIMUTH_data[JSON_STEERWHEEL_MIDDLE] = STEERWHEEL_get_middle();
  AZIMUTH_data[JSON_STEERWHEEL_ACTUAL] = STEERWHEEL_get_actual();
  AZIMUTH_data[JSON_STEERWHEEL_LEFT] = STEERWHEEL_get_left();
  AZIMUTH_data[JSON_STEERWHEEL_RIGHT] = STEERWHEEL_get_right();

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

  text.concat("\r\nAzimuth left: ");
  text.concat(doc[JSON_STEERWHEEL_LEFT].as<float>());

  text.concat("\r\nAzimuth right: ");
  text.concat(doc[JSON_STEERWHEEL_RIGHT].as<float>());

  text.concat("\r\nAzimuth right: ");
  text.concat(doc[JSON_STEERWHEEL_RIGHT].as<float>());

  text.concat("\r\nAzimuth actual: ");
  text.concat(doc[JSON_AZIMUTH_ACTUAL].as<bool>());

  text.concat("\r\nAzimuth delay: ");
  text.concat(doc[JSON_AZIMUTH_TIMEOUT_TO_MIDDLE].as<int>());

  text.concat("\r\n");
  return text;
}

/*******************************************************************
 * Scale functions
 *******************************************************************/
float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
  if ((in_max - in_min) != 0) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  }
  return 0.0;
}

long mapl(long x, long in_min, long in_max, long out_min, long out_max) {
  if ((in_max - in_min) != 0) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  }
  return 0L;
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
  Serial.println("Azimuth analog output enable");
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
int AZIMUTH_get_left(void) {
  int value;
  STORAGE_get_int(JSON_AZIMUTH_LEFT, value);
  return value;
}

void AZIMUTH_set_left(int value) {
  if ((value >= DAC_MIN) && (value <= DAC_MAX)) {
    STORAGE_set_int(JSON_AZIMUTH_LEFT, value);
    AZIMUTH_set_steering(AZIMUTH_get_manual());  // Recalculate
  }
}

int AZIMUTH_get_right(void) {
  int value;
  STORAGE_get_int(JSON_AZIMUTH_RIGHT, value);
  return value;
}

void AZIMUTH_set_right(int value) {
  if ((value >= DAC_MIN) && (value <= DAC_MAX)) {
    STORAGE_set_int(JSON_AZIMUTH_RIGHT, value);
    AZIMUTH_set_steering(AZIMUTH_get_manual());  // Recalculate
  }
}

int AZIMUTH_get_actual(void) {
  // return AZIMUTH_get_right_output();
  return AZIMUTH_data[JSON_AZIMUTH_ACTUAL].as<int>();
}

void AZIMUTH_set_steering(int value) {
  long left = AZIMUTH_get_left();
  long right = AZIMUTH_get_right();

  int output = mapl(value, DAC_MIN, DAC_MAX, left, right);  // map to DAC_MIN...DAC_MAX
  AZIMUTH_set_right_output(output);

  AZIMUTH_data[JSON_AZIMUTH_ACTUAL] = output;

#ifdef ENABLE_LEFT_OUTPUT
  AZIMUTH_set_left_output(output);
#endif
}

int AZIMUTH_get_manual(void) {
  return AZIMUTH_data[JSON_AZIMUTH_MANUAL].as<int>();
}

int AZIMUTH_get_to_middle_timeout(void) {
  int value;
  STORAGE_get_int(JSON_AZIMUTH_TIMEOUT_TO_MIDDLE, value);
  return value;
}

void AZIMUTH_set_to_middle_timeout(int value) {
  if ((value >= 0) && (value <= 120)) {
    STORAGE_set_int(JSON_AZIMUTH_TIMEOUT_TO_MIDDLE, value);
  }
}

/*******************************************************************
 * Output settings
 *******************************************************************/
int STEERWHEEL_get_left(void) {
  int value;
  STORAGE_get_int(JSON_STEERWHEEL_LEFT, value);
  return value;
}

void STEERWHEEL_set_left(int value) {
  if ((value >= ADC_MIN) && (value <= ADC_MAX)) {
    STORAGE_set_int(JSON_STEERWHEEL_LEFT, value);
  }
}

int STEERWHEEL_get_right(void) {
  int value;
  STORAGE_get_int(JSON_STEERWHEEL_RIGHT, value);
  return value;
}

void STEERWHEEL_set_right(int value) {
  if ((value >= ADC_MIN) && (value <= ADC_MAX)) {
    STORAGE_set_int(JSON_STEERWHEEL_RIGHT, value);
  }
}

int STEERWHEEL_get_middle(void) {
  int value;
  STORAGE_get_int(JSON_STEERWHEEL_MIDDLE, value);
  return value;
}

void STEERWHEEL_set_middle(int value) {
  if ((value >= DAC_MIN) && (value <= DAC_MAX)) {
    STORAGE_set_int(JSON_STEERWHEEL_MIDDLE, value);
  }
}

#define MAX_AVERAGE 20
int STEERWHEEL_get_actual(void) {
  static int ndx = 0;
  static int array[MAX_AVERAGE] = {0};
  static long sum = 0;
  int left = STEERWHEEL_get_left();
  int right = STEERWHEEL_get_right();

  sum -= array[ndx];
  array[ndx] = analogRead(STEER_WHEEL_ANALOG_CHANNEL);
  sum += array[ndx];
  ndx = (ndx + 1) % MAX_AVERAGE;

  int value = map((int) sum/MAX_AVERAGE, left, right, DAC_MIN, DAC_MAX);
  return (int) max(min(value, ADC_MAX), ADC_MIN);
}

void AZIMUTH_set_manual(int value) {
  if ((value >= DAC_MIN) && (value <= DAC_MAX)) {
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

  if (strArg.equalsIgnoreCase("left")) {
    float val = cmd.getArg(1).getValue().toFloat();
    if ((val < 0.0) || (val > 5.0)) {
      CLI_println("Illegal value, range: 0.0V ... 5.0V");
      return;
    }
    STORAGE_set_float(JSON_STEERWHEEL_LEFT, val);
    CLI_println("Azimuth left limit has been set to " + String(val) + " Volt");
  }

  if (strArg.equalsIgnoreCase("right")) {
    float val = cmd.getArg(1).getValue().toFloat();
    if ((val < 0.0) || (val > 5.0)) {
      CLI_println("Illegal value, range: 0.0V ... 5.0V");
      return;
    }
    STORAGE_set_float(JSON_STEERWHEEL_RIGHT, val);
    CLI_println("Azimuth right limit has been set to " + String(val) + " Volt");
  }

  if (strArg.equalsIgnoreCase("delay")) {
    int val = cmd.getArg(1).getValue().toInt();
    if ((val < 0) || (val > 60)) {
      CLI_println("Illegal value, range: 0 ... 60s");
      return;
    }
    AZIMUTH_set_to_middle_timeout(val);
    CLI_println("Azimuth delay-to-middle has been set to " + String(val) + " seconds");
    return;
  }

  if (strArg.equalsIgnoreCase("move") && is_calibrating()) {
    // AZIMUTH_set_position(val);
    return;
  }

  CLI_println("Invalid command: AZIMUTH (left <n>, right <n>, delay <n>, move).");
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

  vTaskDelay(2000 / portTICK_PERIOD_MS);  // Startup delay

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
  pinMode(LED_TWAI_PIN, OUTPUT);
  pinMode(STEER_WHEEL_ANALOG_CHANNEL, INPUT);
}

/*******************************************************************
 * Setup variables
 *******************************************************************/
static void AZIMUTH_setup_variables(void) {
  int value;

  if (STORAGE_get_int(JSON_AZIMUTH_LEFT, value)) {
    value = DAC_MIN;
    STORAGE_set_int(JSON_AZIMUTH_LEFT, value);
  }
  AZIMUTH_data[JSON_AZIMUTH_LEFT] = value;

  if (STORAGE_get_int(JSON_AZIMUTH_RIGHT, value)) {
    value = DAC_MAX;
    STORAGE_set_int(JSON_AZIMUTH_RIGHT, value);
  }
  AZIMUTH_data[JSON_AZIMUTH_RIGHT] = value;

  if (STORAGE_get_int(JSON_AZIMUTH_TIMEOUT_TO_MIDDLE, value)) {
    value = DELAY_TO_MIDDLE_DEFAULT;
    STORAGE_set_int(JSON_AZIMUTH_TIMEOUT_TO_MIDDLE, value);
  }
  AZIMUTH_data[JSON_AZIMUTH_TIMEOUT_TO_MIDDLE] = value;

  if (STORAGE_get_int(JSON_STEERWHEEL_LEFT, value)) {
    value = ADC_MAX;
    STORAGE_set_int(JSON_STEERWHEEL_LEFT, value);
  }
  AZIMUTH_data[JSON_STEERWHEEL_LEFT] = value;

  if (STORAGE_get_int(JSON_STEERWHEEL_RIGHT, value)) {
    value = ADC_MIN;
    STORAGE_set_int(JSON_STEERWHEEL_RIGHT, value);
  }
  AZIMUTH_data[JSON_STEERWHEEL_RIGHT] = value;

  if (STORAGE_get_int(JSON_STEERWHEEL_MIDDLE, value)) {
    value = (ADC_MAX - ADC_MIN) / 2;
    STORAGE_set_int(JSON_STEERWHEEL_MIDDLE, value);
  }
  AZIMUTH_data[JSON_STEERWHEEL_MIDDLE] = value;
}

/*******************************************************************
 * Azimuth general
 *******************************************************************/
void AZIMUTH_setup() {
  AZIMUTH_setup_variables();
  AZIMUTH_setup_gpio();
  AZIMUTH_disable();

  Serial.println(F("Azimuth setup completed..."));
}

void AZIMUTH_start() {
  // AZIMUTH_setup_tasks();
  cli_setup();

  Serial.println(F("Azimuth started..."));
}
