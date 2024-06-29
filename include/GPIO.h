/*******************************************************************
 *  Definitions and constants
 *
 *******************************************************************/
#ifndef GPIO_HEADER
#define GPIO_HEADER

/*******************************************************************
 *  Blinking intervals
 *******************************************************************/
#define BLINK_INTERVAL_EMERGENCY 250
#define BLINK_INTERVAL_NO_POSITION 250
#define BLINK_INTERVAL_CALIBRATING 500
#define BLINK_INTERVAL_MOVING 500
#define BLINK_INTERVAL_HEARTBEAT 100

#define BUTTON_TIMEOUT 100

/*******************************************************************
 *  GPIO defenitions
 *******************************************************************/
#define BUTTON_UP_PIN 23
#define BUTTON_DOWN_PIN 25
#define BUTTON_EMERGNECY_PIN 2

#define LED_UP_PIN 14
#define LED_DOWN_PIN 26
#define LED_HEARTBEAT_PIN 15
#define LED_ERROR_PIN 12

// Outputs
#define DMC_ENABLE_PIN 16
#define UP_DOWN_ENABLE_PIN 17
#define MOTOR_UP_PIN 18
#define MOTOR_DOWN_PIN 19

#define MOTOR_UP_SENSOR_PIN 36
#define MOTOR_DOWN_SENSOR_PIN 39

#define WHEEL_PIN 32

#define AZIMUTH_PIN 27

#define ENABLE_ANALOG_OUT_PIN 27

/*******************************************************************
 *  Buttons
 *******************************************************************/
extern void BUTTON_update();

extern bool BUTTON_UP_is_pressed(int delay = 0);
extern bool BUTTON_DOWN_is_pressed(int delay = 0);

extern bool EMERGENCY_STOP_active();

extern int STEERING_WHEEL_get_position();

/*******************************************************************
 *  LEDs
 *******************************************************************/
extern void LED_HEARTBEAT_update();
extern void LED_ERROR_update();

extern void LED_UP_off();
extern void LED_UP_on();
extern void LED_UP_set_interval(int interval);
extern void LED_UP_update();

extern void LED_DOWN_off();
extern void LED_DOWN_on();
extern void LED_DOWN_set_interval(int interval);
extern void LED_DOWN_update();

/*******************************************************************
 *  Sensors
 *******************************************************************/
extern bool RETRACTABLE_is_retracted();
extern bool RETRACTABLE_is_extended();

/*******************************************************************
    Analog enable/disable
 *******************************************************************/
extern void ANALOG_OUT_enable();
extern void ANALOG_OUT_disable();
extern bool ANALOG_OUT_enabled();

/*******************************************************************
  Retractable setup
 *******************************************************************/
extern void RETRACTABLE_enable();
extern void RETRACTABLE_disable();
extern bool RETRACTABLE_enabled();

extern void MOTOR_UP_on();
extern void MOTOR_UP_off();
extern void MOTOR_DOWN_on();
extern void MOTOR_DOWN_off();

/*******************************************************************
    DMC enable/disable
 *******************************************************************/
extern bool DMC_enabled();
extern void DMC_set_high();
extern void DMC_set_low();

/*******************************************************************
    GPIO Setup
 *******************************************************************/
extern void GPIO_test();
extern void GPIO_setup();

#endif