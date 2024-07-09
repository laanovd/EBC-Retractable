#include "GPIO.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Wire.h>

#include "Config.h"
#include "EBC_IOlib.h" 

/*******************************************************************
  Definitions
 *******************************************************************/
#define DEBUG_GPIO

#define SDA0 21  // I2C Bus SDA
#define SCL0 22  // I2C Bus SCL

/*******************************************************************
 * Storage keys and defaults
 *******************************************************************/
#define JSON_GPIO_PCF8574_ADDRESS "dig_out_address"
#define JSON_GPIO_PCF8574_STATUS "dig_out_status"
#define JSON_GPIO_MCP4725_L_ADDRESS "dac_left_address"
#define JSON_GPIO_MCP4725_L_STATUS "dac_left_status"
#define JSON_GPIO_MCP4725_L_EPROM "dac_left_eprom"
#define JSON_GPIO_MCP4725_R_ADDRESS "dac_right_address"
#define JSON_GPIO_MCP4725_R_STATUS "dac_right_status"
#define JSON_GPIO_MCP4725_R_EPROM "dac_right_eprom"

/*******************************************************************
  Globals
 *******************************************************************/
static JsonDocument GPIO_data;

uint8_t PCF8574_address  = 0;
uint8_t MCP4725_R_address  = 0;
uint8_t MCP4725_L_address  = 0;

/********************************************************************
 * Create initial JSON data
 *******************************************************************/
static JsonDocument GPIO_json_int(void) {
  JsonDocument doc;

  doc[JSON_GPIO_PCF8574_ADDRESS] = 0;
  doc[JSON_GPIO_PCF8574_STATUS] = 0;

  doc[JSON_GPIO_MCP4725_L_ADDRESS] = 0;
  doc[JSON_GPIO_MCP4725_L_STATUS] = 0;
  doc[JSON_GPIO_MCP4725_L_EPROM] = 0;

  doc[JSON_GPIO_MCP4725_R_ADDRESS] = 0;
  doc[JSON_GPIO_MCP4725_R_STATUS] = 0;
  doc[JSON_GPIO_MCP4725_R_EPROM] = 0;

  return doc;
}

/*******************************************************************
 * Emergency stop
 *******************************************************************/
bool EMERGENCY_STOP_active(void) {
  // return pinMode(EMERGNECY_STOP_PIN, INPUT);
  return false;
}

/*******************************************************************
 * LED
 *******************************************************************/
static int led_blink_timer = 0;
bool LED_blink_takt(void) {
  return (led_blink_timer < 2);
}

bool LED_error_takt(void) {
  return (led_blink_timer == 1) == 0;
}

/*******************************************************************
  LED heartbeat setup
 *******************************************************************/
void LED_HEARTBEAT_update(void) {
  digitalWrite(LED_HEARTBEAT_PIN, LED_blink_takt());
}

/*******************************************************************
  LED error setup
 *******************************************************************/
void LED_ERROR_update(void) {
  int state = true ? LED_blink_takt() : LOW;  // TODO: link general error status
  digitalWrite(LED_ERROR_PIN, state);
}

/*******************************************************************
  GPIO main task
 *******************************************************************/
static void GPIO_main(void *parameter) {
  (void)parameter;

  while (true) {
    LED_HEARTBEAT_update();
    LED_ERROR_update();

    ++led_blink_timer %= 4;
    vTaskDelay(250 / portTICK_PERIOD_MS);
  }
}

/*******************************************************************
 * System LED setup
 *******************************************************************/
static void LED_setup(void) {
  pinMode(LED_HEARTBEAT_PIN, OUTPUT);
  pinMode(LED_ERROR_PIN, OUTPUT);

  digitalWrite(LED_HEARTBEAT_PIN, HIGH);
  digitalWrite(LED_ERROR_PIN, HIGH);

  vTaskDelay(1000 / portTICK_PERIOD_MS);

  digitalWrite(LED_HEARTBEAT_PIN, LOW);
  digitalWrite(LED_ERROR_PIN, LOW);
}

/*******************************************************************
  I2C setup
 *******************************************************************/
static void I2C_init(void) {
  uint8_t list[6] = {0,0,0,0,0,0};

  PCF8574_address = 0;
  MCP4725_R_address = 0;
  MCP4725_R_address = 0;

  if (I2C_setup(SDA0, SCL0)) {
    if (I2C_scan(list, sizeof(list)) == 2) {
      if (list[0] < 0x60) {
        PCF8574_address = list[0];
        MCP4725_R_address = list[1];
      }
      else {
        MCP4725_R_address = list[0];
        MCP4725_L_address = list[1];
      }

      if (MCP4725_L_address) {
        GPIO_data[JSON_GPIO_MCP4725_L_EPROM] = (int)MCP4725_read_eeprom(MCP4725_L_address);
        GPIO_data[JSON_GPIO_MCP4725_L_STATUS] = (int)MCP4725_read_status(MCP4725_L_address);
      }

      if (MCP4725_R_address) {
        GPIO_data[JSON_GPIO_MCP4725_R_EPROM] = (int)MCP4725_read_eeprom(MCP4725_R_address);
        GPIO_data[JSON_GPIO_MCP4725_R_STATUS] = (int)MCP4725_read_status(MCP4725_R_address);
      }

      Serial.println("I2C Bus 1 opgestart.");
      return;
    }
  }
  Serial.println("I2C Bus 1 error.");
}

/*******************************************************************
  Setup tasks
 *******************************************************************/
static void setup_tasks(void) {
  xTaskCreate(GPIO_main, "GPIO main", 1024, NULL, 15, NULL);
}

/*******************************************************************
  Setup variables
 *******************************************************************/
static void setup_variables(void) {
}

/*******************************************************************
  Main setup
 *******************************************************************/
void GPIO_setup(void) {
  setup_variables();
  I2C_init();
  LED_setup();
}

/*******************************************************************
  Start
 *******************************************************************/
void GPIO_start(void) {
  setup_tasks();

  Serial.println(F("GPIO setup completed..."));
}
