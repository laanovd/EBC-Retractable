#include "GPIO.h"

#include <Arduino.h>

/*******************************************************************
    Buttons setup
 *******************************************************************/
void BUTTONS_setup()
{
    pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
    pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
    pinMode(BUTTON_EMERGNECY_PIN, INPUT_PULLUP);
}

/*******************************************************************
    Buttons loop
 *******************************************************************/
static bool BUTTON_UP_state = false;
static bool BUTTON_DOWN_state = false;

bool get_button_up_state() { return BUTTON_UP_state; }
bool get_button_down_state() { return BUTTON_DOWN_state; }

// * Edit this for active high/low
void BUTTON_update()
{
    BUTTON_UP_state = digitalRead(BUTTON_UP_PIN);
    BUTTON_DOWN_state = digitalRead(BUTTON_DOWN_PIN);
}

/*******************************************************************
    Up button logic
 *******************************************************************/
// Single button presses will be activated on release
// Combined button press will be activated if both buttons are down
// After button activation, short timeout will be activated

static bool BUTTON_UP_previous_state = false;
static bool BUTTON_DOWN_previous_state = false;

static int BUTTON_previous_millis = 0;

bool BUTTON_UP_is_pressed()
{
    bool is_released = BUTTON_UP_previous_state && !BUTTON_UP_state;
    BUTTON_UP_previous_state = BUTTON_UP_state;
    if (millis() - BUTTON_previous_millis < BUTTON_TIMEOUT)
        return false;
    if (is_released)
    {
        BUTTON_previous_millis = millis();
        return true;
    }
    return false;
}

bool BUTTON_DOWN_is_pressed()
{
    bool is_released = BUTTON_DOWN_previous_state && !BUTTON_DOWN_state;
    BUTTON_UP_previous_state = BUTTON_DOWN_state;
    if (millis() - BUTTON_previous_millis < BUTTON_TIMEOUT)
        return false;
    if (is_released)
    {
        BUTTON_previous_millis = millis();
        return true;
    }
    return false;
}

bool BUTTON_COMBINED_is_pressed()
{
    int is_pressed = BUTTON_UP_state && BUTTON_DOWN_state;
    if (is_pressed)
    {
        BUTTON_previous_millis = millis();
        return true;
    }
    return false;
}

bool BUTTON_EMERGENCY_is_pressed()
{
    return digitalRead(BUTTON_EMERGNECY_PIN);
}

/*******************************************************************
    Buttons setup
 *******************************************************************/
void BUTTONS_setup()
{
    pinMode(MOTOR_UP_PIN, INPUT_PULLUP);
    pinMode(MOTOR_DOWN_PIN, INPUT_PULLUP);
}
bool is_motor_retracted()
{
    return digitalRead(MOTOR_UP_PIN);
}
bool is_motor_extended()
{
    return digitalRead(MOTOR_DOWN_PIN);
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
}

/*******************************************************************
    LED heartbeat setup
 *******************************************************************/
static int LED_HEARTBEAT_state = LOW;

void LED_HEARTBEAT_set_high()
{
    LED_HEARTBEAT_state = HIGH;
    digitalWrite(LED_HEARTBEAT_PIN, HIGH);
}
void LED_HEARTBEAT_set_low()
{
    LED_HEARTBEAT_state = LOW;
    digitalWrite(LED_HEARTBEAT_PIN, LOW);
}
void LED_HEARTBEAT_update()
{
    int current = millis();
    if (current % BLINK_INTERVAL_HEARTBEAT > BLINK_INTERVAL_HEARTBEAT / 2 && LED_HEARTBEAT_state == HIGH)
        LED_HEARTBEAT_set_low();
    if (current % BLINK_INTERVAL_HEARTBEAT <= BLINK_INTERVAL_HEARTBEAT / 2 && LED_HEARTBEAT_state == LOW)
        LED_HEARTBEAT_set_high();
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
static int LED_UP_interval = -1;
static int LED_UP_state = LOW;
static int LED_UP_millis = -1;

void LED_UP_set_high()
{
    LED_UP_state = HIGH;
    digitalWrite(LED_UP_PIN, HIGH);
}
void LED_UP_set_low()
{
    LED_UP_state = LOW;
    digitalWrite(LED_UP_PIN, LOW);
}

void LED_UP_set_interval(int interval)
{
    if (interval < -1)
    {
        // TODO: Throw invalid interval value error
    }
    else if (interval == -1) // stop loop and set led low
    {
        LED_UP_set_low();
        LED_UP_interval = -1;
    }
    else if (interval == 0) // stop loop and set led low
    {
        LED_UP_set_high();
        LED_UP_interval = -1;
    }
    else
    {
        LED_UP_interval = interval;
        LED_UP_millis = millis();
    }
}

void LED_UP_update()
{
    if (LED_UP_interval > 0)
    {
        int delta_time = LED_UP_interval - millis();
        if (delta_time % (2 * LED_UP_interval) < LED_UP_interval && LED_UP_state == LOW)
            LED_UP_set_high();
        if (delta_time % (2 * LED_UP_interval) >= LED_UP_interval && LED_UP_state == HIGH)
            LED_UP_set_low();
    }
}

/*******************************************************************
    LED down controls
 *******************************************************************/
static int LED_DOWN_interval = -1;
static int LED_DOWN_state = LOW;
static int LED_DOWN_millis = -1;

void LED_DOWN_set_high()
{
    LED_DOWN_state = HIGH;
    digitalWrite(LED_DOWN_PIN, HIGH);
}

void LED_DOWN_set_low()
{
    LED_DOWN_state = HIGH;
    digitalWrite(LED_DOWN_PIN, HIGH);
}

void LED_DOWN_set_interval(int interval)
{
    if (interval < -1)
    {
        // TODO: Throw invalid interval value error
    }
    else if (interval == -1) // stop loop and set led low
    {
        LED_DOWN_set_low();
        LED_DOWN_interval = -1;
    }
    else if (interval == 0) // stop loop and set led high
    {
        LED_DOWN_set_high();
        LED_DOWN_interval = -1;
    }
    else
    {
        LED_DOWN_interval = interval;
        LED_DOWN_millis = millis();
    }
}

void LED_DOWN_update()
{
    if (LED_DOWN_interval > 0)
    {
        int delta_time = LED_DOWN_interval - millis();
        if (delta_time % (2 * LED_DOWN_interval) < LED_DOWN_interval && LED_UP_state == LOW)
            LED_DOWN_set_high();
        if (delta_time % (2 * LED_DOWN_interval) >= LED_DOWN_interval && LED_UP_state == HIGH)
            LED_DOWN_set_low();
    }
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
    Azimuth
 *******************************************************************/
static void AZIMUTH_setup()
{
    pinMode(AZIMUTH_PIN, OUTPUT);
}

void AZIMUTH_set_position(float position)
{
    analogWrite(AZIMUTH_PIN, int(position * 4095));
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
    BUTTONS_setup();
    LED_setup();
    DMC_setup();
    SENSOR_setup();
    AZIMUTH_setup();
}