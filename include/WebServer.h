/********************************************************************
 * EBC-E Webserver
 *
 ********************************************************************/
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

/*********************************************************************
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

/*********************************************************************
 * Setup REST API handlers
 *********************************************************************/
extern void setup_uri(rest_api_t *uri_hdl);

/*********************************************************************
 * Send data to web
 *********************************************************************/
extern void WEBSocket_set(JsonDocument value);

/*********************************************************************
 * Setup webserver
 *********************************************************************/
extern void WEBSERVER_setup(void);

#endif // HEADER_WEB_H