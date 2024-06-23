#include <Arduino.h>
#include <VEDirectTools.h>

/********************************************************************
 * hex2int
 * take a hex string and convert it to a 32bit number (max 8 hex digits)
 ********************************************************************/
uint32_t hex2int(char *hex) {
    uint32_t val = 0;
    while (*hex) {
        uint8_t byte = *hex++; 

        if (byte >= '0' && byte <= '9') byte = byte - '0';
        else if (byte >= 'a' && byte <='f') byte = byte - 'a' + 10;
        else if (byte >= 'A' && byte <='F') byte = byte - 'A' + 10;    

        val = (val << 4) | (byte & 0xF);
    }
    return val;
}

/********************************************************************
 * hex2ascii
 * take a hex string and convert it to a ascii string
 ********************************************************************/
String hex2ascii(String msg) {
  String result;
  int val, pos = msg.indexOf(':');
  char chr[3] = {0, 0, 0};  // Second char is c-string terminator

  /* Double dot */
  result.concat(msg.substring(pos, pos+1)); pos++;

  /* Command is single nibble */
  result.concat(msg.substring(pos, pos+1)); pos++;

  /* sub-command */ 
  result.concat(msg.substring(pos, pos+1)); pos++;
  result.concat(msg.substring(pos, pos+1)); pos++;
  result.concat(msg.substring(pos, pos+1)); pos++;
  result.concat(msg.substring(pos, pos+1)); pos++;

  /* flags */
  result.concat(msg.substring(pos, pos+1)); pos++;
  result.concat(msg.substring(pos, pos+1)); pos++;

  /* Rest always two characters per 'hex-value' */
  while (msg.length() > pos+1) {
    chr[0] = msg.charAt(pos++);
    chr[1] = msg.charAt(pos++);
    val = hex2int(chr);
    result.concat(char(val));
  }

  return result;
}

/********************************************************************
 * String tools: convert byte to hex string
 ********************************************************************/
String byte2hex(uint8_t value) {
  String result;

  if (value < 10) {
    result = "0" + String(value, HEX);
    result.toUpperCase();
    return result;
  }

  result = String(value, HEX);
  result.toUpperCase();
  return result;
} 

/********************************************************************
 * String tools: convert string to hex-string
 ********************************************************************/
String str2hex(String str) {
  String result = "";
  int hex = 0, pos = 0;

  while (pos < str.length()) {
    result += byte2hex(str[pos++]);
  }

  result.toUpperCase();

  return result;
}

String str2hex20(String str) {
  String result = str2hex(str);

  while (result.length() < (20 * 2)) {
    result += "00";
  }

  return result;
}

String str2hex32(String str) {
  String result = str2hex(str);

  while (result.length() < (24 * 2)) {
    result += "00";
  }

  return result;
}

/********************************************************************
 * VEDirect calc hex-checksum
 ********************************************************************/
int ved_hex_checksum(String text) {
  int val, pos = 1, checksum = 0x55;
  char chr[3] = {0,0,0}; // Second char is c-string terminator

  /* Command is single nibble */ 
  chr[0] = text.charAt(pos++);
  checksum -= hex2int(chr);

  /* Rest always two characters per 'hex-value' */
  while (text.length() > pos) {
    chr[0] = text.charAt(pos++);
    chr[1] = text.charAt(pos++);
    checksum -= hex2int(chr);
  }
  return (checksum & 0xFF);
}

/********************************************************************
 * VEDirect calc text-checksum
 ********************************************************************/
int ved_text_checksum(String text) {
  int checksum = 0, pos = 0;

  while (pos < text.length()) {
    checksum = (checksum + text[pos++]) & 255; /* Take modulo 256 in account */
  }

  return (checksum & 0xFF);
}

/********************************************************************
 * VEDirect is-valid_text
 ********************************************************************/
bool ved_is_valid_text(String msg) {
  return (ved_text_checksum(msg) == 0);
}

/********************************************************************
 * VEDirect add-text-checksum
 ********************************************************************/
String ved_add_text_checksum(String msg) {
  int checksum = ved_text_checksum(msg);

  msg += char(0x100 - checksum);

  return msg;
}
