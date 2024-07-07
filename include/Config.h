/*******************************************************************
 *  Definitions and constants
 *
 *******************************************************************/
#ifndef CONFIG_HEADER_ID
#define CONFIG_HEADER_ID

/*******************************************************************
 * Program name and version definitions
 *******************************************************************/
#define ProgramName "EBC-Retractable"   // Program name
#define ProgramVersion "v0.1"           // Program version
#define ProgramTitle "EBC-Retractable"  // Title Web interface
#define WiFiSSIDPrefix "EBC-E"          // Wifi ssid name
#define PCBVersion "v0.31"              // Hardware version
#define HTMLTitlePrefix "RDT"           // HTML page title prefix

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

#define LIFT_MOTOR_UP_PIN 3
#define LIFT_MOTOR_DOWN_PIN 1

/*******************************************************************
 * LIFT ESP32 GPIO
 *******************************************************************/
#define LIFT_LED_UP_PIN 32
#define LIFT_LED_DOWN_PIN 26

#define LIFT_SENSOR_UP_PIN 36
#define LIFT_SENSOR_DOWN_PIN 39

#define LIFT_BUTTON_UP_PIN 23
#define LIFT_BUTTON_DOWN_PIN 25

/*******************************************************************
 * AZIMUTH PCF8574 
 *******************************************************************/
#define AZIMUTH_ENABLE_PIN 0

#define WHEEL_PIN 32

/*******************************************************************
 * AZIMUTH MCP4725 
 *******************************************************************/


/*******************************************************************
 * AZIMUTH ESP32 GPIO
 *******************************************************************/
// #define AZIMUTH_LED_UP_PIN 32
// #define AZIMUTH_LED_DOWN_PIN 26
// #define AZIMUTH_SENSOR_UP_PIN 36
// #define AZIMUTH_SENSOR_DOWN_PIN 39
// #define AZIMUTH_BUTTON_UP_PIN 36
// #define AZIMUTH_BUTTON_DOWN_PIN 39

/*******************************************************************
 * DMC PCF8574 pin numbers
 *******************************************************************/
#define DMC_ENABLE_PIN 4

/*******************************************************************
 * Hardware Implementation definitions
 *******************************************************************/
#define DEBOUNCE_TIME 100

#endif  // CONFIG_HEADER_ID
