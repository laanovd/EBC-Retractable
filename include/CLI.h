/*******************************************************************
 * Commandline interface
 *   
 *******************************************************************/
#ifndef CLI_HEADER
#define CLI_HEADER

#include <SimpleCLI.h>

/********************************************************************
 * Type definitions
 *******************************************************************/
typedef enum {
  CLI_SERIAL,
  CLI_WEBSERIAL
} cli_output_t; /* Commandline interface output channel */

/********************************************************************
 * Command line interface
 *******************************************************************/
extern SimpleCLI cli;

/********************************************************************
 * Add description to help list
 *********************************************************************/
extern void cli_add_help_description(const char *description);

/********************************************************************
 * Output mode for commandline interface
 *******************************************************************/
extern void cli_set_output(cli_output_t value);

/********************************************************************
 *  WebSerial Command line interface
 *******************************************************************/
extern void CLI_webserial_task(uint8_t* data, size_t len);

/********************************************************************
 * Output ComandlIne interface data
 *******************************************************************/
extern void CLI_print(String txt);
extern void CLI_println(String txt);

/*******************************************************************
 * Restarts the system after a delay of 3 seconds.
 *******************************************************************/
extern void system_restart(void);

/*******************************************************************
 * @brief Initializes the CLI module.
 * 
 * This function sets up the tasks and handlers for the CLI module.
 * After calling this function, the CLI handlers are ready to process commands.
 * 
 * @note This function should be called before using any CLI functionality.
 *******************************************************************/
extern void CLI_init(void);

/*******************************************************************
 * @brief Starts the CLI handlers.
 * 
 * This function prints a message to the serial monitor indicating 
 * that the CLI handlers have started.
 *******************************************************************/
extern void CLI_start(void);


#endif // CLI_HEADER
