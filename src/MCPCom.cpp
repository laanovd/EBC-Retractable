/*******************************************************************
 *    MCP.ino
 *
 *    Handling MCP communication, and call frame handlers
 *
 *******************************************************************/
#include <Arduino.h>
#include <ArduinoJson.h>
#include <mcp2515_can.h>

#include "Config.h"
#include "Debug.h"
#include "WebServer.h"
#include "CLI.h"

#include "MCPCom.h"

/*******************************************************************
 *  Constants
 *
 *******************************************************************/
#undef DEBUG_TASKS
#undef DEBUG_FRAMES

/*******************************************************************
 * Type definitions
 *******************************************************************/
typedef struct mcp_frame_t {
    uint32_t id;          /**< 11 or 29 bit identifier */
    uint8_t ext;          /** ext flag */
    uint8_t rtr;          /** rtr flag */
    uint8_t length;       /**< Data length code */
    uint8_t buffer[8];    /**< Data bytes (not relevant in RTR frame) */
}  mcp_frame_t;

/*******************************************************************
 *  Variables
 *******************************************************************/
static QueueHandle_t MCP_TX_QUEUE;
static QueueHandle_t MCP_RX_QUEUE;

static uint32_t _mcp1_baudrate = 500000;

static uint32_t _mcp1_transmited = 0;
static uint32_t _mcp1_transmited_error = 0;
static uint32_t _mcp1_received = 0;
static uint32_t _mcp1_received_error = 0;

static CANReceiveHandler MCP_receive_handler = nullptr;

/*******************************************************************
 *  Globals
 *******************************************************************/
mcp2515_can CAN1(SPI0_CS);  // Set CAN-1 CS to pin 15

/*********************************************************************
 * Create initial JSON data
 ********************************************************************/
static JsonDocument MCP_json(void) {
  JsonDocument doc;

  doc[JSON_MCP_DEVICE] = "mcp2515";

  doc[JSON_MCP_BAUDRATE] = _mcp1_baudrate / 1000;

  doc[JSON_MCP_RX_FRAMES] = _mcp1_received;
  doc[JSON_MCP_RX_ERRORS] = _mcp1_received_error;

  doc[JSON_MCP_TX_FRAMES] = _mcp1_transmited;
  doc[JSON_MCP_TX_ERRORS] = _mcp1_transmited_error;

  return doc;
}

/*********************************************************************
 * Create WiFi string
 ********************************************************************/
String MCP_string(void) {
  JsonDocument doc = MCP_json();

  String text = "--- MCP ---";
  text.concat("\r\nTWAI baudrate: ");
  text.concat(doc[JSON_MCP_BAUDRATE].as<int>());

  text.concat("\r\nReceived: ");
  text.concat(doc[JSON_MCP_RX_FRAMES].as<int>());
  text.concat(", errors: ");
  text.concat(doc[JSON_MCP_RX_ERRORS].as<int>());
  
  text.concat("\r\nTransmitted: ");
  text.concat(doc[JSON_MCP_TX_FRAMES].as<int>());
  text.concat(", errors: ");
  text.concat(doc[JSON_MCP_TX_ERRORS].as<int>());

  text.concat("\r\n");
  return text;  
}

/*******************************************************************
 *  MCP frames transmitted
 *******************************************************************/
bool MCP_tx_frames(void) {
  static int memo = 0;
  if (memo < _mcp1_transmited) {
    memo = _mcp1_transmited;
    return true;
  }
  return false;
}

/*******************************************************************
 *  MCP frames received
 *******************************************************************/
bool MCP_rx_frames(void) {
  static int memo = 0;
  if (memo < _mcp1_received) {
    memo = _mcp1_received;
    return true;
  }
  return false;
}

/*******************************************************************
 *  MCP check errors
 * - Receive Error Counter (REC)
 * - Transmit Error Counter (TEC)
 * - Message Assembly Buffer (MAB)
 * - The remaining two receive buffers, called RXB0 and RXB1, can receive a
 *    complete message from the protocol engine via the MAB
 *
 * - REGISTER 6-3: EFLG: ERROR FLAG REGISTER (ADDRESS: 2Dh)
 *    bit 0 EWARN: Error Warning Flag bit
 *      - Sets when TEC or REC is equal to or greater than 96 (TXWAR or RXWAR = 1)
 *      - Resets when both REC and TEC are less than 96
 *    bit 1 RXWAR: Receive Error Warning Flag bit
 *      - Sets when REC is equal to or greater than 96
 *      - Resets when REC is less than 96
 *    bit 2 TXWAR: Transmit Error Warning Flag bit
 *      - Sets when TEC is equal to or greater than 96
 *      - Resets when TEC is less than 96
 *    bit 3 RXEP: Receive Error-Passive Flag bit
 *      - Sets when REC is equal to or greater than 128
 *      - Resets when REC is less than 128
 *    bit 4 TXEP: Transmit Error-Passive Flag bit
 *      - Sets when TEC is equal to or greater than 128
 *      - Resets when TEC is less than 128
 *    bit 5 TXBO: Bus-Off Error Flag bit
 *      - Sets when TEC reaches 255
 *      - Resets after a successful bus recovery sequence
 *    bit 6 RX0OVR: Receive Buffer 0 Overflow Flag bit
 *      - Sets when a valid message is received for RXB0 and RX0IF (CANINTF[0]) = 1
 *      - Must be reset by MCU
 *    bit 7 RX1OVR: Receive Buffer 1 Overflow Flag bit
 *      - Sets when a valid message is received for RXB1 and RX1IF (CANINTF[1]) = 1
 *      - Must be reset by MCU
 *
 *******************************************************************/
#define CAN_EWARN (1 << 0)   // EWARN: Error Warning Flag bit
#define CAN_RXWAR (1 << 1)   // RXWAR: Receive Error Warning Flag bit
#define CAN_TXWAR (1 << 2)   // TXWAR: Transmit Error Warning Flag bit
#define CAN_RXEP (1 << 3)    // RXEP: Receive Error-Passive Flag bit
#define CAN_TXEP (1 << 4)    // TXEP: Transmit Error-Passive Flag bit
#define CAN_TXBO (1 << 5)    // TXBO: Bus-Off Error Flag bit
#define CAN_RX0OVR (1 << 6)  // RX0OVR: Receive Buffer 0 Overflow Flag bit
#define CAN_RX1OVR (1 << 7)  // RX1OVR: Receive Buffer 1 Overflow Flag bit

static void MCP_check_errors(void) {
  String msg;
  uint8_t error;

  if (CAN1.checkError(&error) != CAN_OK) {
    msg = "MCP error flags: ";
    if (error & CAN_EWARN)
      msg += "EWARN (Error Warning Flag bit) ";
    if (error & CAN_RXWAR)
      msg += "RXWAR (Receive Error Warning Flag bit) ";
    if (error & CAN_TXWAR)
      msg += "TXWAR (Transmit Error Warning Flag bit) ";
    if (error & CAN_RXEP)
      msg += "RXEP (Receive Error-Passive Flag bit) ";
    if (error & CAN_TXEP)
      msg += "TXEP (Transmit Error-Passive Flag bit) ";
    if (error & CAN_TXBO)
      msg += "TXBO (Bus-Off Error Flag bit) ";
    if (error & CAN_TXBO)
      msg += "TXBO (Bus-Off Error Flag bit) ";
    if (error & CAN_RX0OVR)
      msg += "RX0OVR (Receive Buffer 0 Overflow Flag bit) ";
    if (error & CAN_RX1OVR)
      msg += "RX1OVR (Receive Buffer 1 Overflow Flag bit) ";

    Serial.println(msg);
  }
}

/*******************************************************************
 * Set receive handler
 *******************************************************************/
void MCP_rx_handler(CANReceiveHandler handler) {
  MCP_receive_handler = handler;
}

/*******************************************************************
 *  Send a frame through MCP CAN channel
 *******************************************************************/
int MCP_send(uint32_t id, uint8_t* buffer, uint8_t length, bool rtr, bool ext) {
  mcp_frame_t frame;

  frame.id = id & 0x0FFFFFFF;
  frame.ext = ext ? 1 : 0;
  frame.rtr = rtr ? 1 : 0;
  frame.length = length;

  if (buffer && length) {
    memcpy(frame.buffer, buffer, length);
  }

#ifdef DEBUG_FRAMES
  Serial.printf("MCP[tx]: Queue frame with id=0x%08X, length:%d, ext=%d, rtr=%d.\n\r", (int)frame.id, (int)frame.length, (int)frame.ext, (int)frame.rtr);
#endif

  if (xQueueSend(MCP_TX_QUEUE, &frame, 0) != pdPASS) {
    Serial.println(F("MCP error queueing frame..."));
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
  return ESP_OK;
}

/*******************************************************************
 *  Setup Serial Paralel interface (SPI)
 *
 *******************************************************************/
static void MCP_setup_SPI(void) {
  digitalWrite(RST_MCP, LOW);           // RESET MCP2515
  vTaskDelay(1 / portTICK_PERIOD_MS);   // 1 mSec
  digitalWrite(RST_MCP, HIGH);          // RESET opgeheven MCP2515
  vTaskDelay(10 / portTICK_PERIOD_MS);  // 10 mSec

  // Start SPI Poort
  SPI.begin(SCK, MISO, MOSI, SPI0_CS);  // SCLK, MISO, MOSI, Slave Select
}

/*******************************************************************
 * Setup MCP driver
 *******************************************************************/
static esp_err_t MCP_setup_driver(int mode) {
  int result;

  switch (mode) {
    case CAN_500KB:
      _mcp1_baudrate = 500000;
      result = CAN1.begin(CAN_500KBPS, MCP_16MHz);
      CLI_println("Setup MCP with 500kbps.");
      break;

    case CAN_250KB:
    default:
      _mcp1_baudrate = 250000;
      result = CAN1.begin(CAN_250KBPS, MCP_16MHz);
      CLI_println("Setup MCP with 250kbps.");
      break;
  }

  if (result == CAN_OK) {
    CAN1.setMode(MODE_NORMAL);  // Set operation mode to normal so the MCP2515 sends acks to received data.
    pinMode(SPI0_INT, INPUT);   // Configuring pin for /INT input
    Serial.println(F("SUCCESS initialisation MCP2515 driver."));
    return ESP_OK;  // Success
  }

  Serial.printf("Initialisation MCP2515 driver failed, (errno. %d).", (int)result);
  return ESP_FAIL;  // Failed
}

/*******************************************************************
 *  Setup the MCP transmitting frame
 *
 *******************************************************************/
void MCP_transmit_task(void* parameter) {
  mcp_frame_t frame;
  (void)parameter;

  while (1) {
    if (xQueueReceive(MCP_TX_QUEUE, &frame, 0) == pdPASS) {
#ifdef DEBUG_FRAMES
      Serial.printf("MCP[tx]: id=0x%08X, length:%d, ext=%d, rtr=%d.\n\r", (int)frame.id, (int)frame.length, (int)frame.ext, (int)frame.rtr);
#endif
      DEBUG_can("MCP[tx]: ", frame.length, frame.id, frame.rtr ? 1 : 0, frame.buffer);

      if (CAN1.sendMsgBuf((unsigned long)frame.id, frame.ext ? 1 : 0, frame.rtr ? 1 : 0, frame.length, frame.buffer) != CAN_OK) {
        MCP_check_errors();
      }
      else {
        _mcp1_transmited++;
      }

#ifdef DEBUG_TASKS
      Serial.println(F("Success sending MCP CAN frame..."));
#endif
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

/*******************************************************************
 *  Setup the MCP receiving frame
 *
 *******************************************************************/
void MCP_receive_task(void* parameter) {
  mcp_frame_t frame;
  (void)parameter;

  while (true) {
    if (CAN1.readMsgBuf(&frame.length, frame.buffer) == CAN_OK) {
      _mcp1_received++;

      frame.id = CAN1.getCanId();
      frame.ext = CAN1.isExtendedFrame() ? 1 : 0;
      frame.rtr = CAN1.isRemoteRequest() ? 1 : 0;

#ifdef DEBUG_FRAMES
      Serial.printf("MCP[rx]: id=0x%08X, length:%d, ext=%d, rtr=%d.\n\r", (int)frame.id, (int)frame.length, (int)frame.ext, (int)frame.rtr);
#endif

      if (xQueueSend(MCP_RX_QUEUE, &frame, 0) != pdPASS) {
        Serial.println(F("MCP error queueing received frame..."));
        vTaskDelay(500 / portTICK_PERIOD_MS);
        continue;
      }
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

/*******************************************************************
 *  MCP main task
 *******************************************************************/
void MCP_main_task(void* parameter) {
  mcp_frame_t frame;
  (void)parameter;

  while (1) {
    if (xQueueReceive(MCP_RX_QUEUE, &frame, 0) == pdPASS) {
      _mcp1_received++;

    #ifdef DEBUG_FRAMES
      Serial.printf("MCP[rx]: Process frame with id=0x%08X, length:%d, ext=%d, rtr=%d.\n\r", (int)frame.id, (int)frame.length, (int)frame.ext, (int)frame.rtr);
    #endif

      if (MCP_receive_handler)
        MCP_receive_handler(frame.id, frame.buffer, frame.length, frame.rtr ? true : false, frame.ext ? true : false);
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

/*********************************************************************
 * REST API: read handler
 *********************************************************************/
void MCP_rest_read(AsyncWebServerRequest* request) {
  String str;
  serializeJson(MCP_json(), str);
  request->send(200, "application/json", str.c_str());
}

static rest_api_t MCP_api_handlers = {
    /* uri */ "/api/v1/mcp",
    /* comment */ "MCP module",
    /* instances */ 1,
    /* fn_create */ nullptr,
    /* fn_read */ MCP_rest_read,
    /* fn_update */ nullptr,
    /* fn_delete */ nullptr,
};

/*********************************************************************
 *  CLI: List storage content
 ********************************************************************/
static void clicb_list_mcp1(cmd* c) {
  (void)c;
  CLI_println(MCP_string());
}

/*********************************************************************
 *  Setup storage command handlers
 *
 ********************************************************************/
static void MCP_cli_handlers(void) {
  cli.addCommand("mcp", clicb_list_mcp1);
}

/*******************************************************************
 *  Setup MCP tasks
 *
 *******************************************************************/
static void MCP_setup_queues() {
  MCP_TX_QUEUE = xQueueCreate(10, sizeof(mcp_frame_t)); // Transmit queue
  MCP_RX_QUEUE = xQueueCreate(10, sizeof(mcp_frame_t)); // Receive queue
}

/*******************************************************************
 *  Setup MCP tasks
 *
 *******************************************************************/
static void MCP_setup_tasks() {
  xTaskCreate(MCP_main_task, "MCP main task", 4096, NULL, 3, NULL);
  xTaskCreate(MCP_receive_task, "MCP receive task", 4096, NULL, 5, NULL);
  xTaskCreate(MCP_transmit_task, "MCP transmit task", 2048, NULL, 5, NULL);
}

/*******************************************************************
 * @brief Initializes the MCP module.
 * 
 * This function sets up the queues and SPI for the MCP module. It then initializes the MCP driver
 * based on the specified mode. If the initialization fails, an error message is printed and the function
 * returns. Otherwise, a success message is printed.
 * 
 * @param mode The mode for MCP initialization.
 *******************************************************************/
void MCP_init(int mode) {
  MCP_setup_queues();
  MCP_setup_SPI();

  if (MCP_setup_driver(mode) != ESP_OK) {
    Serial.println(F("MCP setup driver failed..."));
    Serial.println(F("MCP initialisation aborted..."));
    return;
  }

  Serial.println(F("MCP initialized..."));
}

/*******************************************************************
 * @brief Starts the MCP module.
 * 
 * This function initializes the MCP module by setting up tasks, 
 * CLI handlers, and API handlers. It also prints a message to the 
 * serial monitor indicating that the MCP has started.
 *******************************************************************/
void MCP_start(void) {
  MCP_setup_tasks();

  MCP_cli_handlers();
  setup_uri(&MCP_api_handlers);

  Serial.println(F("MCP started..."));
}
