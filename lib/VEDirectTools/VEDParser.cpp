#include <Arduino.h>
#include <VEDirectTools.h>
#include <endian.h>
#include <string.h>
#include <unity.h>

/********************************************************************
 * VEDirect receive parser
 ********************************************************************/
static VEDirect_callback ved_text_callback = NULL;
static VEDirect_callback ved_hex_callback = NULL;

/********************************************************************
 * VEDirect Set TEXT handler
 ********************************************************************/
void VEDirect_TEXT_callback(VEDirect_callback handler) {
  ved_text_callback = handler;
}

/********************************************************************
 * VEDirect Set HEX handler
 ********************************************************************/
void VEDirect_HEX_callback(VEDirect_callback handler) {
  ved_hex_callback = handler;
}

/********************************************************************
 * VEDirect parser
 ********************************************************************/
void VEDirect_parser(int rx_char) {
  static String str = "";
  static bool hex_mode;

  switch (rx_char) {
    case ':':
      hex_mode = str.isEmpty();
      str += ':';
      break;

    case '\r':
      // Ignore
      break;

    case '\n':
      if (str.length() >= 4) {
        if (hex_mode) {
          if (ved_hex_callback && str[0] == ':') {
            ved_hex_callback(str);
          }
          hex_mode = false;
        } else {
          if (ved_text_callback) {
            ved_text_callback(str);
          }
        }
      }
      str = "";
      break;

    default:
      str += char(rx_char);
      break;
  }
}
