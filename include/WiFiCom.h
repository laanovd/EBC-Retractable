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

/*******************************************************************
 * Get IP address
 *******************************************************************/
extern String WiFi_ip(void);

/********************************************************************
 * Create WiFi string
 *******************************************************************/
extern String WiFi_string(void);

/*******************************************************************
 * @brief Initializes the WiFi module.
 * 
 * This function sets up the necessary variables for WiFi operation 
 * and prints a message to the serial monitor indicating that WiFi 
 * has been initialized.
 *******************************************************************/
extern void WiFi_init(void);

/*******************************************************************
 * @brief Starts the WiFi connection.
 * 
 * This function sets up the WiFi connection by calling the necessary 
 * setup functions, registering the command line interface (CLI) handlers, 
 * and setting up the URI for the WiFi API handlers. After the setup 
 * is complete, it prints a message to the serial monitor indicating 
 * that the WiFi has started.
 *******************************************************************/
extern void WiFi_start(void);

#endif // WIFI_HEADER_ID