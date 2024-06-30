#include "GPIO.h"

#include <Arduino.h>

/*******************************************************************
  Defineitions
 *******************************************************************/
#define DEBUG_IO

/*******************************************************************
    Buttons setup
 *******************************************************************/
void BUTTON_setup() {
  pinMode(BUTTON_UP_PIN, INPUT);
  pinMode(BUTTON_DOWN_PIN, INPUT);
  pinMode(BUTTON_EMERGNECY_PIN, INPUT);
}

/*******************************************************************
    Buttons loop
 *******************************************************************/
static bool BUTTON_UP_state = false;
static bool BUTTON_DOWN_state = false;

static bool BUTTON_UP_active = false;
static bool BUTTON_DOWN_active = false;

static int BUTTON_UP_millis = -1;
static int BUTTON_DOWN_millis = -1;

bool BUTTON_UP_is_pressed(int delay) { 
    return BUTTON_UP_active; 
}

bool BUTTON_DOWN_is_pressed(int delay) { 
    return BUTTON_DOWN_active; 
}

// * Edit this for active high/low
void BUTTON_update() {
  static bool BUTTON_UP_memo = false;
  static bool BUTTON_DOWN_memo = false;

  BUTTON_UP_state = digitalRead(BUTTON_UP_PIN) == HIGH;
  BUTTON_UP_active = (!BUTTON_UP_state && !BUTTON_DOWN_state && BUTTON_UP_memo);

  BUTTON_DOWN_state = digitalRead(BUTTON_DOWN_PIN) == HIGH;
  BUTTON_DOWN_active = (!BUTTON_UP_state && !BUTTON_DOWN_state && BUTTON_DOWN_memo);

  if (BUTTON_UP_state && BUTTON_DOWN_state)
    BUTTON_UP_active = BUTTON_DOWN_active = true;

  BUTTON_UP_memo = BUTTON_UP_state;
  BUTTON_DOWN_memo = BUTTON_DOWN_state;
}

/*******************************************************************
    Emergency stop
 *******************************************************************/
bool EMERGENCY_STOP_active() {
  return false;  // digitalRead(BUTTON_EMERGNECY_PIN);
}

/*******************************************************************
    LED setup
 *******************************************************************/
static void LED_setup(void) {
  pinMode(LED_UP_PIN, OUTPUT);
  pinMode(LED_DOWN_PIN, OUTPUT);
  pinMode(LED_HEARTBEAT_PIN, OUTPUT);
  pinMode(LED_ERROR_PIN, OUTPUT);

  digitalWrite(LED_UP_PIN, LOW);
  digitalWrite(LED_DOWN_PIN, LOW);
  digitalWrite(LED_HEARTBEAT_PIN, HIGH);
  digitalWrite(LED_ERROR_PIN, HIGH);

  vTaskDelay(1000 / portTICK_PERIOD_MS);

  digitalWrite(LED_UP_PIN, HIGH);
  digitalWrite(LED_DOWN_PIN, HIGH);
  digitalWrite(LED_HEARTBEAT_PIN, LOW);
  digitalWrite(LED_ERROR_PIN, LOW);
}

/*******************************************************************
  LED heartbeat setup
 *******************************************************************/
void LED_HEARTBEAT_update() {
  digitalWrite(LED_HEARTBEAT_PIN, !digitalRead(LED_HEARTBEAT_PIN));
}

int LED_HAERTBEAT_on() {
  return digitalRead(LED_HEARTBEAT_PIN);
}

/*******************************************************************
  LED error setup
 *******************************************************************/
void LED_ERROR_update() {
  int state = true ? LED_HAERTBEAT_on() : LOW;  // TODO: link general error status
  digitalWrite(LED_ERROR_PIN, state);
}

/*******************************************************************
    LED up controls
 *******************************************************************/
static int LED_UP_interval = 0;
static bool LED_UP_state = LOW;
static int LED_UP_millis = -1;

void LED_UP_on() {
  LED_UP_interval = 0;
  LED_UP_state = LOW;
}

void LED_UP_off() {
  LED_UP_interval = 0;
  LED_UP_state = HIGH;
}

void LED_UP_set_interval(int interval) {
  LED_UP_interval = interval;
  LED_UP_millis = millis();
}

void LED_UP_update() {
  if (LED_UP_interval > 0) {
    int delta_time = millis() - LED_UP_millis;
    if (delta_time % (2 * LED_UP_interval) < LED_UP_interval)
      LED_UP_state = HIGH;
    if (delta_time % (2 * LED_UP_interval) >= LED_UP_interval)
      LED_UP_state = LOW;
  }

  digitalWrite(LED_UP_PIN, LED_UP_state);
}

/*******************************************************************
    LED down controls
 *******************************************************************/
static int LED_DOWN_interval = 0;
static bool LED_DOWN_state = LOW;
static int LED_DOWN_millis = -1;

void LED_DOWN_on() {
  LED_DOWN_interval = 0;
  LED_DOWN_state = LOW;
}

void LED_DOWN_off() {
  LED_DOWN_interval = 0;
  LED_DOWN_state = HIGH;
}

void LED_DOWN_set_interval(int interval) {
  LED_DOWN_interval = interval;
  LED_DOWN_millis = millis();
}

void LED_DOWN_update() {
  if (LED_DOWN_interval > 0) {
    int delta_time = millis() - LED_DOWN_millis;
    if (delta_time % (2 * LED_DOWN_interval) < LED_DOWN_interval)
      LED_DOWN_state = HIGH;
    if (delta_time % (2 * LED_DOWN_interval) >= LED_DOWN_interval)
      LED_DOWN_state = LOW;
  }

  digitalWrite(LED_DOWN_PIN, LED_DOWN_state);
}

/*******************************************************************
    DMC Enable setup
 *******************************************************************/
void DMC_set_high() {
  digitalWrite(DMC_ENABLE_PIN, HIGH);
}
void DMC_set_low() {
  digitalWrite(DMC_ENABLE_PIN, LOW);
}

bool DMC_enabled() {
  return digitalRead(DMC_ENABLE_PIN) == HIGH;
}

static void DMC_setup() {
  pinMode(DMC_ENABLE_PIN, OUTPUT);
  DMC_set_low();
}

/*******************************************************************
  RETRACTABLE enable setup
 *******************************************************************/
void RETRACTABLE_enable() {
  digitalWrite(UP_DOWN_ENABLE_PIN, HIGH);
#ifdef DEBUG_IO
  Serial.println("Up-down ENABLE");
#endif
}
void RETRACTABLE_disable() {
  digitalWrite(UP_DOWN_ENABLE_PIN, LOW);

#ifdef DEBUG_IO
  Serial.println("Up-down DISABLE");
#endif
}

bool RETRACTABLE_enabled() {
  return digitalRead(UP_DOWN_ENABLE_PIN) == HIGH;
}

static void RETRACTABLE_setup() {
  pinMode(UP_DOWN_ENABLE_PIN, OUTPUT);
  RETRACTABLE_disable();
}

/*******************************************************************
  Retractable motor setup
 *******************************************************************/
void MOTOR_UP_on() {
  digitalWrite(MOTOR_UP_PIN, HIGH);
#ifdef DEBUG_IO
  Serial.println("Retractable up ON");
#endif
}

void MOTOR_UP_off() {
  digitalWrite(MOTOR_UP_PIN, LOW);
#ifdef DEBUG_IO
  Serial.println("Retractable up OFF");
#endif
}

void MOTOR_DOWN_on() {
  digitalWrite(MOTOR_DOWN_PIN, HIGH);
#ifdef DEBUG_IO
  Serial.println("Retractable down ON");
#endif
}

void MOTOR_DOWN_off() {
  digitalWrite(MOTOR_DOWN_PIN, LOW);
#ifdef DEBUG_IO
  Serial.println("Retractable down OFF");
#endif
}

static void MOTOR_UP_DOWN_setup() {
  pinMode(MOTOR_UP_PIN, INPUT_PULLDOWN);
  MOTOR_UP_off();
  pinMode(MOTOR_DOWN_PIN, INPUT_PULLDOWN);
  MOTOR_DOWN_off();
}

/*******************************************************************
    Analog enable/disable
 *******************************************************************/
void ANALOG_OUT_enable() {
  digitalWrite(ENABLE_ANALOG_OUT_PIN, HIGH);
#ifdef DEBUG_IO
  Serial.println("Enable steering");
#endif
}

void ANALOG_OUT_disable() {
  digitalWrite(ENABLE_ANALOG_OUT_PIN, HIGH);
#ifdef DEBUG_IO
  Serial.println("Disable steering");
#endif
}

bool ANALOG_OUT_enabled() {
  return digitalRead(ENABLE_ANALOG_OUT_PIN) == HIGH;
}

static void ANALOG_OUT_setup() {
  pinMode(ENABLE_ANALOG_OUT_PIN, OUTPUT);
  ANALOG_OUT_disable();
}

/*******************************************************************
    Retractable sensor setup
 *******************************************************************/
static void SENSOR_setup() {
  pinMode(MOTOR_UP_SENSOR_PIN, INPUT_PULLDOWN);
  pinMode(MOTOR_DOWN_SENSOR_PIN, INPUT_PULLDOWN);
}

bool RETRACTABLE_is_retracted() {
  return digitalRead(MOTOR_UP_SENSOR_PIN) == HIGH;
}

bool RETRACTABLE_is_extended() {
  return digitalRead(MOTOR_DOWN_SENSOR_PIN) == HIGH;
}

/*******************************************************************
    Steering wheel
 *******************************************************************/
int STEERING_WHEEL_get_position() {
  return analogRead(WHEEL_PIN);
}

/*******************************************************************
  GPIO test
 *******************************************************************/
void GPIO_test() {
  // digitalWrite(DMC_ENABLE_PIN, !digitalRead(DMC_ENABLE_PIN));
  // digitalWrite(UP_DOWN_ENABLE_PIN, !digitalRead(UP_DOWN_ENABLE_PIN));
  // digitalWrite(MOTOR_UP_PIN, !digitalRead(MOTOR_UP_PIN));
  // digitalWrite(MOTOR_DOWN_PIN, !digitalRead(MOTOR_DOWN_PIN));
  // digitalWrite(ENABLE_ANALOG_OUT_PIN, !digitalRead(ENABLE_ANALOG_OUT_PIN));
}

/*******************************************************************
    Main GPIO setup
 *******************************************************************/
void GPIO_setup() {
  BUTTON_setup();
  LED_setup();
  DMC_setup();
  RETRACTABLE_setup();
  MOTOR_UP_DOWN_setup();
  ANALOG_OUT_enable();
  SENSOR_setup();

  Serial.println(F("GPIO setup completed..."));
}