/*******************************************************************
 * Commandline interface
 *   
 *******************************************************************/
#ifndef CLI_HEADER
#define CLI_HEADER

#include <SimpleCLI.h>

/*********************************************************************
 * Type definitions
 ********************************************************************/
typedef enum {
  CLI_SERIAL,
  CLI_WEBSERIAL
} cli_output_t; /* Commandline interface output channel */

/*********************************************************************
 * Command line interface
 ********************************************************************/
extern SimpleCLI cli;

/*********************************************************************
 * Add description to help list
 *********************************************************************/
extern void cli_add_help_description(const char *description);

/*********************************************************************
 * Output mode for commandline interface
 ********************************************************************/
extern void cli_set_output(cli_output_t value);

/*********************************************************************
 *  WebSerial Command line interface
 ********************************************************************/
extern void CLI_webserial_task(uint8_t* data, size_t len);

/*********************************************************************
 * Output ComandlIne interface data
 ********************************************************************/
extern void CLI_print(String txt);
extern void CLI_println(String txt);

/********************************************************************
 * System reboot
 ********************************************************************/
extern void system_restart(void);

/*********************************************************************
 *  Initialize the command line interface
 ********************************************************************/
extern void CLI_setup(void);

/*********************************************************************
 *  Set calibration mode
 ********************************************************************/
extern void set_calibrating(bool value);
extern bool is_calibrating();

#endif // CLI_HEADER
