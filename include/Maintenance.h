/*******************************************************************
 *    Maintenace.h
 *
 *    Retractable maintenance mode
 *
 *******************************************************************/
#ifndef MAINTENANCE_H
#define MAINTENANCE_H

#include <ArduinoJson.h>


/********************************************************************
 * Enables maintenance mode if the emergency stop is not active.
 * 
 * This function enables maintenance mode if the emergency stop is not active.
 * The maintenance mode allows performing maintenance tasks on the system.
 * 
 * @note If the emergency stop is active, the maintenance mode will not be enabled.
 *******************************************************************/
extern void MAINTENANCE_enable(void);

/********************************************************************
 * Disables the maintenance mode.
 * 
 * This function disables maintenance mode and disables 
 * the DMC, LIFT, and AZIMUTH subsystems.
 *******************************************************************/
extern void MAINTENANCE_disable(void);

/********************************************************************
 * Check if maintenance mode is enabled.
 *
 * @return true if maintenance mode is enabled, false otherwise.
 *******************************************************************/
extern bool MAINTENANCE_enabled(void);

/********************************************************************
 * Command handler
 *******************************************************************/
extern int MAINTENANCE_command_handler(const char *data);

/********************************************************************
 * Setup and start
 *******************************************************************/
extern void MAINTENANCE_setup(void);
extern void MAINTENANCE_start(void);

#endif // MAINTENANCE_H