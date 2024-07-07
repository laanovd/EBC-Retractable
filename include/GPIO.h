/*******************************************************************
 *  Definitions and constants
 *
 *******************************************************************/
#ifndef EBC_GPIO_HEADER
#define EBC_GPIO_HEADER

#include <stdint.h>

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
 * PCF8574 digital output
 *******************************************************************/
extern void PCF8574_write(int output, int value);
extern bool PCF8574_read(int output);

/*******************************************************************
 * MC4725 DAC out (12bit)
 *******************************************************************/
extern void MCP4725_write(uint8_t address, uint16_t value);

extern uint8_t MCP4725_DAC_R;
extern uint8_t MCP4725_DAC_L;

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