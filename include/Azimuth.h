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
#define JSON_AZIMUTH_LEFT_DEFAULT 0

#define JSON_AZIMUTH_RIGHT "azimuth_right"
#define JSON_AZIMUTH_RIGHT_DEFAULT 5

extern void AZIMUTH_update();
extern void AZIMUTH_enable();
extern void AZIMUTH_disable();
extern void AZIMUTH_setup();

#endif