/*******************************************************************
 * Azimuth.h
 *    
 * EBC azimuth control
 * 
 *******************************************************************/
#ifndef AZIMUTH_HEADER
#define AZIMUTH_HEADER

extern void AZIMUTH_update();
extern void AZIMUTH_enable();
extern void AZIMUTH_disable();
extern bool AZIMUTH_enabled();


extern float AZIMUH_get_actual(void);
extern float AZIMUH_get_left(void);
extern void AZIMUH_set_left(float value);
extern float AZIMUH_get_right(void);
extern void AZIMUH_set_right(float value);
extern int AZIMUTH_get_steering(void);
extern void AZIMUTH_set_steering(int value);

extern int STEERING_WHEEL_get_position(void);

extern int AZIMUTH_to_the_middle_delay(void);

extern void AZIMUTH_setup();
extern void AZIMUTH_start();

#endif