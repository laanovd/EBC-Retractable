/*******************************************************************
 * EBC-E Webserver and OTA
 *
 *******************************************************************/
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <ElegantOTAPro.h>
#include <FS.h>
#include <LittleFS.h>
#include <WebSerial.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <WiFi.h>
#include <esp_err.h>

#include "CLI.h"
#include "Config.h"
#include "Debug.h"
#include "Maintenance.h"
#include "Storage.h"
#include "WiFiCom.h"

/********************************************************************
 * Constants
 *********************************************************************/
#define DEFAULT_HTML_PAGE "/web/index.html"

#define WEBSERVER_PORT 80
#define WEBSOCKET_PORT 81

#define DEBUG_WEBSOCKET

/********************************************************************
 * Type definitions
 *********************************************************************/

/********************************************************************
 * Variables
 *********************************************************************/
AsyncWebServer web_server(WEBSERVER_PORT);
WebSocketsServer web_socket_server = WebSocketsServer(WEBSOCKET_PORT);

static bool OTA_Start = false;
static bool OTA_Ready = false;

static String htmlString;

static JsonDocument WebSocket_JSON_data;

static bool WebSocket_JSON_data_push = false;

/********************************************************************
 * Create initial JSON data
 *******************************************************************/
static JsonDocument WEBSERVER_json(void) {
  JsonDocument doc;

  doc[JSON_WEBS_FLASH_SIZE] = (uint32_t)(ESP.getFlashChipSize() / 1024);  // kByte
  doc[JSON_WEBS_FLASH_USED] = (uint32_t)(ESP.getSketchSize() / 1024);     // kByte

  doc[JSON_WEBS_HEAP_TOTAL] = (uint32_t)(ESP.getHeapSize() / 1024);  // kByte
  doc[JSON_WEBS_HEAP_FREE] = (uint32_t)(ESP.getFreeHeap() / 1024);   // kByte

  doc[JSON_WEBS_CHIP_ID] = ChipIds();

  return doc;
}

/********************************************************************
 * Create BMS string
 *******************************************************************/
String WEBSERVER_string(void) {
  JsonDocument doc = WEBSERVER_json();

  String text = "--- WEB-SERVER ---";

  text.concat("\r\nFlash size(kb): ");
  text.concat(doc[JSON_WEBS_FLASH_SIZE].as<int>());
  text.concat(", used(kb): ");
  text.concat(doc[JSON_WEBS_FLASH_USED].as<int>());

  text.concat("\r\nHeap size(kb): ");
  text.concat(doc[JSON_WEBS_HEAP_TOTAL].as<int>());
  text.concat(", free(kb): ");
  text.concat(doc[JSON_WEBS_HEAP_FREE].as<int>());

  text.concat("\r\n");
  return text;
}

/*******************************************************************
 * Get Web page title
 *******************************************************************/
String HTML_title(void) {
  char buf[64];
  snprintf(buf, sizeof(buf), "%s-%s", HTMLTitlePrefix, ChipIds().c_str());
  return String(buf);
}

/********************************************************************
 * Default HTTP handlers
 *********************************************************************/
void default_on_create(AsyncWebServerRequest *request) {
  request->send(501, "text/plain", "501-Not Implemented");
}

void default_on_read(AsyncWebServerRequest *request) {
  request->send(501, "text/plain", "501-Not Implemented");
}

void default_on_update(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  request->send(501, "text/plain", "501-Not Implemented");
}

void default_on_delete(AsyncWebServerRequest *request) {
  request->send(501, "text/plain", "501-Not Implemented");
}

/********************************************************************
 * Setup REST API handlers
 *********************************************************************/
void setup_uri(rest_api_t *uri_hdl) {
  if (!uri_hdl->fn_create)
    uri_hdl->fn_create = default_on_create;
  web_server.on(uri_hdl->uri, HTTP_POST, uri_hdl->fn_create);

  if (!uri_hdl->fn_read)
    uri_hdl->fn_read = default_on_read;
  web_server.on(uri_hdl->uri, HTTP_GET, uri_hdl->fn_read);

  /* UPDATE is only with HTML body data */
  if (!uri_hdl->fn_update)
    uri_hdl->fn_update = default_on_update;
  web_server.on(
      uri_hdl->uri, HTTP_PUT, [](AsyncWebServerRequest *request) {}, NULL, uri_hdl->fn_update);

  if (!uri_hdl->fn_delete)
    uri_hdl->fn_delete = default_on_delete;
  web_server.on(uri_hdl->uri, HTTP_DELETE, uri_hdl->fn_delete);
}

/********************************************************************
 * ElegantOTA Profesional Routines
 *********************************************************************/
static void onOTAStart() {
  DEBUG_info("OTA update started!");
  OTA_Start = true;
  OTA_Ready = false;
}

static void onOTAProgress(size_t current, size_t final) {
  static unsigned long ota_progress_millis = 0;
  if (millis() - ota_progress_millis > 1000) {
    char msg[64];
    ota_progress_millis = millis();
    snprintf(msg, sizeof(msg), "OTA Progress Current: %u bytes, Final: %u bytes\n", (unsigned int)current, (unsigned int) final);
    DEBUG_info(msg);
  }
}

static void onOTAEnd(bool success) {
  // Log when OTA has finished
  if (success)
    DEBUG_info("OTA update finished successfully!");
  else
    DEBUG_info("There was an error during OTA update!");
  OTA_Ready = true;
}

/********************************************************************
 * WebServer main task
 *********************************************************************/
static void WEBSERVER_main_task(void *parameter) {
  (void)parameter;

  while (true) {
    vTaskDelay(200 / portTICK_PERIOD_MS);

    web_socket_server.loop();
  }
}

/********************************************************************
 * WebSocketsServer update
 *********************************************************************/
static void WEBSOCKET_send_pair(JsonPair kv) {
  String value;

  if (kv.value().is<const char *>()) {
    value = "\"" + String(kv.value().is<const char *>()) + "\"";
  } else if (kv.value().is<int>()) {
    value = String(kv.value().as<int>());
  } else if (kv.value().is<float>()) {
    value = String(kv.value().as<float>());
  } else if (kv.value().is<bool>()) {
    value = kv.value().as<bool>() ? String("true") : String("false");
  } else {
    value = "<invalid type>";
  }

  String msg = "{\"" + String(kv.key().c_str()) + "\":" + value + "}";
  web_socket_server.broadcastTXT(msg);
}

static bool WEBSOCKET_store_pair(JsonPair kv) {
  if (WebSocket_JSON_data[kv.key().c_str()] != kv.value()) {
    WebSocket_JSON_data[kv.key().c_str()] = kv.value();
    return true;
  }
  return false;
}

/********************************************************************
 * Updates the WebSocket JSON data with the given key-value pair.
 * If the key does not exist in the WebSocket JSON data, it is added with a null value.
 * Then, the key-value pair is stored and sent via WebSocket.
 *
 * @param kv The key-value pair to update.
 *********************************************************************/
void WEBSOCKET_update_pair(JsonPair kv) {
  if (!WebSocket_JSON_data.containsKey(kv.key().c_str())) {
    WebSocket_JSON_data[kv.key().c_str()] = NULL;
  }

  if (WEBSOCKET_store_pair(kv)) {
    WEBSOCKET_send_pair(kv);
  }
}

/********************************************************************
 * Updates the WebSocket with the values from the given JSON document.
 *
 * @param doc The JSON document containing the values to update.
 *********************************************************************/
void WEBSOCKET_update_doc(JsonDocument doc) {
  for (JsonPair kv : doc.as<JsonObject>()) {
    WEBSOCKET_update_pair(kv);
  }
}

/********************************************************************
 * Sends a JSON document over a WebSocket connection.
 *
 * @param doc The JSON document to send.
 *********************************************************************/
void WEBSOCKET_send_doc(JsonDocument doc) {
  String msg;

  // Store values
  for (JsonPair kv : doc.as<JsonObject>()) {
    WEBSOCKET_store_pair(kv);
  }

  // Send complete doc
  serializeJson(doc, msg);
  web_socket_server.broadcastTXT(msg);
}

/********************************************************************
 * WebSocketsServer task
 *********************************************************************/
static void WEBSOCKET_task(void *parameter) {
  (void)parameter;
  JsonDocument doc;
  String str;

  doc["program_name"] = ProgramName;
  doc["chip_id"] = ChipIds();
  doc["wifi_ssid"] = WiFi_ssid();
  WEBSOCKET_update_doc(doc);

  while (true) {
    if (WebSocket_JSON_data_push) {
      WEBSOCKET_send_doc(WebSocket_JSON_data);
      WebSocket_JSON_data_push = false;
    }

    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

/********************************************************************
 * REST API: read handler
 *********************************************************************/
void WEBSERVER_rest_read(AsyncWebServerRequest *request) {
  String json;
  serializeJson(WEBSERVER_json(), json);
  request->send(200, "application/json", json.c_str());
}

/********************************************************************
 * REST API: update handler
 *
 * Result code:
 * - 200, Ok
 * - 201, successfull created (HTTP_POST)
 * - 204, No content (missing/incorrect fields)
 *
 *
 *********************************************************************/
void WEBSERVER_rest_update(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  (void)len;
  (void)index;
  (void)total;

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, (char *)data);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    request->send(204, "text/plain", "204, No content");
    return;
  }

#ifdef DEBUG_API
  String str;
  Serial.print(F("WEBSERVER REST API received: "));
  serializeJson(doc, str);
  Serial.println(str);
#endif

  // Serial.printf("SYSTEM-on-update: len=%d, index=%d, total=%d\n\r", (int)len, (int)index, (int)total);
  // Serial.println(doc.as<String>());

  if (doc.containsKey("password"))
    Serial.println(doc["password"].as<String>());

  request->send(200, "text/plain", "200, OK");
}

static rest_api_t WEBSERVER_api_handlers = {
    /* uri */ "/api/v1/webserver",
    /* comment */ "WebSerever module",
    /* instances */ 1,
    /* fn_create */ nullptr,
    /* fn_read */ WEBSERVER_rest_read,
    /* fn_update */ WEBSERVER_rest_update,
    /* fn_delete */ nullptr,
};

/********************************************************************
 * WebSocketServer events
 *********************************************************************/
void WebSocketsEvents(byte num, WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {              // switch on the type of information sent
    case WStype_DISCONNECTED:  // if a client is disconnected, then type == WStype_DISCONNECTED
      break;
    case WStype_CONNECTED:  // if a client is connected, then type == WStype_CONNECTED
      WebSocket_JSON_data_push = true;
      break;
    case WStype_TEXT:  // if a client has sent data, then type == WStype_TEXT
#ifdef DEBUG_WEBSOCKET
      for (int i = 0; i < length; i++) {  // print received data from client
        Serial.print((char)payload[i]);
      }
      Serial.println("");
#endif

      if (MAINTENANCE_command_handler((const char *)payload) < 0) {
        Serial.print(F("MAINTENANCE command handler failed!"));
        return;
      }

      break;
  }
}

/********************************************************************
 * Read HTML from file
 *********************************************************************/
static void Load_HTML_Page(fs::FS &fs, const char *path) {
  File file = fs.open(path);
  if (!file || file.isDirectory()) {
    Serial.println("- failed to read HTML page from file.");
    return;
  }

  htmlString = "";
  while (file.available()) {
    htmlString += file.readStringUntil(EOF);
  }
  file.close();
}

/*******************************************************************
 *  Initialize the debug webserver
 *******************************************************************/
static void Setup_HTML_Page_Title(void) {
  int start, end;
  String title = HTML_title();

  // Wijzig de titel van de pagina
  start = htmlString.indexOf("<title>") + sizeof("<title>") - 1;
  end = htmlString.indexOf("</title>");
  htmlString = htmlString.substring(0, start) + title + htmlString.substring(end);
}

/********************************************************************
 *  Initialize the debug webserver
 *********************************************************************/
static void WEBSERVER_init(void) {
  String title = HTML_title();

  Load_HTML_Page(LittleFS, DEFAULT_HTML_PAGE);
  Setup_HTML_Page_Title();

  ElegantOTA.setID(ProgramName);            // Set Hardware ID
  ElegantOTA.setFWVersion(ProgramVersion);  // Set Firmware Version
  ElegantOTA.setTitle(title.c_str());       // Set OTA Webpage Title

  ElegantOTA.begin(&web_server);  // Start ElegantOTA

  // ElegantOTA callbacks
  ElegantOTA.onStart(onOTAStart);        // ...
  ElegantOTA.onProgress(onOTAProgress);  // ...
  ElegantOTA.onEnd(onOTAEnd);            // ...

  // WebSerial is accessible at "<IP Address>/webserial" in browser
  WebSerial.begin(&web_server);
  WebSerial.msgCallback(CLI_webserial_task);

  web_server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(200, "text/html", htmlString); });

  web_server.on("/maintenance", HTTP_GET, [](AsyncWebServerRequest *request) { 
    String html = "/web" + request->url();
    File file = LittleFS.open(html);
    request->send(200, "text/html", file.readStringUntil(EOF)); });

  web_server.begin();  // Start WebServer

  web_socket_server.begin();
  web_socket_server.onEvent(WebSocketsEvents);
}

/********************************************************************
 *  CLI: List storage content
 *******************************************************************/
static void clicb_list_web(cmd *c) {
  Command cmd(c);
  Argument arg = cmd.getArg(0);
  String strArg = arg.getValue();

  /* List settings */
  if (strArg.isEmpty()) {
    CLI_println(WEBSERVER_string());
    return;
  }

  if (strArg.equalsIgnoreCase("data")) {
    serializeJson(WebSocket_JSON_data, strArg);
    CLI_println(strArg);
    return;
  }

  CLI_println("Invalid command: WEB <data>.");
}

/********************************************************************
 * Setup CommandLine handler(s)
 *******************************************************************/
void WEBSERVER_cli_handlers(void) {
  cli.addBoundlessCmd("web", clicb_list_web);
}

/********************************************************************
 *  Initialize tasks
 *
 *********************************************************************/
static void WEBSERVER_setup_tasks(void) {
  xTaskCreate(WEBSERVER_main_task, "WebServer main task", 4096, NULL, 15, NULL);
  xTaskCreate(WEBSOCKET_task, "WebSocketsServer main task", 4096, NULL, 15, NULL);
}

/********************************************************************
 *  Setup webserver
 *
 *********************************************************************/
void WEBSERVER_setup(void) {
  WEBSERVER_init();

  WEBSERVER_setup_tasks();

  WEBSERVER_cli_handlers();
  setup_uri(&WEBSERVER_api_handlers);

  Serial.println(F("WebServer setup completed..."));
}