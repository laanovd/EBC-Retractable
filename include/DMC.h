/*******************************************************************
 * DMC.h
 *    
 * EBC Dynamic Motor Control
 * 
 *******************************************************************/
#ifndef DMC_HEADER
#define DMC_HEADER

/*******************************************************************
 * JSON and Websocket keys
 *******************************************************************/
#define JSON_DMC_ENABLED "dmc_enabled"

/*******************************************************************
 * DMC enable
 *******************************************************************/
extern void DMC_enable();
extern void DMC_disable();
extern bool DMC_enabled();

/*******************************************************************
 * Stops the DMC (Dynamic Motor Controller).
 * This function disables the DMC and prints a message to the serial monitor.
 *******************************************************************/
extern void DMC_stop(void);

/*******************************************************************
 * @brief Sets up the DMC (Dynamic Motor Controller).
 * 
 * This function initializes the variables, configures the GPIO pins, and disables the DMC.
 * 
 * @note This function should be called once during the setup phase of the program.
 *******************************************************************/
extern void DMC_setup(void);

/*******************************************************************
 * @brief Starts the DMC module.
 * 
 * This function initializes the DMC module by setting up the command-line interface (CLI)
 * and configuring the API handlers. It also prints a message to the serial monitor indicating
 * that the DMC module has started.
 *******************************************************************/
extern void DMC_start(void);

#endif