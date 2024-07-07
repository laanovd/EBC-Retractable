/*******************************************************************
 *    WiFi.ino
 *    
 *    WiFi setup
 * 
 *******************************************************************/
#ifndef WIFI_HEADER_ID
#define WIFI_HEADER_ID

/*******************************************************************
 * JSON fields
 *******************************************************************/
#define JSON_WIFI_SSID "ssid"
#define JSON_WIFI_MAC "mac-address"
#define JSON_WIFI_IP "ip-address"
#define JSON_WIFI_SUBNET "subnetmask"
#define JSON_WIFI_GATEWAY "gateway"
#define JSON_WIFI_DNS "dns"
#define JSON_WIFI_CONNECTED "connected"

/*******************************************************************
 * Check if WiFi connected
 *******************************************************************/
extern bool wifi_connected(void);

/*******************************************************************
 * Get WiFi SSID
 *******************************************************************/
extern String WiFi_ssid(void);

/*******************************************************************
 * Get WiFi MAC address
 *******************************************************************/
extern String WiFi_mac(void);

/********************************************************************
 * Create WiFi string
 *******************************************************************/
extern String WiFi_string(void);

/*******************************************************************
 * Externals
 *******************************************************************/
extern void WiFi_setup(void);

#endif // WIFI_HEADER_ID