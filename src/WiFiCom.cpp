/*******************************************************************
 *    WiFi.ino
 *
 *    WiFi handling
 *
 *******************************************************************/
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>

#include "Config.h"
#include "CLI.h"
#include "Storage.h"
#include "WebServer.h"

#include "WiFiCom.h"

/*******************************************************************
 * Constants
 *******************************************************************/
#undef DEBUG_NETWWORKS

/*******************************************************************
 * Types
 *******************************************************************/
struct network_t {
  const String ssid;
  const String password;
};

/*******************************************************************
 * Network definitions
 *******************************************************************/
const char* password_ap = "QweR019283";

#define MAX_NETWORKS 7
struct network_t known_networks[MAX_NETWORKS] = {
  // SSID                   PASSWORD
  {"PPPWE13Q45A",     "3d22G5AQZ#12"},  // 0: Netwerk RVeer
  {"ARV751909B7A8",   "29F004FE4697"},  // 1: Netwerk Zetten 1
  {"ARV751909B7A8-5", "29F004FE4697"},  // 2: Netwerk Zetten 2
  {"H369A3D48D4",     "ZLJHLB8W52ZQ"},  // 3: Netwerk Weert
  {"Botel technic",   "12Stilsloep!"},  // 4: Netwerk Botel Habour
  {"RDT_Guest",       "=RDT350!"},      // 5: Netwerk Rim Drive Technology
  {"ColorBC",         "avrT33kv"}};     // 6: Netwerk Rim Drive Technology

/*******************************************************************
 * Check if WiFi connected
 *******************************************************************/
bool wifi_connected(void) {
  return (WiFi.status() == WL_CONNECTED) ? true : false;
}

/*******************************************************************
 * Get WiFi SSID
 *******************************************************************/
String WiFi_ssid(void) {
  char buf[64];
  snprintf(buf, sizeof(buf), "%s-%s", WiFiSSIDPrefix, ChipIds().c_str());
  return String(buf);
}

/*******************************************************************
 * Get WiFi MAC address
 *******************************************************************/
String WiFi_mac(void) {
  return String(WiFi.macAddress());
}

/*******************************************************************
 * Get IP address
 *******************************************************************/
String WiFi_ip(void) {
  return WiFi.localIP().toString();
}

/********************************************************************
 * Create initial JSON data
 *******************************************************************/
static JsonDocument WiFi_json(void) {
  JsonDocument doc;

  doc[JSON_WIFI_SSID] = WiFi_ssid();
  doc[JSON_WIFI_MAC] = WiFi_mac();
  doc[JSON_WIFI_IP] = WiFi.localIP().toString();
  doc[JSON_WIFI_SUBNET] = WiFi.subnetMask().toString();
  doc[JSON_WIFI_GATEWAY] = WiFi.gatewayIP().toString();
  doc[JSON_WIFI_DNS] = WiFi.dnsIP().toString();
  doc[JSON_WIFI_CONNECTED] = WiFi.isConnected();

  return doc;
}

/********************************************************************
 * Create WiFi string
 *******************************************************************/
String WiFi_string(void) {
  JsonDocument doc = WiFi_json();

  String text = "--- WiFi ---";

  text.concat("\r\nWiFi connected: ");
  text.concat(doc[JSON_WIFI_CONNECTED]?"true":"false");
  text.concat(", SSID: ");
  text.concat(doc[JSON_WIFI_SSID].as<const char*>());

  text.concat("\r\nIP-address: ");
  text.concat(doc[JSON_WIFI_IP].as<const char*>());
  text.concat(", MAC-address: ");
  text.concat(doc[JSON_WIFI_MAC].as<const char*>());
  
  text.concat("\r\nSubNetMask: ");
  text.concat(doc[JSON_WIFI_SUBNET].as<const char*>());
  text.concat(", Gateway: ");
  text.concat(doc[JSON_WIFI_GATEWAY].as<const char*>());
  text.concat(", DNS: ");
  text.concat(doc[JSON_WIFI_DNS].as<const char*>());
  text.concat("\r\n");

  return text;  
}

/*******************************************************************
 *  Connect to network with ssid
 *
 *******************************************************************/
static void connect_to_network(String ssid, String password) {
  int Error_count = 0;

  Serial.print("WiFi connect to network ");
  Serial.print(ssid);
  WiFi.mode(WIFI_STA);

  // Use const char* insted of Sting
  WiFi.begin(ssid.c_str(), password.c_str());

  while ((WiFi.status() != WL_CONNECTED) and (Error_count < 30)) {
    vTaskDelay(500 / portTICK_PERIOD_MS);
    Error_count++;
  }

  if (wifi_connected()) {
    Serial.println(", success.");

    IPAddress address = WiFi.localIP();
    Serial.print("Station IP-address ");
    Serial.println(address);
  } else
    Serial.println(", failed.");
}

/*******************************************************************
 *  Connect to network
 *
 *  Returns 0 if connected
 *******************************************************************/
static int wifi_connect(int networks) {
  int i1, i2;

  WiFi.mode(WIFI_STA);  // Put in Station Mode ...

  for (i1 = 0; i1 < networks; i1++) {
#ifdef DEBUG_NETWWORKS
    Serial.print(i1);
    Serial.print(": ");
    Serial.print(WiFi.SSID(i1));
    Serial.print(" (");
    Serial.print(WiFi.RSSI(i1));
    Serial.print(")");
    Serial.println((WiFi.encryptionType(i1) == WIFI_AUTH_OPEN) ? " " : "*");
#endif

    for (i2 = 0; i2 < MAX_NETWORKS; i2++) {
      if (WiFi.SSID(i1) == known_networks[i2].ssid)  // ssid found
      {
        connect_to_network(known_networks[i2].ssid, known_networks[i2].password);
        if (wifi_connected())
          return 0;  // Ok, connected
      }
    }
  }
  Serial.println(F("WiFi not connected to know network."));
  return -1;  // Not connected
}

/*******************************************************************
 *  Scan networks
 *
 *******************************************************************/
int wifi_scan(void) {
  Serial.print("WiFi scan, ");
  int networks = WiFi.scanNetworks();
  Serial.print(networks);
  Serial.println(" networks found.");
  return networks;
}

/*******************************************************************
 *  Setup network as Access Point (AP)
 *
 *******************************************************************/
static void wifi_setup_ap(void) {
  String ssid_ap = WiFi_ssid();

  Serial.println(F("Setting up AP (Access Point)…"));

  WiFi.mode(WIFI_AP);

  WiFi.softAP(ssid_ap.c_str(), password_ap);

  IPAddress IP = WiFi.softAPIP();

  Serial.print("AP IP address: ");
  Serial.print(IP);
  Serial.print(", ssid: ");
  Serial.println(ssid_ap);
  Serial.println(F("AP (Access Point) available …"));
}

/*******************************************************************
 * WiFi startup
 *
 *******************************************************************/
static void WiFi_init(void) {
  String ssid = WiFi_ssid();

  WiFi.disconnect();  // First disconnect when it was connected

  WiFi.setHostname(ssid.c_str());  // define hostname

  int networks = wifi_scan();  // Scan for networks
  if (wifi_connect(networks))  // Try to connect to known network
  {
    wifi_setup_ap();  // Setup Access Point (AP)
  }
}

/********************************************************************
 * REST API: read handler
 *********************************************************************/
void WIFI_rest_read(AsyncWebServerRequest* request) {

  String json;
  serializeJson(WiFi_json(), json);
  request->send(200, "application/json", json.c_str());
}

static rest_api_t WIFI_api_handlers = {
    /* uri */ "/api/v1/wifi",
    /* comment */ "WiFi module",
    /* instances */ 1,
    /* fn_create */ nullptr,
    /* fn_read */ WIFI_rest_read,
    /* fn_update */ nullptr,
    /* fn_delete */ nullptr,
};

/********************************************************************
 * WiFi: List WiFi content
 *******************************************************************/
static void clicb_list_wifi(cmd *c) {
  CLI_println(WiFi_string());
}

/*******************************************************************
 * Setup variables
 *******************************************************************/
static void WiFi_setup_variables(void) {
  String ssid;
  
  if (STORAGE_get_string(JSON_WIFI_SSID, ssid)) {
    ssid = WiFi_ssid();
    STORAGE_set_string(JSON_WIFI_SSID, ssid);
  }

  if (!ssid.equals(WiFi_ssid())) {
    STORAGE_set_string(JSON_WIFI_SSID, WiFi_ssid());
  }
}

/********************************************************************
 * Command Line handler(s)
 *********************************************************************/
static void WiFi_cli_handlers(void) {
  cli.addCommand("wifi", clicb_list_wifi);
}

/*******************************************************************
 * WiFi setup
 *******************************************************************/
void WiFi_setup(void) {
  WiFi_setup_variables();
  WiFi_init();

  WiFi_cli_handlers();
  setup_uri(&WIFI_api_handlers);

  Serial.println(F("WiFi setup completed..."));
}
