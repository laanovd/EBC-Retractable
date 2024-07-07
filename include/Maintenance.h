/*******************************************************************
 *    Maintenace.h
 *
 *    Retractable maintenance mode
 *
 *******************************************************************/
#ifndef MAINTENACE_H
#define MAINTENACE_H

/********************************************************************
 * Manitenace mode started
 *******************************************************************/
extern bool MAINTENANCE_enabled(void);
extern void MAINTENANCE_disable(void);
extern void MAINTENANCE_enable(void);
extern bool MAINTENANCE_activate(void);

/********************************************************************
 * Setup
 *******************************************************************/
extern void MAINTENANCE_setup(void);
extern void MAINTENANCE_start(void);

#endif // MAINTENACE_H