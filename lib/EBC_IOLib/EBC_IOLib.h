/*******************************************************************
 * main.cpp
 *
 * EBC PC8574 support
 *
 *******************************************************************/
#ifndef EBC_PCF8574_HEADER
#define EBC_PCF8574_HEADER

#include <Arduino.h>
#include <Wire.h>

/*******************************************************************
 * Definitions
 *******************************************************************/
#define PCF8574_ON 1
#define PCF8574_OFF 0
#define PCF8574_RESET -1

// MCP4725
#define DAC_READ_ERROR 5000

#define ADC_MIN 0
#define ADC_MAX 4095
#define DAC_MIN 0
#define DAC_MAX 4095

/*******************************************************************
 * @brief Starts the I2C/TwoWire driver.
 *
 * This function initializes the I2C/TwoWire driver with the
 * specified SDA and SCL pins and a clock speed of 400000.
 *
 * @param sda0 The SDA pin number.
 * @param scl0 The SCL pin number.
 *
 * @return True if the setup was successful, false otherwise.
 *******************************************************************/
extern bool I2C_setup(int sda0, int scl0);

/*******************************************************************
 * @brief Scans the I2C/TwoWire bus for connected devices.
 *
 * This function scans the I2C bus from address 1 to 126 and checks if a device responds.
 * If a device is found, its address is stored in the I2C_list array.
 * The function returns the number of found devices and, zero is no device found.
 *
 * @param list Reference to a uint8_t list of found device addresses will be stored.
 * @param len Length of the list variable.
 *
 * @return The number of devices found on the I2C bus.
 * If no devices are found, it returns 0.
 *******************************************************************/
extern int I2C_scan(uint8_t *I2C_list, int len);

/*******************************************************************
 * @brief Writes a value to a PCF8574 I2C device.
 *
 * This function writes a value to a PCF8574 I2C device at the specified address.
 *
 * @param address The I2C address of the PCF8574 device.
 * @param pin The output pin number to be set.
 * @param value The value to be set on the specified output pin.
 *
 * @return True if the write was successful, false otherwise.
 *******************************************************************/
extern bool PCF8574_write(uint8_t address, int pin, int value);

/*******************************************************************
 * @brief Reads the state of a specified output pin from a PCF8574 I2C device.
 *
 * This function reads the state of a specified output pin
 * from a PCF8574 I2C device at the specified address.
 *
 * @param pin The output pin number to be read.
 *
 * @return PCF8575_ON if the specified output was ON, PCF8575_OFF otherwise.
 *******************************************************************/
extern int PCF8574_read(uint8_t address, int pin);

/*******************************************************************
 * @brief Reads the EEPROM of the specified MCP4725 address.
 *
 * This function reads the EEPROM of the specified MCP4725
 * address using I2C communication.
 *
 * @param address The address of the MCP4725 device to read from.
 *
 * @return The 12-bit value stored in the EEPROM of the specified MCP4725 address.
 *         If an error occurs during the reading process, it returns DAC_READ_ERROR.
 *******************************************************************/
extern int MCP4725_read_eeprom(uint8_t address);

/*******************************************************************
 * @brief Reads the status of the specified MCP4725 address.
 *
 * This function reads the status of the specified MCP4725
 * address using I2C communication.
 *
 * @param address The address of the MCP4725 device to read from.
 *
 * @return The 4-bit status value stored in the specified MCP4725 address.
 *         If an error occurs during the reading process, it returns DAC_READ_ERROR.
 *******************************************************************/
extern int MCP4725_read_status(uint8_t address);

/*******************************************************************
 * @brief Writes a 12-bit value to the specified MCP4725 address using I2C communication.
 *
 * This function writes a 12-bit value to the specified MCP4725 address using I2C communication.
 *
 * @param address The address of the MCP4725 device to write to.
 * @param value The 12-bit value to be written.
 *
 * @return 0 if successful, -1 otherwise.
 *******************************************************************/
extern int MCP4725_write(uint8_t address, uint16_t value);

#endif  // EBC_PCF8574_HEADER