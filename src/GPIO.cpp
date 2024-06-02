#include <Arduino.h>

/*******************************************************************
    Buttons setup
 *******************************************************************/
#define BUTTON_UP_PIN 23
#define BUTTON_DOWN_PIN 25
#define BUTTON_EMERGNECY_PIN 2
void BUTTONS_setup()
{
    pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
    pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
    pinMode(BUTTON_EMERGNECY_PIN, INPUT_PULLUP);
}

#define BUTTON_DELAY_MS 250

/*******************************************************************
    Up button logic
 *******************************************************************/
// Single button presses will be activated on release
// Combined button press will be activated if both buttons are down
// After button activation, short timeout will be activated
#define BUTTON_TIMEOUT 100
#define BUTTON_COMPARE_VALUE LOW // Value to compare for active high/low

int BUTTON_UP_previous_state = LOW;
int BUTTON_DOWN_previous_state = LOW;

int BUTTON_previous_millis = 0;

bool BUTTON_UP_is_pressed()
{
    int current_state = digitalRead(BUTTON_UP_PIN);
    bool is_released = BUTTON_UP_previous_state == !BUTTON_COMPARE_VALUE && current_state == BUTTON_COMPARE_VALUE;
    BUTTON_UP_previous_state = current_state;
    if (millis() - BUTTON_previous_millis < BUTTON_TIMEOUT)
        return false;
    {
        BUTTON_previous_millis = millis();
        return true;
    }
    return false;
}

bool BUTTON_DOWN_is_pressed()
{
    int current_state = digitalRead(BUTTON_DOWN_PIN);
    bool is_released = BUTTON_DOWN_previous_state == !BUTTON_COMPARE_VALUE && current_state == BUTTON_COMPARE_VALUE;
    BUTTON_UP_previous_state = current_state;
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
    int is_pressed = digitalRead(BUTTON_UP_PIN) && digitalRead(BUTTON_DOWN_PIN);
    if(is_pressed) {
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
#define MOTOR_UP_PIN 18
#define MOTOR_DOWN_PIN 19
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
#define LED_UP_PIN 14
#define LED_DOWN_PIN 26
#define LED_HEARTBEAT_PIN 2
#define LED_ERROR_PIN 27
void LED_setup(void)
{
    pinMode(LED_UP_PIN, OUTPUT);
    pinMode(LED_DOWN_PIN, OUTPUT);
    pinMode(LED_HEARTBEAT_PIN, OUTPUT);
    pinMode(LED_ERROR_PIN, OUTPUT);
}

/*******************************************************************
    LED heartbeat setup
 *******************************************************************/
#define HEARTBEAT_FREQUENCY 1000
int LED_HEARTBEAT_state = LOW;

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
    if (current % (2 * HEARTBEAT_FREQUENCY) > HEARTBEAT_FREQUENCY && LED_HEARTBEAT_state == HIGH)
        LED_HEARTBEAT_set_low();
    if (current % (2 * HEARTBEAT_FREQUENCY) <= HEARTBEAT_FREQUENCY && LED_HEARTBEAT_state == LOW)
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
int LED_UP_interval = -1;
int LED_UP_state = LOW;
int LED_UP_millis = -1;

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
int LED_DOWN_interval = -1;
int LED_DOWN_state = LOW;
int LED_DOWN_millis = -1;

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
#define DMC_ENABLE_PIN 16
// TODO: Check if enable is high or low
void DMC_setup()
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
    Main GPIO setup
 *******************************************************************/
void GPIO_setup()
{
    BUTTONS_setup();
    LED_setup();
    DMC_setup();
}