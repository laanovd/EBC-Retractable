/*********************************************************************
 *    Debug.ino
 *    
 *    Debugging tools
 * 
 ********************************************************************/
#ifndef DEBUG_HEADER_ID
#define DEBUG_HEADER_ID

/*********************************************************************
 * Constants
 ********************************************************************/

/*********************************************************************
 * Show information (unconditional)
 ********************************************************************/
extern void DEBUG_info(String text);

/*********************************************************************
 * Show communication (conditional)
 ********************************************************************/
extern bool DEBUG_frames_active(void);
extern void DEBUG_frames(String text);

/*********************************************************************
 * Show data (conditional)
 ********************************************************************/
extern bool DEBUG_data_active(void);
extern void DEBUG_data(String text);

/*********************************************************************
 * Show communication-bus (conditional)
 ********************************************************************/
extern void DEBUG_bus(String text);

/*********************************************************************
 * Show CAN commnication (conditional)
 ********************************************************************/
extern void DEBUG_can(String label, const int length, const int id, const int rtr, const uint8_t buffer[]);

/*********************************************************************
 *  Setup debugging
 ********************************************************************/
extern void DEBUG_setup(void);

#endif // DEBUG_HEADER_ID