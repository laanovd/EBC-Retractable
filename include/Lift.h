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
#define JSON_LIFT_HOME "lift_home"
#define JSON_LIFT_MOTOR_UP "lift_motor_up"
#define JSON_LIFT_MOTOR_DOWN "lift_motor_down"
#define JSON_LIFT_SENSOR_UP "lift_sensor_up"
#define JSON_LIFT_SENSOR_DOWN "lift_sensor_down"

/*******************************************************************
 * Storage keys and defaults
 *******************************************************************/
#define JSON_LIFT_MOVE_TIMEOUT "lift_move_timeout"
#define DELFAULT_LIFT_MOVE_TIMEOUT 120

#define JSON_RETRACTED_COUNT "retracted"
#define JSON_EXTENDED_COUNT "extended"

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
extern void LIFT_start_homing();

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
 * Stops the lift by disabling the lift motor and turning off the 
 * lift up and lift down signals.
 * 
 * Prints a message to the serial monitor indicating that the lift has stopped.
 *******************************************************************/
extern void LIFT_stop(void);

/*******************************************************************
 * @brief Initializes the lift setup by setting up variables and GPIO pins.
 * 
 * It also disables the lift and turns off the lift up and lift down signals.
 *******************************************************************/
extern void LIFT_setup(void);

/*******************************************************************
 * Starts the lift functionality.
 * 
 * This function sets up tasks, command-line interface (CLI), and 
 * API handlers for the lift. It also initializes the error mask and 
 * prints a message indicating that the lift has started.
 * 
 * @note This function should be called to start the lift.
 *******************************************************************/
extern void LIFT_start(void);

#endif // EBC_LIFT_HEADER