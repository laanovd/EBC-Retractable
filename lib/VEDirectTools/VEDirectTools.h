#include <Arduino.h>

#ifndef VEDIRECTTOOLS_H
#define VEDIRECTTOOLS_H

/********************************************************************
 * VEDirect receive handlers format
 ********************************************************************/
typedef void (*VEDirect_callback) (String msg);

/********************************************************************
 * VEDirect Set callbacks
 ********************************************************************/
extern void VEDirect_TEXT_callback(VEDirect_callback handler);
extern void VEDirect_HEX_callback(VEDirect_callback handler);

/********************************************************************
 * VEDirect parser
 ********************************************************************/
extern void VEDirect_parser(int rx_char);

/********************************************************************
 * hex2int
 * take a hex string and convert it to a 32bit number (max 8 hex digits)
 ********************************************************************/
extern uint32_t hex2int(char *hex);

/********************************************************************
 * String tools: convert byte to hex string
 ********************************************************************/
extern String byte2hex(uint8_t value);

/********************************************************************
 * hex2ascii
 * take a hex string and convert it to a ascii string
 ********************************************************************/
extern String hex2ascii(String msg);

/********************************************************************
 * String tools: convert string to hex-string
 ********************************************************************/
extern String str2hex(String str);
extern String str2hex20(String str);
extern String str2hex32(String str);

/********************************************************************
 * VEDOut calc hex-checksum
 ********************************************************************/
extern int ved_hex_checksum(String text);

/********************************************************************
 * VEDOut calc text-checksum
 ********************************************************************/
extern int ved_text_checksum(String text);

/********************************************************************
 * VEDOut is-valid_hex
 * Message from device including Checksum
 ********************************************************************/
extern bool ved_is_valid_text(String msg);

/********************************************************************
 * VEDOut add-text-checksum
 ********************************************************************/
extern String ved_add_text_checksum(String msg);

#endif // VEDIRECTTOOLS_h