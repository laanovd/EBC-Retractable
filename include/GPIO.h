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
#define BLINK_INTERVAL_NO_POSITION 500
#define BLINK_INTERVAL_CALIBRATING 500
#define BLINK_INTERVAL_MOVING 500
#define BLINK_INTERVAL_HEARTBEAT 500

#define BUTTON_TIMEOUT 100

/*******************************************************************
 *  GPIO defenitions
 *******************************************************************/
#define BUTTON_UP_PIN 23
#define BUTTON_DOWN_PIN 25
#define BUTTON_EMERGNECY_PIN 2

#define LED_UP_PIN 14
#define LED_DOWN_PIN 26
#define LED_HEARTBEAT_PIN 2
#define LED_ERROR_PIN 27

#define MOTOR_UP_PIN 18
#define MOTOR_DOWN_PIN 19

#define MOTOR_UP_SENSOR_PIN 18
#define MOTOR_DOWN_SENSOR_PIN 19

#define DMC_ENABLE_PIN 16

#define WHEEL_PIN 32

#define AZIMUTH_PIN 0

#define ENABLE_ANALOG_OUT_PIN 27

/*******************************************************************
 *  Azimuth
 *******************************************************************/
void AZIMUTH_set_position(float position);

#endif