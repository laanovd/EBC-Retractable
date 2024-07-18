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
#define JSON_AZIMUTH_LEFT "azimuth_left_counts"
#define JSON_AZIMUTH_RIGHT "azimuth_right_counts"
#define JSON_AZIMUTH_ACTUAL "azimuth_actual_counts"
#define JSON_AZIMUTH_MANUAL "azimuth_manual"
#define JSON_DELAY_TO_MIDDLE "steering_delay_to_the_middle"
#define JSON_AZIMUTH_HOME "azimuth_home"
#define JSON_AZIMUTH_HOMING "azimuth_homing"

#define JSON_STEERWHEEL_START_CALIBRATION "steering_start_calibration"
#define JSON_STEERWHEEL_LEFT "steering_left_counts"
#define JSON_STEERWHEEL_RIGHT "steering_right_counts"
#define JSON_STEERWHEEL_MIDDLE "steering_middle_counts"
#define JSON_STEERWHEEL_ACTUAL "steering_actual_counts"

/*******************************************************************
 * Azimuth
 *******************************************************************/
extern void AZIMUTH_update();
extern void AZIMUTH_enable();
extern void AZIMUTH_disable();
extern bool AZIMUTH_enabled();

extern int AZIMUTH_get_left(void);
extern void AZIMUTH_set_left(int value);
extern int AZIMUTH_get_right(void);
extern void AZIMUTH_set_right(int value);
extern int AZIMUTH_get_actual(void);

extern void AZIMUTH_set_steering(int value);
extern void AZIMUTH_set_output_manual(int value);

extern int AZIMUTH_to_the_middle_delay(void);

extern void AZIMUTH_set_manual(int value);
extern int AZIMUTH_get_manual(void);

extern bool AZIMUTH_home();
extern void AZIMUTH_start_homing();

extern bool AZIMUTH_analog_enabled();
extern void AZIMUTH_analog_enable();
extern void AZIMUTH_analog_disable();

extern void AZIMUTH_setup();
extern void AZIMUTH_start();

/*******************************************************************
 * Steering wheel
 *******************************************************************/
extern int STEERWHEEL_get_left(void);
extern void STEERWHEEL_set_left(int value);
extern int STEERWHEEL_get_right(void);
extern void STEERWHEEL_set_right(int value);
extern int STEERWHEEL_get_middle(void);
extern void STEERWHEEL_set_middle(int value);
extern int STEERWHEEL_get_actual(void);

#endif