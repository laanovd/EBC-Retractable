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
 * Manitenace mode started
 *******************************************************************/
extern bool MAINTENANCE_enabled(void);
extern void MAINTENANCE_disable(void);
extern void MAINTENANCE_enable(void);
// extern bool MAINTENANCE_activate(void);

/********************************************************************
 * Setup
 *******************************************************************/
extern void MAINTENANCE_setup(void);
extern void MAINTENANCE_start(void);

/********************************************************************
 * Command handler
 *******************************************************************/
extern DeserializationError MAINTENANCE_command_handler(char *data);

#endif // MAINTENANCE_H