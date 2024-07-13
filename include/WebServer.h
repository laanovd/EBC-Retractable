/*******************************************************************
 * EBC-E Webserver
 *
 *******************************************************************/
#ifndef HEADER_WEB_H
#define HEADER_WEB_H

#include <ElegantOTAPro.h>
#include <ArduinoJson.h>

/*******************************************************************
 * JSON fields
 *******************************************************************/
#define JSON_WEBS_FLASH_SIZE "flash-size"
#define JSON_WEBS_FLASH_USED "flash-used"

#define JSON_WEBS_HEAP_TOTAL "heap-total"
#define JSON_WEBS_HEAP_FREE "heap-free"

#define JSON_WEBS_CHIP_ID "chip-id"

/********************************************************************
 * Type defintions
 *********************************************************************/
typedef void (*RestApiHandler1) (AsyncWebServerRequest *request);
typedef void (*RestApiHandler2) (AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);

typedef struct {
  char uri[64];
  char comment[64];
  int instances;
  RestApiHandler1 fn_create;
  RestApiHandler1 fn_read;
  RestApiHandler2 fn_update;
  RestApiHandler1 fn_delete;
} rest_api_t;

extern AsyncWebServer web_server;

/********************************************************************
 * Setup REST API handlers
 *********************************************************************/
extern void setup_uri(rest_api_t *uri_hdl);

/********************************************************************
 * Updates the WebSocket JSON data with the given key-value pair.
 * If the key does not exist in the WebSocket JSON data, it is added with a null value.
 * Then, the key-value pair is stored and sent via WebSocket.
 *
 * @param kv The key-value pair to update.
 *********************************************************************/
extern void WEBSOCKET_update_pair(JsonPair kv);

/********************************************************************
 * Updates the WebSocket with the values from the given JSON document.
 * 
 * @param doc The JSON document containing the values to update.
 *********************************************************************/
extern void WEBSOCKET_update_doc(JsonDocument doc);

/********************************************************************
 * Sends a JSON document over a WebSocket connection.
 * 
 * @param doc The JSON document to send.
 *********************************************************************/
extern void WEBSOCKET_send_doc(JsonDocument doc);

/********************************************************************
 * Setup webserver
 *********************************************************************/
extern void WEBSERVER_setup(void);

#endif // HEADER_WEB_H