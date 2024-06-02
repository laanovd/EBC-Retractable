/*******************************************************************
 *  Definitions and constants
 *
 *******************************************************************/
#ifndef CONTROLLER_HEADER
#define CONTROLLER_HEADER


/*******************************************************************
 *  Storage keys and defaults
 *******************************************************************/
#define JSON_RETRACTED_COUNT "retracted"
#define JSON_EXTENDED_COUNT "extended"

#define JSON_MOVE_TIMEOUT "move_timeout"
#define JSON_MOVE_TIMEOUT_DEFAULT 15

#define JSON_DELAY_TO_MIDDLE "delay_to_middle"
#define JSON_DELAY_TO_MIDDLE_DEFAULT 5

#define BLINK_INTERVAL_EMERGENCY 250
#define BLINK_INTERVAL_NO_POSITION 500
#define BLINK_INTERVAL_CALIBRATING 500
#define BLINK_INTERVAL_MOVING 500
#define BLINK_INTERVAL_HEARTBEAT 500

#endif