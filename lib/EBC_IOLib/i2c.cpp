/*******************************************************************
 * main.cpp
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
TwoWire I2C_1 = TwoWire(0);

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
bool I2C_setup(int sda0, int scl0) {
    return I2C_1.begin(sda0, scl0, 400000);  // 12c bus Netwerk 0
}

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
int I2C_scan(uint8_t *I2C_list, int len) {
  byte address;
  int index = 0;

  memset(I2C_list, 0, len);

  for (address = 1; address < 127; address++) {
    I2C_1.beginTransmission(address);
    if (!I2C_1.endTransmission()) {
      if (index < len) {
        *I2C_list++ = address;  // Store address in array
        index++;
      }
    }
  }
  return index;
}
