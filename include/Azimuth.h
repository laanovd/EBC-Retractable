/*******************************************************************
 * Azimuth.h
 *    
 * EBC azimuth control
 * 
 *******************************************************************/
#ifndef AZIMUTH_HEADER
#define AZIMUTH_HEADER

/*******************************************************************
 * JSON and Websocket keys
 *******************************************************************/
#define JSON_AZIMUTH_ENABLED "azimuth_enabled"
#define JSON_AZIMUTH_OUTPUT_ENABLED "azimuth_analog_out_enabled"
#define JSON_AZIMUTH_LOW "azimuth_low_counts"
#define JSON_AZIMUTH_HIGH "azimuth_high_counts"
#define JSON_AZIMUTH_MIDDLE "azimuth_middle_counts"
#define JSON_AZIMUTH_ACTUAL "azimuth_actual_counts"
#define JSON_AZIMUTH_MANUAL "azimuth_manual"
#define JSON_AZIMUTH_TIMEOUT_TO_MIDDLE "azimuth_timeout_to_the_middle"
#define JSON_AZIMUTH_HOME "azimuth_home"
#define JSON_AZIMUTH_HOMING "azimuth_homing"


/*******************************************************************
 * Azimuth
 *******************************************************************/
extern void AZIMUTH_enable();
extern void AZIMUTH_disable();
extern bool AZIMUTH_enabled();

extern int AZIMUTH_get_low(void);
extern void AZIMUTH_set_low(int value);
extern int AZIMUTH_get_high(void);
extern void AZIMUTH_set_high(int value);
extern int AZIMUTH_get_middle(void);
extern void AZIMUTH_set_middle(int value);
extern int AZIMUTH_get_actual(void);

extern void AZIMUTH_set_steering(int value);
extern void AZIMUTH_set_output_manual(int value);

extern int AZIMUTH_get_timeout(void);
extern void AZIMUTH_set_timeout(int value);

extern void AZIMUTH_set_manual(int value);
extern int AZIMUTH_get_manual(void);

extern bool AZIMUTH_home();
extern void AZIMUTH_start_homing();

extern bool AZIMUTH_analog_enabled();
extern void AZIMUTH_analog_enable();
extern void AZIMUTH_analog_disable();

/*******************************************************************
 * Stops the azimuth movement.
 * 
 * This function disables the azimuth control and analog output.
*******************************************************************/
extern void AZIMUTH_stop();

/*******************************************************************
 * @brief Initializes the Azimuth module.
 * 
 * This function sets up the necessary configurations and resources 
 * for the Azimuth module. It should be called before using any 
 * other functions related to the Azimuth module.
*******************************************************************/
extern void AZIMUTH_setup();

/*******************************************************************
 * @brief Starts the azimuth process.
 *
 * This function initializes and starts the azimuth process.
 * It should be called before any other azimuth-related functions are used.
*******************************************************************/
extern void AZIMUTH_start();

#endif