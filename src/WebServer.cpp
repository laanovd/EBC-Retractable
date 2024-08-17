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

#undef DEBUG_WEBSOCKET
#undef DEBUG_WEBSERVER

/********************************************************************
 * Type definitions
 *********************************************************************/

/********************************************************************
 * Variables
 *********************************************************************/
AsyncWebServer web_server(WEBSERVER_PORT);
WebSocketsServer web_socket_server = WebSocketsServer(WEBSOCKET_PORT);

static JsonDocument WebSocket_JSON_data;

static int ota_restart_countdown = 0;

static int websocket_client_connected = 0;

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
  Serial.print("\nOTA update started...\n");
}

static void onOTAProgress(size_t current, size_t final) {
  static unsigned long ota_progress_millis = 0;
  if (millis() - ota_progress_millis > 500) {
    ota_progress_millis = millis();
    Serial.printf("\rOTA update progress: %u bytes, Final: %u bytes", (unsigned int)current, (unsigned int) final);
  }
}

static void onOTAEnd(bool success) {
  if (success) {
    Serial.println("\nOTA update finished...");
  } else {
    Serial.println("\nOTA update failed...");
  }
  ota_restart_countdown = 12;  // Short delay, in main-task-cycles
}

/********************************************************************
 * Main task
 *********************************************************************/
extern void all_stop(void);

static void WEBSERVER_task(void *parameter) {
  (void)parameter;

  while (true) {
    web_socket_server.loop();

    if (ota_restart_countdown > 0) {
      ota_restart_countdown--;
      if (ota_restart_countdown == 0) {
        Serial.println("\nRestarting system after OTA update...\n");
        all_stop();  // Stop al modules

        vTaskDelay(1000 / portTICK_PERIOD_MS);
        ESP.restart();
      }
    }

    vTaskDelay(250 / portTICK_PERIOD_MS);
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

/********************************************************************
 * Stores a key-value pair in the WebSocket JSON data if
 * the value has changed.
 *********************************************************************/
static bool WEBSOCKET_store_pair(JsonPair kv) {
  if (!WebSocket_JSON_data.containsKey(kv.key().c_str())) {
    WebSocket_JSON_data[kv.key().c_str()] = NULL;
  }

  if (WebSocket_JSON_data[kv.key().c_str()] != kv.value()) {
    WebSocket_JSON_data[kv.key().c_str()] = kv.value();
    return true;
  }
  return false;
}

/********************************************************************
 * Updates the WebSocket JSON data with the given key-value pair.
 * If the key does not exist in the WebSocket JSON data, it is
 * added with a null value. * Then, the key-value pair is stored
 * and sent via WebSocket.
 *
 * @param kv The key-value pair to update.
 *********************************************************************/
void WEBSOCKET_update_pair(JsonPair kv) {
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
 * Sends a JSON key-value pair to the WebSocket connection.
 *
 * @param key The key of the JSON pair to push.
 * @param doc The JSON document containing the key-value pairs.
 *********************************************************************/
void WEBSOCKET_send(String key, JsonDocument doc) {
  String msg;

  // Store values
  for (JsonPair kv : doc.as<JsonObject>()) {
    if (String(kv.key().c_str()) == key) {
      WEBSOCKET_send_pair(kv);
      break;
    }
  }
}

/********************************************************************
 * WebSockets on connect
 *********************************************************************/
static void WEBSOCKET_on_connect(void) {
  WEBSOCKET_send_doc(WebSocket_JSON_data);
  JsonDocument doc;

  //  General program data
  doc["program_name"] = ProgramName;
  doc["chip_id"] = ChipIds();
  doc["wifi_ssid"] = WiFi_ssid();

  WEBSOCKET_send_doc(doc);
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
#ifdef DEBUG_WEBSOCKET
      Serial.println("Websocket client disconnected.");
#endif
      MAINTENANCE_disable();
      break;

    case WStype_CONNECTED:  // if a client is connected, then type == WStype_CONNECTED
#ifdef DEBUG_WEBSOCKET
      Serial.println("Websocket client connected.");
#endif
      MAINTENANCE_disable();
      WEBSOCKET_on_connect();
      break;

    case WStype_TEXT:  // if a client has sent data, then type == WStype_TEXT
#ifdef DEBUG_WEBSOCKET
      Serial.print("Cmd handler: ");
      for (int i = 0; i < length; i++) {
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
static String load_file(fs::FS &fs, const char *path) {
  File file = fs.open(path);
  String content = "";

  if (!file || file.isDirectory()) {
    Serial.println("- failed to read HTML page from file.");
    return "";
  }

  content = file.readStringUntil(EOF);
  file.close();

  return content;
}

/********************************************************************
 * Replaces a placeholder in an HTML page with a given value.
 *
 * @param page The HTML page as a string.
 * @param placeholder The placeholder to be replaced.
 * @param value The value to replace the placeholder with.
 *
 * @return The modified HTML page with the placeholder replaced.
 *********************************************************************/
static void html_replace_placeholder(String &page, String placeholder, String value) {
  int start = page.indexOf(placeholder);
  if (start >= 0) {
    int end = start + placeholder.length();
    page = page.substring(0, start) + value + page.substring(end);
  }
}

/********************************************************************
 * @brief Replaces a placeholder in an HTML page with the program name.
 *
 * This function takes an HTML page as input and replaces a specific placeholder
 * with the program name. The placeholder is defined as "<span id=\"program_name\">-</span>".
 *
 * @param page The HTML page to modify.
 * @return The modified HTML page with the program name inserted.
 *********************************************************************/
static void html_set_program_name(String &page) {
#define PROGRAM_NAME_PLACEHOLDER "<span id=\"program_name\">-</span>"
  html_replace_placeholder(page, PROGRAM_NAME_PLACEHOLDER, ProgramName);
}

/********************************************************************
 * @brief Replaces a placeholder in an HTML page with the program version.
 *
 * This function takes an HTML page as input and replaces a specific placeholder
 * with the program version. The placeholder is defined as "<span id=\"program_version\">-</span>".
 *
 * @param page The HTML page to modify.
 * @return The modified HTML page with the program version inserted.
 *********************************************************************/
static void html_set_program_version(String &page) {
#define PROGRAM_VERSION_PLACEHOLDER "<span id=\"program_version\">-</span>"
  html_replace_placeholder(page, PROGRAM_VERSION_PLACEHOLDER, ProgramVersion);
}

/********************************************************************
 * @brief Replaces a placeholder in an HTML page with the chip ID.
 *
 * This function takes an HTML page as input and replaces a specific placeholder
 * with the chip ID. The chip ID is obtained from the `ProgramVersion` variable.
 *
 * @param page The HTML page to modify.
 * @return The modified HTML page with the chip ID inserted.
 *********************************************************************/
static void html_set_chip_id(String &page) {
#define CHIP_ID_PLACEHOLDER "<span id=\"chip_id\">-</span>"
  html_replace_placeholder(page, CHIP_ID_PLACEHOLDER, ChipIds());
}

/********************************************************************
 * @brief Replaces a placeholder in an HTML page with the WiFi SSID.
 *
 * This function takes an HTML page as input and replaces a specific placeholder
 * with the current WiFi SSID. The placeholder is defined as "<span id=\"wifi_ssid\">-</span>".
 *
 * @param page The HTML page to modify.
 * @return The modified HTML page with the WiFi SSID inserted.
 *********************************************************************/
static void html_set_wifi_ssid(String &page) {
#define WIFI_SSID_PLACEHOLDER "<span id=\"wifi_ssid\">-</span>"
  html_replace_placeholder(page, WIFI_SSID_PLACEHOLDER, WiFi_ssid());
}

/********************************************************************
 * @brief Replaces a placeholder in an HTML page with the current IP address.
 *
 * This function takes an HTML page as input and replaces a specific placeholder
 * with the current IP address obtained from the WiFi module. The placeholder is
 * defined as "<span id=\"ip_address\">-</span>". The modified HTML page is then
 * returned as output.
 *
 * @param page The input HTML page.
 * @return The modified HTML page with the IP address placeholder replaced.
 *********************************************************************/
static void html_set_ip_address(String &page) {
#define WIFI_IP_ADDRESS_PLACEHOLDER "<span id=\"ip_address\">-</span>"
  html_replace_placeholder(page, WIFI_IP_ADDRESS_PLACEHOLDER, WiFi_ip());
}

/********************************************************************
 * @brief Replaces the placeholder in the given HTML page with the
 * title generated from HTML_title().
 *
 * @param page The HTML page to modify.
 * @return The modified HTML page with the updated title.
 *********************************************************************/
static void html_set_page_title(String &page) {
#define PLACEHOLDER_PAGE_TITLE "<title>...</title>"
  String title = "<title>" + HTML_title() + "</title>";
  html_replace_placeholder(page, PLACEHOLDER_PAGE_TITLE, title);
}

/********************************************************************
 * @brief Retrieves a file from the web server.
 *
 * This function is responsible for handling requests to retrieve files
 * from the web server. It takes an `AsyncWebServerRequest` object as a parameter.
 *
 * The function constructs the file path based on the request URL and checks
 * if the file exists. If the file does not exist, it sends a 404 response.
 *
 * If the file exists, it determines the file type based on the file extension
 * and sends the file content as a response with the appropriate content type.
 *
 * @param request The request object representing the client's request.
 *********************************************************************/
static void WEBSERVER_get_file(AsyncWebServerRequest *request) {
  String filename = "/web" + request->url();
  if (filename.endsWith("/")) {
    filename += "index.html";
  }
#ifdef DEBUG_WEBSERVER
  Serial.printf("Webserver load page: [%s]\n", filename.c_str());
#endif

  String content = load_file(LittleFS, filename.c_str());
  if (content.isEmpty()) {
    request->send(404, "text/plain", "404-Not Found");
    return;
  }

  String filetype;
  if (filename.endsWith(".css")) {
    filetype = "text/css";
  } else if (filename.endsWith(".js")) {
    filetype = "text/javascript";
  } else {
    filetype = "text/html";
    html_set_page_title(content);
    html_set_program_name(content);
    html_set_program_version(content);
    html_set_chip_id(content);
    html_set_wifi_ssid(content);
    html_set_ip_address(content);
  }

  request->send(200, filetype, content);
}

/********************************************************************
 *  Initialize the debug webserver
 *********************************************************************/
static void WEBSERVER_init(void) {
  String title = HTML_title();

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

  // WebServer main page
  web_server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    WEBSERVER_get_file(request);
  });

  // WebServer maintenance mode
  web_server.on("/maintenance", HTTP_GET, [](AsyncWebServerRequest *request) {
    WEBSERVER_get_file(request);
  });

  // WebServer system mode
  web_server.on("/system", HTTP_GET, [](AsyncWebServerRequest *request) {
    WEBSERVER_get_file(request);
  });

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
 *********************************************************************/
static void WEBSERVER_setup_tasks(void) {
  xTaskCreate(WEBSERVER_task, "WebServer task", 8192, NULL, 10, NULL);  // 8Kb stack
}

/********************************************************************
 *  Setup webserver
 *********************************************************************/
void WEBSERVER_setup(void) {
  WEBSERVER_init();

  WEBSERVER_setup_tasks();

  WEBSERVER_cli_handlers();
  setup_uri(&WEBSERVER_api_handlers);

  Serial.println(F("WebServer setup completed..."));
}