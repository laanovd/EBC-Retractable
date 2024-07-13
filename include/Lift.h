/*******************************************************************
 * Lift.h
 *    
 * EBC Retractable/lift control
 * 
 *******************************************************************/
#ifndef EBC_LIFT_HEADER
#define EBC_LIFT_HEADER

/*******************************************************************
 * JSON and Websocket keys
 *******************************************************************/
#define JSON_LIFT_ENABLED "lift_enabled"
#define JSON_LIFT_HOMING "lift_homing"
#define JSON_LIFT_MOTOR_UP "lift_motor_up"
#define JSON_LIFT_MOTOR_DOWN "lift_motor_down"
#define JSON_LIFT_SENSOR_UP "lift_sensor_up"
#define JSON_LIFT_SENSOR_DOWN "lift_sensor_down"

/*******************************************************************
 * Lift getters
 *******************************************************************/
extern int LIFT_move_timeout(void);

/*******************************************************************
 * Lift enable
 *******************************************************************/
extern void LIFT_enable(void);
extern void LIFT_disable(void);
extern bool LIFT_enabled(void);

/*******************************************************************
 * Lift motor
 *******************************************************************/
extern void LIFT_UP_on(void);
extern void LIFT_UP_off(void);
extern void LIFT_DOWN_on(void);
extern void LIFT_DOWN_off(void);
extern bool LIFT_UP_moving(void);
extern bool LIFT_DOWN_moving(void);

/*******************************************************************
 * Lift position
 *******************************************************************/
extern bool LIFT_UP_sensor(void);
extern bool LIFT_DOWN_sensor(void);
extern bool LIFT_HOME_sensor(void);

/*******************************************************************
 * LIFT counters
 *******************************************************************/
extern void LIFT_retected_increment(void);
extern void LIFT_extended_increment(void);

/*******************************************************************
 * Lift buttons
 *******************************************************************/
extern bool LIFT_UP_button(void);
extern bool LIFT_DOWN_button(void);
extern bool LIFT_BOTH_button(void);

/*******************************************************************
 * Lift general
 *******************************************************************/
extern void LIFT_setup(void);
extern void LIFT_start(void);

#endif // EBC_LIFT_HEADER