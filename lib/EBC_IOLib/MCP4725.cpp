/*******************************************************************
 * MCP4725.cpp
 *
 * EBC MCP4725 support
 *
 *******************************************************************/
#include "EBC_IOLib.h"

/*******************************************************************
 * Definitions
 *******************************************************************/
#define MCP4725_CMD_WRITEDAC 0x40      // Commando write to analog output
#define MCP4725_CMD_WRITE_EEprom 0x60  // Commando write to analog output and Eeprom

#define MCP4725_MSG_LEN 6  // I2C message length

/*******************************************************************
 * Variables
 *******************************************************************/
extern TwoWire I2C_1;

/*******************************************************************
 * @brief Reads analog input from the specified MCP4725 address.
 *
 * This function reads analog input from the specified MCP4725
 * address using I2C communication.
 *
 * @param address The address of the MCP4725 device to read from.
 * @param data Pointer to the buffer where the read data will be stored.
 * @param len The length of the data to be read.
 *
 * @return 0 if successful, -1 otherwise.
 *******************************************************************/
static int MCP4725_read(uint8_t address, uint8_t *data, int len) {
  int result, ndx = 0;

  if (address != 0) {
    I2C_1.requestFrom((int)address, len);
    vTaskDelay(5 / portTICK_PERIOD_MS);  // Wait for the data to be available
    if (!I2C_1.endTransmission()) {
      while (I2C_1.available() && (ndx++ < len)) {
        *data++ = I2C_1.read();
      }
      return 0;  // Ok
    }
  }
  return -1;  // Invalid address
}

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
int MCP4725_read_eeprom(uint8_t address) {
  uint8_t data[MCP4725_MSG_LEN] = {0, 0, 0, 0, 0, 0};

  if (address && !MCP4725_read(address, data, sizeof(data))) {
    return (int(data[3]) << 8) + int(data[4]);  // big-endian
  }

  return DAC_READ_ERROR;  // Error
}

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
int MCP4725_read_status(uint8_t address) {
  uint8_t data[MCP4725_MSG_LEN] = {0, 0, 0, 0, 0, 0};

  if (address && !MCP4725_read(address, data, sizeof(data))) {
    return (int(data[1]) << 4) + ((int(data[2]) >> 4) & 0x000F);  // big-endian
  }

  return DAC_READ_ERROR;  // Error
}

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
int MCP4725_write(uint8_t address, uint16_t value) {

  if (address != 0) {
    value <<= 4;  // Shift 12 Bit Value to 16 Bit

    I2C_1.beginTransmission(address);             // Set the MCP4725 address
    if (I2C_1.write(MCP4725_CMD_WRITEDAC) == 1) { // Write command to analog output
      if (I2C_1.write(highByte(value)) == 1) {    // Upper data bits (D11.D10.D9.D8.D7.D6.D5.D4)
        if (I2C_1.write(lowByte(value)) == 1) {   // Lower data bits (D3.D2.D1.D0.x.x.x.x)
          return int(I2C_1.endTransmission());    // End transmission
        }
      }
    }
    return 0; // Ok
  }

  return -1;  // Write failed
}