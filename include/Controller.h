/*******************************************************************
 *  Definitions and constants
 *
 *******************************************************************/
#ifndef CONTROLLER_HEADER
#define CONTROLLER_HEADER

#include <StateMachineLib.h>

/*******************************************************************
 * Requests maintenance mode for the controller.
 * 
 * This function is used to initiate a maintenance request for 
 * the controller statemachine. 
 *******************************************************************/
extern void CONTROLLER_request_maintenance(void);

/*******************************************************************
 * Setup and start
 *******************************************************************/
extern void CONTROLLER_setup();
extern void CONTROLLER_start();

#endif