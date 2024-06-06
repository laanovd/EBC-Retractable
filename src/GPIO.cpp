#include "GPIO.h"

#include <Arduino.h>

#define DEBUG_IO
// #define DEBUG_IO

/*******************************************************************
    Buttons setup
 *******************************************************************/
void BUTTON_setup()
{
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

bool BUTTON_UP_is_pressed(int delay) { return BUTTON_UP_active; }
bool BUTTON_DOWN_is_pressed(int delay) { return BUTTON_DOWN_active; }

// * Edit this for active high/low
void BUTTON_update()
{
    BUTTON_UP_state = digitalRead(BUTTON_UP_PIN) == HIGH;
    BUTTON_DOWN_state = digitalRead(BUTTON_DOWN_PIN) == HIGH;

    static bool BUTTON_UP_memo = false;
    static bool BUTTON_DOWN_memo = false;

    BUTTON_UP_active = (!BUTTON_UP_state && BUTTON_UP_memo);
    BUTTON_DOWN_active = (!BUTTON_DOWN_state && BUTTON_DOWN_memo);

    if (BUTTON_UP_state && BUTTON_DOWN_state)
        BUTTON_UP_active = BUTTON_DOWN_active = true;

    BUTTON_UP_memo = BUTTON_UP_state;
    BUTTON_DOWN_memo = BUTTON_DOWN_state;
}

bool BUTTON_EMERGENCY_is_pressed()
{
    return false; // digitalRead(BUTTON_EMERGNECY_PIN);
}

/*******************************************************************
    LED setup
 *******************************************************************/
static void LED_setup(void)
{
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

    vTaskDelay(1000 / portTICK_PERIOD_MS);
}

/*******************************************************************
    LED heartbeat setup
 *******************************************************************/
static int LED_HEARTBEAT_state = LOW;

void LED_HEARTBEAT_update()
{
    LED_HEARTBEAT_state = !LED_HEARTBEAT_state;
    digitalWrite(LED_HEARTBEAT_PIN, LED_HEARTBEAT_state);
}

/*******************************************************************
    LED error controls
 *******************************************************************/
void LED_ERROR_set_high()
{
    digitalWrite(LED_ERROR_PIN, HIGH);
}
void LED_ERROR_set_low()
{
    digitalWrite(LED_ERROR_PIN, LOW);
}

/*******************************************************************
    LED up controls
 *******************************************************************/
static int LED_UP_interval = 0;
static bool LED_UP_state = LOW;
static int LED_UP_millis = -1;

void LED_UP_set_high()
{
    LED_UP_interval = 0;
    LED_UP_state = LOW;
}

void LED_UP_set_low()
{
    LED_UP_interval = 0;
    LED_UP_state = HIGH;
}

void LED_UP_set_interval(int interval)
{
    LED_UP_interval = interval;
    LED_UP_millis = millis();
}

void LED_UP_update()
{
    if (LED_UP_interval > 0)
    {
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

void LED_DOWN_set_high()
{
    LED_DOWN_interval = 0;
    LED_DOWN_state = LOW;
}

void LED_DOWN_set_low()
{
    LED_DOWN_interval = 0;
    LED_DOWN_state = HIGH;
}

void LED_DOWN_set_interval(int interval)
{
    LED_DOWN_interval = interval;
    LED_DOWN_millis = millis();
}

void LED_DOWN_update()
{
    if (LED_DOWN_interval > 0)
    {
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
static void DMC_setup()
{
    pinMode(DMC_ENABLE_PIN, OUTPUT);
}

void DMC_set_high()
{
    digitalWrite(DMC_ENABLE_PIN, HIGH);
}
void DMC_set_low()
{
    digitalWrite(DMC_ENABLE_PIN, LOW);
}

/*******************************************************************
    Retractable sensor setup
 *******************************************************************/
static void SENSOR_setup()
{
    pinMode(MOTOR_UP_SENSOR_PIN, INPUT_PULLDOWN);
    pinMode(MOTOR_DOWN_SENSOR_PIN, INPUT_PULLDOWN);
}

bool RETRACTABLE_is_retracted()
{
    return digitalRead(MOTOR_UP_SENSOR_PIN);
}

bool RETRACTABLE_is_extended()
{
    return digitalRead(MOTOR_DOWN_SENSOR_PIN);
}

/*******************************************************************
    Steering wheel
 *******************************************************************/
float WHEEL_get_position()
{
    return analogRead(WHEEL_PIN) / 4095;
}

/*******************************************************************
    Analog enable/disable
 *******************************************************************/
static void ANALOG_OUT_setup()
{
    pinMode(ENABLE_ANALOG_OUT_PIN, OUTPUT);
}

void ANALOG_OUT_enable()
{
    digitalWrite(ENABLE_ANALOG_OUT_PIN, HIGH);
}

void ANALOG_OUT_disable()
{
    digitalWrite(ENABLE_ANALOG_OUT_PIN, HIGH);
}

/*******************************************************************
    Main GPIO setup
 *******************************************************************/
void GPIO_setup()
{
    BUTTON_setup();
    LED_setup();
    DMC_setup();
    SENSOR_setup();

    Serial.println(F("GPIO setup completed..."));
}