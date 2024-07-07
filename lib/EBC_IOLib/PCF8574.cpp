/*******************************************************************
 * BCF8574.cpp
 *
 * EBC PC8574 support
 *
 *******************************************************************/
#include "EBC_IOLib.h"

/*******************************************************************
 * Definitions
 *******************************************************************/

/*******************************************************************
 * Variables
 *******************************************************************/
extern TwoWire I2C_1;
static int PCF8574_mask = 0xFF;

/*******************************************************************
 * @brief Sets the value of a PCF8574 I2C device.
 *
 * This function sets the value of a PCF8574 I2C device at the specified address.
 *
 * @param address The I2C address of the PCF8574 device.
 * @param data The value to be set on the PCF8574 device.
 *
 * @return True if the transmission was successful, false otherwise.
 *******************************************************************/
static bool PCF8574_set(uint8_t address, uint8_t data) {
  if (!address) return false;  // No valid I2C address

  I2C_1.beginTransmission(address);
  if (I2C_1.write(data) == 1) {
    return (!I2C_1.endTransmission()) ? true : false;
  }
  return false;
}

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
bool PCF8574_write(uint8_t address, int pin, int value) {
    switch (value) {
        case PCF8574_RESET:
            /**
             * @brief Resets the PCF8574 device.
             */
            PCF8574_mask = 0xFF;  // Reset
            break;
        case PCF8574_OFF:
            /**
             * @brief Sets the specified output pin to OFF.
             */
            PCF8574_mask |= (1 << pin);  // 1 is output OFF
            break;
        case PCF8574_ON:
            /**
             * @brief Sets the specified output pin to ON.
             */
            PCF8574_mask &= ~(1 << pin);  // 0 is output ON
            break;
        default:
            // No action
            break;
    }
    return PCF8574_set(address, PCF8574_mask);
}

/*******************************************************************
 * @brief Reads the state of a specified output pin from a PCF8574 I2C device.
 *
 * This function reads the state of a specified output pin from a PCF8574 I2C device at the specified address.
 *
 * @param address The I2C address of the PCF8574 device.
 * @param pin The output pin number to be read.
 *
 * @return PCF8575_ON if the specified output was ON, PCF8575_OFF otherwise.
 *******************************************************************/
int PCF8574_read(uint8_t address, int pin) {
    (void)address;

    return (PCF8574_mask & (1 << pin)) ? PCF8574_OFF : PCF8574_ON;
}
