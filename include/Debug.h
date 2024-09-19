/********************************************************************
 *    Debug.ino
 *    
 *    Debugging tools
 * 
 *******************************************************************/
#ifndef DEBUG_HEADER_ID
#define DEBUG_HEADER_ID

/********************************************************************
 * Constants
 *******************************************************************/

/********************************************************************
 * Show information (unconditional)
 *******************************************************************/
extern void DEBUG_info(String text);

/********************************************************************
 * Show communication (conditional)
 *******************************************************************/
extern bool DEBUG_frames_active(void);
extern void DEBUG_frames(String text);

/********************************************************************
 * Show data (conditional)
 *******************************************************************/
extern bool DEBUG_data_active(void);
extern void DEBUG_data(String text);

/********************************************************************
 * Show communication-bus (conditional)
 *******************************************************************/
extern void DEBUG_bus(String text);

/********************************************************************
 * Show CAN commnication (conditional)
 *******************************************************************/
extern void DEBUG_can(String label, const int length, const int id, const int rtr, const uint8_t buffer[]);

/********************************************************************
 * @brief Switches off debug output.
 *
 * This function disables the debug frames, debug data, and debug bus.
 *********************************************************************/
extern void DEBUG_stop(void);

/********************************************************************
 * @brief Initializes the debug functionality.
 * 
 * This function sets up the debug queue, tasks, and command line 
 * interface handlers. It also marks the debug setup as completed 
 * and prints a message to indicate that the setup is done.
 *********************************************************************/
extern void DEBUG_init(void);

/********************************************************************
 * @brief Starts the debug mode.
 * 
 * This function prints a debug message indicating that the debug 
 * mode has started.
 *********************************************************************/
extern void DEBUG_start(void);

#endif // DEBUG_HEADER_ID