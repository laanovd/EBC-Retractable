/*******************************************************************
 * test_gpio.cpp
 *
 * EBC DMC unittest
 *
 *******************************************************************/
#include <Arduino.h>
#include <endian.h>
#include <string.h>
#include <unity.h>
#include <Wire.h>

#include "Config.h"
#include "EBC_IOLib.h"

/*******************************************************************
 * Definitions
 *******************************************************************/
#define TEST_PIN_P0  0
#define TEST_PIN_P1  1
#define TEST_PIN_P2  2
#define TEST_PIN_P3  3
#define TEST_PIN_P4  4
#define TEST_PIN_P5  5
#define TEST_PIN_P6  6
#define TEST_PIN_P7  7

#define DAC_READ_ERROR 5000

#define SDA0 21  // I2C Bus SDA
#define SCL0 22  // I2C Bus SCL

/*******************************************************************
 * Globals
 *******************************************************************/
static uint8_t PCF8574_address = 0x20;
static uint8_t MCP4725_address = 0x60;
static uint8_t PCF8574_list[6] = {0,0,0,0,0,0};

/*******************************************************************
 * SUITE SetUp
 *******************************************************************/
void setUp(void) {
  // set stuff up here
  TEST_ASSERT_TRUE(I2C_setup(SDA0, SCL0));
}

/*******************************************************************
 * SUITE TearDown
 *******************************************************************/
void tearDown(void) {
  // clean stuff up here
}

/*******************************************************************
 * Unittests I2C
 *******************************************************************/
void test_i2c_scan(void) {
  uint8_t list[6] = {0,0,0,0,0,0};

  TEST_ASSERT_TRUE(I2C_scan(list, sizeof(list)) == 2);
  TEST_ASSERT_EQUAL(0x20, list[0]); // PCF8574
  TEST_ASSERT_EQUAL(0x60, list[1]); // MCP4725
}

/*******************************************************************
 * Unittests PCF8574
 *******************************************************************/
void test_PCF8574_reset(void) {
  TEST_ASSERT_EQUAL((int)0x20, (int)PCF8574_address);

  PCF8574_write(PCF8574_address, 0, PCF8574_RESET); // Pin is ignored on reset
  TEST_ASSERT_EQUAL(PCF8574_OFF, PCF8574_read(PCF8574_address, TEST_PIN_P0));
  TEST_ASSERT_EQUAL(PCF8574_OFF, PCF8574_read(PCF8574_address, TEST_PIN_P1));
  TEST_ASSERT_EQUAL(PCF8574_OFF, PCF8574_read(PCF8574_address, TEST_PIN_P2));
  TEST_ASSERT_EQUAL(PCF8574_OFF, PCF8574_read(PCF8574_address, TEST_PIN_P3));
  TEST_ASSERT_EQUAL(PCF8574_OFF, PCF8574_read(PCF8574_address, TEST_PIN_P4));
  TEST_ASSERT_EQUAL(PCF8574_OFF, PCF8574_read(PCF8574_address, TEST_PIN_P5));
  TEST_ASSERT_EQUAL(PCF8574_OFF, PCF8574_read(PCF8574_address, TEST_PIN_P6));
}

void test_PCF8574_set_reset(void) {
  PCF8574_write(PCF8574_address, 0, PCF8574_RESET); // Pin is ignored on reset

  TEST_ASSERT_FALSE(PCF8574_read(PCF8574_address, TEST_PIN_P0));
  PCF8574_write(PCF8574_address, TEST_PIN_P0, PCF8574_ON);
  TEST_ASSERT_TRUE(PCF8574_read(PCF8574_address, TEST_PIN_P0));
  sleep(1);
  PCF8574_write(PCF8574_address, TEST_PIN_P0, PCF8574_OFF);
  TEST_ASSERT_FALSE(PCF8574_read(PCF8574_address, TEST_PIN_P0));

  TEST_ASSERT_FALSE(PCF8574_read(PCF8574_address, TEST_PIN_P1));
  PCF8574_write(PCF8574_address, TEST_PIN_P1, PCF8574_ON);
  TEST_ASSERT_TRUE(PCF8574_read(PCF8574_address, TEST_PIN_P1));
  sleep(1);
  PCF8574_write(PCF8574_address, TEST_PIN_P1, PCF8574_OFF);
  TEST_ASSERT_FALSE(PCF8574_read(PCF8574_address, TEST_PIN_P1));

  TEST_ASSERT_FALSE(PCF8574_read(PCF8574_address, TEST_PIN_P2));
  PCF8574_write(PCF8574_address, TEST_PIN_P2, PCF8574_ON);
  TEST_ASSERT_TRUE(PCF8574_read(PCF8574_address, TEST_PIN_P2));
  sleep(1);
  PCF8574_write(PCF8574_address, TEST_PIN_P2, PCF8574_OFF);
  TEST_ASSERT_FALSE(PCF8574_read(PCF8574_address, TEST_PIN_P2));

  TEST_ASSERT_FALSE(PCF8574_read(PCF8574_address, TEST_PIN_P3));
  PCF8574_write(PCF8574_address, TEST_PIN_P3, PCF8574_ON);
  TEST_ASSERT_TRUE(PCF8574_read(PCF8574_address, TEST_PIN_P3));
  sleep(1);
  PCF8574_write(PCF8574_address, TEST_PIN_P3, PCF8574_OFF);
  TEST_ASSERT_FALSE(PCF8574_read(PCF8574_address, TEST_PIN_P3));

  TEST_ASSERT_FALSE(PCF8574_read(PCF8574_address, TEST_PIN_P4));
  PCF8574_write(PCF8574_address, TEST_PIN_P4, PCF8574_ON);
  TEST_ASSERT_TRUE(PCF8574_read(PCF8574_address, TEST_PIN_P4));
  sleep(1);
  PCF8574_write(PCF8574_address, TEST_PIN_P4, PCF8574_OFF);
  TEST_ASSERT_FALSE(PCF8574_read(PCF8574_address, TEST_PIN_P4));

  TEST_ASSERT_FALSE(PCF8574_read(PCF8574_address, TEST_PIN_P5));
  PCF8574_write(PCF8574_address, TEST_PIN_P5, PCF8574_ON);
  TEST_ASSERT_TRUE(PCF8574_read(PCF8574_address, TEST_PIN_P5));
  sleep(1);
  PCF8574_write(PCF8574_address, TEST_PIN_P5, PCF8574_OFF);
  TEST_ASSERT_FALSE(PCF8574_read(PCF8574_address, TEST_PIN_P5));

  TEST_ASSERT_FALSE(PCF8574_read(PCF8574_address, TEST_PIN_P6));
  PCF8574_write(PCF8574_address, TEST_PIN_P6, PCF8574_ON);
  TEST_ASSERT_TRUE(PCF8574_read(PCF8574_address, TEST_PIN_P6));
  sleep(1);
  PCF8574_write(PCF8574_address, TEST_PIN_P6, PCF8574_OFF);
  TEST_ASSERT_FALSE(PCF8574_read(PCF8574_address, TEST_PIN_P6));

  TEST_ASSERT_FALSE(PCF8574_read(PCF8574_address, TEST_PIN_P7));
  PCF8574_write(PCF8574_address, TEST_PIN_P7, PCF8574_ON);
  TEST_ASSERT_TRUE(PCF8574_read(PCF8574_address, TEST_PIN_P7));
  sleep(1);
  PCF8574_write(PCF8574_address, TEST_PIN_P7, PCF8574_OFF);
  TEST_ASSERT_FALSE(PCF8574_read(PCF8574_address, TEST_PIN_P7));

  sleep(1);
  PCF8574_write(PCF8574_address, TEST_PIN_P0, PCF8574_ON);
  PCF8574_write(PCF8574_address, TEST_PIN_P1, PCF8574_ON);
  PCF8574_write(PCF8574_address, TEST_PIN_P2, PCF8574_ON);
  PCF8574_write(PCF8574_address, TEST_PIN_P3, PCF8574_ON);
  PCF8574_write(PCF8574_address, TEST_PIN_P4, PCF8574_ON);
  PCF8574_write(PCF8574_address, TEST_PIN_P5, PCF8574_ON);
  PCF8574_write(PCF8574_address, TEST_PIN_P6, PCF8574_ON);
  PCF8574_write(PCF8574_address, TEST_PIN_P7, PCF8574_ON);
  sleep(1);
  PCF8574_write(PCF8574_address, TEST_PIN_P0, PCF8574_OFF);
  PCF8574_write(PCF8574_address, TEST_PIN_P1, PCF8574_OFF);
  PCF8574_write(PCF8574_address, TEST_PIN_P2, PCF8574_OFF);
  PCF8574_write(PCF8574_address, TEST_PIN_P3, PCF8574_OFF);
  PCF8574_write(PCF8574_address, TEST_PIN_P4, PCF8574_OFF);
  PCF8574_write(PCF8574_address, TEST_PIN_P5, PCF8574_OFF);
  PCF8574_write(PCF8574_address, TEST_PIN_P6, PCF8574_OFF);
  PCF8574_write(PCF8574_address, TEST_PIN_P7, PCF8574_OFF);
  sleep(1);
}

/*******************************************************************
 * Unittests MCP4725
 *******************************************************************/
void test_MCP4725_DAC(void) {
  TEST_ASSERT_EQUAL(0, MCP4725_write(MCP4725_address, 0));
  TEST_ASSERT_EQUAL(0, MCP4725_read_status(MCP4725_address));

  TEST_ASSERT_EQUAL(0, MCP4725_write(MCP4725_address, 1000));
  TEST_ASSERT_EQUAL(1000, MCP4725_read_status(MCP4725_address));
}

/*******************************************************************
 * Setup and loop
 *******************************************************************/
void setup() {
  // NOTE!!! Wait for > 2 secs
  // if board doesn't support software reset via Serial.DTR/RTS
  delay(2000);
}

void loop() {
  UNITY_BEGIN();

  // I2C
  RUN_TEST(test_i2c_scan);

  // PCF8574
  RUN_TEST(test_PCF8574_reset);
  RUN_TEST(test_PCF8574_set_reset);

  // MCP4725
  RUN_TEST(test_MCP4725_DAC);

  UNITY_END();
}