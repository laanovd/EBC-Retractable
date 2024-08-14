/*******************************************************************
 *  EBC-e Retractable Control Unit (RCU) - Config.h
 *
 * @versions: A0.2.00 2024-07-31
 * - initial version.
 * 
 * @versions: A0.3.00 2024-08-01
 * - Add dounble linearisation of steering wheel / azimuth.
 * - Add version to main web-page.
 * - Add actual azumith edit field, and keep in sync with manual slider.
 * - Turn of blinking error led.
 *
 * @versions: A0.4.00 2024-08-08
 * - Change minimum ADC from 400 to 0.
 * - Add I2C addresses to system information in CLI.
 * - Fix enable/disable manintenance mode
 * - Fix showing asimzuth actual value
 * - Change Steerwheel actual text in Raw in maintenace screen.
 * - Code cleanup
 * 
 * @versions: A0.5.00 2024-08-12
 * - Remove LIFT_disable() on multiple places.
 * - Save azimuth middle calibration value to MCP4725 Eeprom for right output
 *
 * @versions: A0.6.00 2024-08-13
 * - Add button actions to stop lift moving up/down.
 * - Add Save and restore calibration values for azimuth.
 * - Add azimuth go to middle functionality. (keep azimuth analog enabled)
 * - Return to init-state after maintenance mode. (not to no-position)
 * 
 *******************************************************************/
#ifndef CONFIG_HEADER_ID
#define CONFIG_HEADER_ID

/*******************************************************************
 * Program name and version definitions
 *******************************************************************/
#define ProgramName "EBC-Retractable"   // Program name
#define ProgramVersion "A0.6"           // Program version
#define ProgramTitle "EBC-RCU"          // Title Web interface
#define WiFiSSIDPrefix "EBC-E"          // Wifi ssid name
#define PCBVersion "RCU v0.3"           // Hardware version
#define HTMLTitlePrefix "RCU"           // HTML page title prefix

/*******************************************************************
 * Global debug flags
 *******************************************************************/
#define DEBUG_API

/*******************************************************************
 * CAN receive handler
 *******************************************************************/
typedef void (*CANReceiveHandler)(uint32_t id, uint8_t *buffer, uint8_t size, bool rtr, bool ext);

/*******************************************************************
 * Communication
 *******************************************************************/
/* Communication error countdown preload */
#define ERROR_DELAY_PRELOAD 5

/* Communication */
#define ERROR_RELOAD_COUNTS 7 /* Communication error delay */

/* Program settings */
#define DEFAUL_EMULATION_MODE operational_mode_t::MODE_UNKNOWN
#define DEFAUL_EMULATION_INTERVAL 1000
#define DEFAUL_EMULATION_DYNAMIC 1

/*******************************************************************
 * Hardware definitions
 *******************************************************************/
#define MOSI 25     // SPI - Master Out, slave in
#define MISO 19     // SPI - Master In, slave Out
#define SCK 18      // SPI - Master Clock
#define RST_MCP 23  // Hardware Reset MCP2515

#define SPI0_INT 16  // Int melding MCP2515 Can controller
#define SPI0_CS 17   // Chip select MCP2515 Can controller

// JTAG aansluiting
#define ESP_TDI 12  // ...
#define ESP_TCK 13  // ...
#define ESP_TMS 14  // ...
#define ESP_TDO 15  // ...

// Programeer/download aansluiting
#define ESP_Boot 0  // ...
#define ESP_EN EN   // ...
#define ESP_TX0 1   // ...
#define ESP_RX0 3   // ...

#define UART0_RX 1  // Serial RX
#define UART0_TX 3  // Serial TX

#define CAN_RX 4  // CAN Receive = GPIO04
#define CAN_TX 5  // CAN Transmit = GPIO05

/*******************************************************************
 * GENERAL GPIO pin numbers
 *******************************************************************/
#define EMERGNECY_STOP_PIN 2

#define LED_TWAI_PIN 13
#define LED_HEARTBEAT_PIN 15
#define LED_ERROR_PIN 12

/*******************************************************************
 * LIFT PCF8574 pin numbers
 *******************************************************************/
#define LIFT_ENABLE_PIN 0
#define LIFT_START_HOMING_PIN 2
#define LIFT_MOTOR_UP_PIN 3
#define LIFT_MOTOR_DOWN_PIN 1

/*******************************************************************
 * LIFT ESP32 GPIO
 *******************************************************************/
#define LIFT_LED_UP_PIN 32
#define LIFT_LED_DOWN_PIN 26

#define LIFT_SENSOR_UP_PIN 36
#define LIFT_SENSOR_DOWN_PIN 39
#define LIFT_SENSOR_HOME_PIN 19

#define LIFT_BUTTON_UP_PIN 23
#define LIFT_BUTTON_DOWN_PIN 25

/*******************************************************************
 * AZIMUTH PCF8574 
 *******************************************************************/
#define AZIMUTH_ENABLE_PIN 6
#define AZIMUTH_START_HOMING_PIN 5

/*******************************************************************
 * AZIMUTH MCP4725 
 *******************************************************************/

/*******************************************************************
 * AZIMUTH ESP32 GPIO
 *******************************************************************/
// Analog inputs
#define STEER_WHEEL_ANALOG_CHANNEL A6
#define THOTTLE_ANALOG_CHANNEL A5

// Digital inputs & outputs
#define AZIMUTH_HOME_PIN 19
#define AZIMUTH_ANALOG_ENABLE_PIN 27

/*******************************************************************
 * DMC PCF8574 pin numbers
 *******************************************************************/
#define DMC_ENABLE_PIN 4

/*******************************************************************
 * Application definitions
 *******************************************************************/
#define LINEAR_MIN    0
#define LINEAR_MIDDLE 2048
#define LINEAR_MAX    4096

#endif  // CONFIG_HEADER_ID
