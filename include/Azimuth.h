/*******************************************************************
 *  Definitions and constants
 *
 *******************************************************************/
#ifndef AZIMUTH_HEADER
#define AZIMUTH_HEADER

/*******************************************************************
 *  Storage keys and defaults
 *******************************************************************/
#define JSON_AZIMUTH_LEFT "azimuth_left"
#define JSON_AZIMUTH_LEFT_DEFAULT 0.0

#define JSON_AZIMUTH_RIGHT "azimuth_right"
#define JSON_AZIMUTH_RIGHT_DEFAULT 5.0

#define JSON_AZIMUTH_ACTUAL "azimuth_actual"
#define JSON_AZIMUTH_STEERING "azimuth_steering"

extern void AZIMUTH_update();
extern void AZIMUTH_enable();
extern void AZIMUTH_disable();
extern void AZIMUTH_setup();


extern float AZIMUH_get_actual(void);
extern float AZIMUH_get_left(void);
extern float AZIMUH_get_right(void);
extern int AZIMUTH_get_steering(void);

extern void AZIMUTH_set_left(float value);
extern void AZIMUTH_set_right(float value);

#endif