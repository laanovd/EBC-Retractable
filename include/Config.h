/*******************************************************************
 *  Definitions and constants
 *
 *******************************************************************/
#ifndef CONFIG_HEADER_ID
#define CONFIG_HEADER_ID

/*******************************************************************
 * Program name and version definitions
 *******************************************************************/
#define ProgramName "EBC-Base-Program"  // Program name
#define ProgramVersion "v0.1"           // Program version
#define ProgramTitle "EBC-Base-program" // Title Web interface
#define WiFiSSIDPrefix "EBC-E"          // Wifi ssid name
#define PCBVersion "v0.31"              // Hardware version

/*******************************************************************
 * CAN receive handler
 *******************************************************************/
typedef void (*CANReceiveHandler) (uint32_t id, uint8_t *buffer, uint8_t size, bool rtr, bool ext);

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

// Hardware Aansluitingen Interface
#define HeartBeat_LedPin 2  // Hearbeat led
#define Error_LedPin 27     // Power/error led
#define TWAI_LedPin 26      // NMEA2000 communication led
#define MCP_LedPin 33       // MCP communication led
#define VED_LedPin 32       // VEDirect communication led

#define UART0_RX 1  // Serial RX
#define UART0_TX 3  // Serial TX

#define CAN_RX 4  // CAN Receive = GPIO04
#define CAN_TX 5  // CAN Transmit = GPIO05

#define BUTTON_UP_PIN 23
#define BUTTON_UP_LED_PIN 14
#define BUTTON_DOWN_PIN 25
#define BUTTON_DOWN_LED_PIN 26

#define IN_1_PIN 36
#define IN_2_PIN 39
#define IN_3_PIN 34
#define IN_4_PIN 35

#define DMC_ENABLE_PIN 16
#define RETRACT_ENABLE_PIN 17
#define MOTOR_UP_PIN 18
#define MOTOR_DOWN_PIN 19

#define WHEEL_PIN 32
#define ENABLE_ANALOG_OUT_PIN 27

/*******************************************************************
 * Hardware Implementation definitions
 *******************************************************************/
#define DEBOUNCE_TIME 100

#endif  // CONFIG_HEADER_ID
