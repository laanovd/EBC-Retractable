/*******************************************************************
 *  Definitions and constants
 *
 *******************************************************************/
#ifndef EBC_GPIO_HEADER
#define EBC_GPIO_HEADER

#include <stdint.h>

/*******************************************************************
 * RESTfull API keys
 *******************************************************************/
#define JSON_EMERGENCY_STOP "emergency_stop"

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
// Get/set digital IO status
#define IO_ON 1
#define IO_OFF 0
#define IO_RESET -1

/*******************************************************************
 * Global variables
 *******************************************************************/
extern uint8_t PCF8574_address;
extern uint8_t MCP4725_L_address;
extern uint8_t MCP4725_R_address;

/*******************************************************************
 * Emergency stop
 *******************************************************************/
extern bool EMERGENCY_STOP_active(void);

/*******************************************************************
  * LED blink timing
 *******************************************************************/
extern bool LED_blink_takt(void);
extern bool LED_error_takt(void);

/*******************************************************************
 * General
 *******************************************************************/
extern void GPIO_setup(void);
extern void GPIO_start(void);

#endif  // EBC_GPIO_HEADER