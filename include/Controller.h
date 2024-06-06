/*******************************************************************
 *  Definitions and constants
 *
 *******************************************************************/
#ifndef CONTROLLER_HEADER
#define CONTROLLER_HEADER

#include <StateMachineLib.h>

/*******************************************************************
 *  Storage keys and defaults
 *******************************************************************/
#define JSON_RETRACTED_COUNT "retracted"
#define JSON_EXTENDED_COUNT "extended"

#define JSON_MOVE_TIMEOUT "move_timeout"
#define JSON_MOVE_TIMEOUT_DEFAULT 20

#define JSON_DELAY_TO_MIDDLE "delay_to_middle"
#define JSON_DELAY_TO_MIDDLE_DEFAULT 5

#define DOUBLE_PRESS_HOLD_TIME 5000

extern void CONTROLLER_setup();

#endif