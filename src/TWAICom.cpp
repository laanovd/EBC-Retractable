/********************************************************************************
 *    TWAI.ino
 *
 *    TWAI CAN communication channel
 *
 ********************************************************************************/
#include <Arduino.h>
#include <ArduinoJson.h>
#include <esp_err.h>

#include "Config.h"
#include "Debug.h"
#include "WebServer.h"
#include "CLI.h"

#include "TWAICom.h"

/********************************************************************************
 * Constants
 ********************************************************************************/

/********************************************************************************
 * Type definitions
 ********************************************************************************/

/********************************************************************************
 *  Variables
 ********************************************************************************/
static QueueHandle_t TWAI_TX_QUEUE;
static QueueHandle_t TWAI_RX_QUEUE;

static uint32_t _twai_baudrate = 500000;

static uint32_t _twai_received = 0;
static uint32_t _twai_transmitted = 0;
static uint32_t _twai_error = 0;

static CANReceiveHandler TWAI_receive_handler = nullptr;

/*********************************************************************
 * TWAI state to string
 ********************************************************************/
static String twai_state_to_str(twai_state_t state)
{
    switch (state)
    {
    case TWAI_STATE_STOPPED: /**< Stopped state. The TWAI controller will not participate in any TWAI bus activities */
        return "STOPPED";
    case TWAI_STATE_RUNNING: /**< Running state. The TWAI controller can transmit and receive messages */
        return "RUNNING";
    case TWAI_STATE_BUS_OFF: /**< Bus-off state. The TWAI controller cannot participate in bus activities until it has recovered */
        return "BUS-OFF";
    case TWAI_STATE_RECOVERING: /**< Recovering state. The TWAI controller is undergoing bus recovery */
        return "RECOVERING";
    default:
        return "UNKNONW";
    }
}

/*********************************************************************
 * Create initial JSON data
 ********************************************************************/
static JsonDocument TWAI_json(void)
{
    JsonDocument doc;

    twai_status_info_t info;
    twai_get_status_info(&info);

    doc[JSON_TWAI_DEVICE] = "TWAI";
    doc[JSON_TWAI_STATE] = twai_state_to_str(info.state);

    doc[JSON_TWAI_BAUDRATE] = _twai_baudrate / 1000;

    doc[JSON_TWAI_RX_FRAMES] = _twai_received;
    doc[JSON_TWAI_RX_ERRORS] = info.rx_error_counter;
    doc[JSON_TWAI_RX_MISSED] = info.rx_missed_count;
    doc[JSON_TWAI_RX_OVERRUN] = info.rx_overrun_count;
    doc[JSON_TWAI_RX_QUEUED] = info.msgs_to_rx;

    doc[JSON_TWAI_TX_FRAMES] = _twai_transmitted;
    doc[JSON_TWAI_TX_ERRORS] = info.tx_error_counter;
    doc[JSON_TWAI_TX_FAILED] = info.tx_failed_count;
    doc[JSON_TWAI_TX_QUEUED] = info.msgs_to_tx;

    doc[JSON_TWAI_ARB_LOST] = info.arb_lost_count;
    doc[JSON_TWAI_BUS_ERROR] = info.bus_error_count;

    return doc;
}

/*********************************************************************
 * Create TWAI string
 ********************************************************************/
String TWAI_string(void)
{
    JsonDocument doc = TWAI_json();

    String text = "--- TWAI ---";

    text.concat("\r\nTWAI baudrate: ");
    text.concat(doc[JSON_TWAI_BAUDRATE].as<int>());
    text.concat("kb, State: ");
    text.concat(doc[JSON_TWAI_STATE].as<const char *>());

    text.concat("\r\nReceived: ");
    text.concat(doc[JSON_TWAI_RX_FRAMES].as<int>());
    text.concat(", errors: ");
    text.concat(doc[JSON_TWAI_RX_ERRORS].as<int>());
    text.concat(", missed: ");
    text.concat(doc[JSON_TWAI_RX_MISSED].as<int>());
    text.concat(", overrun: ");
    text.concat(doc[JSON_TWAI_RX_OVERRUN].as<int>());
    text.concat(", queued: ");
    text.concat(doc[JSON_TWAI_RX_QUEUED].as<int>());

    text.concat("\r\nTransmitted: ");
    text.concat(doc[JSON_TWAI_TX_FRAMES].as<int>());
    text.concat(", errors: ");
    text.concat(doc[JSON_TWAI_TX_ERRORS].as<int>());
    text.concat(", failed: ");
    text.concat(doc[JSON_TWAI_TX_FAILED].as<int>());
    text.concat(", queued: ");
    text.concat(doc[JSON_TWAI_TX_QUEUED].as<int>());

    text.concat("\r\nArbitrage lost: ");
    text.concat(doc[JSON_TWAI_ARB_LOST].as<int>());
    text.concat(", bus-errors: ");
    text.concat(doc[JSON_TWAI_BUS_ERROR].as<int>());

    text.concat("\r\n");
    return text;
}

/********************************************************************************
 * TWAI frame has been transmitted
 ********************************************************************************/
bool TWAI_tx_frames(void)
{
    static int memo = 0;
    if (memo != _twai_transmitted)
    {
        memo = _twai_transmitted;
        return true;
    }
    return false;
}

/********************************************************************************
 * COMCOM frame has been received
 *
 ********************************************************************************/
bool TWAI_rx_frames(void)
{
    static int memo = 0;
    if (memo != _twai_received)
    {
        memo = _twai_received;
        return true;
    }
    return false;
}

/*******************************************************************
 * Set receive handler
 *******************************************************************/
void TWAI_rx_handler(CANReceiveHandler handler)
{
    TWAI_receive_handler = handler;
}

/********************************************************************************
 *  Send a frame through CAN channel
 *
 ********************************************************************************/
int TWAI_send(uint32_t id, uint8_t *buffer, uint8_t length, bool rtr, bool extd)
{
    twai_message_t frame;

    frame.identifier = id;
    frame.extd = extd ? 1 : 0;
    frame.rtr = rtr ? 1 : 0;
    frame.data_length_code = length;
    memcpy(frame.data, buffer, length);

    if (xQueueSend(TWAI_TX_QUEUE, &frame, 0) != pdPASS)
    {
        // Serial.println(F("TWAI transmit error queueing frame..."));
        return ESP_FAIL;
    }
    return ESP_OK;
}

/********************************************************************************
 *  Setup the TWAI transmitting frame
 *
 ********************************************************************************/
void TWAI_transmit_task(void *parameter)
{
    twai_message_t frame;
    int error;
    (void)parameter;

    while (1)
    {
        if (xQueueReceive(TWAI_TX_QUEUE, &frame, 0) == pdPASS)
        {
            DEBUG_can("TWAI-tx: ", frame.data_length_code, frame.identifier, 0, frame.data);
            _twai_transmitted++;

            _twai_error = twai_transmit(&frame, portMAX_DELAY);
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

/********************************************************************************
 *  Setup the TWAI receiving frame
 *
 ********************************************************************************/
void TWAI_receive_task(void *parameter)
{
    twai_message_t frame;
    (void)parameter;

    while (true)
    {
        if (twai_receive(&frame, 0) == ESP_OK)
        {
            _twai_received++;

            if (xQueueSend(TWAI_RX_QUEUE, &frame, 0) != pdPASS)
            {
                Serial.println(F("TWAI error queueing received frame..."));
                vTaskDelay(500 / portTICK_PERIOD_MS);
            }
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

/*******************************************************************
 *  TWAI main task
 *******************************************************************/
void TWAI_main_task(void *parameter)
{
    twai_message_t frame;
    (void)parameter;

    while (1)
    {
        if (xQueueReceive(TWAI_RX_QUEUE, &frame, 0) == pdPASS)
        {
            _twai_received++;

#ifdef DEBUG_FRAMES
            Serial.printf("TWAI frame received with id 0x%04x.\n\r", (int)frame.identifier);
#endif

            if (TWAI_receive_handler)
                TWAI_receive_handler(frame.identifier, frame.data, frame.data_length_code, frame.rtr ? true : false, frame.extd ? true : false);
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

/***********************************************************************************************************************
 *   Define your CAN messages here, OR you can define them locally...
 *   standard 11-bit frame ID = 0
 *   extended 29-bit frame ID = 1
 *   format: can_message_t (name of your message) = {std/ext/Req frame, message ID, message DLC, {data bytes here}};
 *
 ***********************************************************************************************************************/
static esp_err_t TWAI_setup_driver(int mode)
{
    twai_general_config_t general_config = {
        .mode = TWAI_MODE_NORMAL,
        .tx_io = (gpio_num_t)GPIO_NUM_5,
        .rx_io = (gpio_num_t)GPIO_NUM_4,
        .clkout_io = (gpio_num_t)TWAI_IO_UNUSED,
        .bus_off_io = (gpio_num_t)TWAI_IO_UNUSED,
        .tx_queue_len = 0, // NO QueQue
        .rx_queue_len = 65,
        .alerts_enabled = TWAI_ALERT_NONE, // geen alert berichten
                                           //.alerts_enabled = TWAI_ALERT_AND_LOG, // Stuurt CANBUS Alert berichten via Debug poort
        .clkout_divider = 0,
        .intr_flags = 1,
    };

    twai_timing_config_t timing_config;
    switch (mode)
    {
    case CAN_500KB:
        _twai_baudrate = 500000;
        timing_config = TWAI_TIMING_CONFIG_500KBITS();
        CLI_println(F("Setup TWAI with 500kbps."));
        break;

    case CAN_250KB:
    default:
        _twai_baudrate = 250000;
        timing_config = TWAI_TIMING_CONFIG_250KBITS();
        CLI_println(F("Setup TWAI with 250kbps."));
        break;
    }

    twai_filter_config_t filter_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    esp_err_t error = twai_driver_install(&general_config, &timing_config, &filter_config);
    if (error != ESP_OK)
    {
        Serial.printf("TWAI driver installation failed, (errno. %d).\n\r", (int)error);
        return ESP_FAIL;
    }

    // start TWAI driver
    error = twai_start();
    if (error != ESP_OK)
    {
        Serial.printf("TWAI driver start failed, (errno. %d).\n\r", (int)error);
        return ESP_FAIL;
    }

    CLI_println(F("TWAI setup completed..."));
    return ESP_OK;
}

/*********************************************************************
 * REST API: read handler
 *********************************************************************/
void TWAI_rest_read(AsyncWebServerRequest *request)
{
    String str;
    serializeJson(TWAI_json(), str);
    request->send(200, "application/json", str.c_str());
}

static rest_api_t TWAI_api_handlers = {
    /* uri */ "/api/v1/twai",
    /* comment */ "TWAI module",
    /* instances */ 1,
    /* fn_create */ nullptr,
    /* fn_read */ TWAI_rest_read,
    /* fn_update */ nullptr,
    /* fn_delete */ nullptr,
};

/*********************************************************************
 * TWAI: List content
 ********************************************************************/
static void clicb_list_twai(cmd *c)
{
    CLI_println(TWAI_string());
}

/*********************************************************************
 * Setup TWAI handlers
 ********************************************************************/
static void TWAI_cli_handlers(void)
{
    cli.addCommand("twai", clicb_list_twai);
}

/********************************************************************************
 *  Setup TWAI tasks
 *
 ********************************************************************************/
void TWAI_setup_queues()
{
    TWAI_TX_QUEUE = xQueueCreate(10, sizeof(twai_message_t)); // Transmit queue
    TWAI_RX_QUEUE = xQueueCreate(10, sizeof(twai_message_t)); // Receive queue
}

/********************************************************************************
 *  Setup TWAI tasks
 *
 ********************************************************************************/
void TWAI_setup_tasks()
{
    xTaskCreate(TWAI_main_task, "TWAI main task", 4096, NULL, 3, NULL);
    xTaskCreate(TWAI_receive_task, "TWAI receive task", 2048, NULL, 5, NULL);
    xTaskCreate(TWAI_transmit_task, "TWAI transmit task", 2048, NULL, 5, NULL);
}

/********************************************************************************
 *  Setup TWAICom
 *
 ********************************************************************************/
void TWAI_setup(int mode)
{
    TWAI_setup_queues();

    if (TWAI_setup_driver(mode) != ESP_OK)
    {
        Serial.println(F("TWAI setup aborted."));
        return;
    }

    TWAI_setup_tasks();

    TWAI_cli_handlers();
    setup_uri(&TWAI_api_handlers);
}