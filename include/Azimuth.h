/*******************************************************************
 * Azimuth.h
 *    
 * EBC azimuth control
 * 
 *******************************************************************/
#ifndef AZIMUTH_HEADER
#define AZIMUTH_HEADER

/*******************************************************************
 * RESTfull API keys
 *******************************************************************/
#define JSON_AZIMUTH_ENABLE "steering_enable"
#define JSON_AZIMUTH_ENABLED "steering_enabled"
#define JSON_AZIMUTH_HOME "steering_home"
#define JSON_AZIMUTH_LEFT_V "steering_left_volt"
#define JSON_AZIMUTH_RIGHT_V "steering_right_volt"
#define JSON_AZIMUTH_ACTUAL_V "steering_output_volt"
#define JSON_AZIMUTH_MANUAL "steering_manual"
#define JSON_AZIMUTH_STEERING "steering_steer"
#define JSON_AZIMUTH_OUTPUT_ENABLE "steering_analog_out_enable"
#define JSON_AZIMUTH_OUTPUT_ENABLED "steering_analog_out_enabled"
#define JSON_DELAY_TO_MIDDLE "azimuth_delay_to_the_middle"

/*******************************************************************
 * Azimuth
 *******************************************************************/
extern void AZIMUTH_update();
extern void AZIMUTH_enable();
extern void AZIMUTH_disable();
extern bool AZIMUTH_enabled();

extern float AZIMTUH_get_actual(void);
extern float AZIMTUH_get_left(void);
extern void AZIMTUH_set_left(float value);
extern float AZIMTUH_get_right(void);
extern void AZIMTUH_set_right(float value);
extern void AZIMUTH_set_steering(int value);

extern int AZIMUTH_read_steer(void);

extern int AZIMUTH_to_the_middle_delay(void);

extern void AZIMUTH_set_manual(int value);
extern int AZIMUTH_get_manual(void);

extern int AZIMUTH_get_wheel(void);

extern bool AZIMUTH_home();
extern void AZIMUTH_start_homing();

extern bool AZIMUTH_output_enabled();
extern void AZIMUTH_output_enable();
extern void AZIMUTH_output_disable();

extern void AZIMUTH_setup();
extern void AZIMUTH_start();

#endif