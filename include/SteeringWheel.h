/*******************************************************************
 * Azimuth.h
 *    
 * EBC azimuth control
 * 
 *******************************************************************/
#ifndef STEERINGWHEEL_HEADER
#define STEERINGWHEEL_HEADER

/*******************************************************************
 * JSON and Websocket keys
 *******************************************************************/
#define JSON_STEERWHEEL_LEFT "steering_left_counts"
#define JSON_STEERWHEEL_RIGHT "steering_right_counts"
#define JSON_STEERWHEEL_MIDDLE "steering_middle_counts"

#define JSON_STEERWHEEL_ACTUAL "steering_actual_counts"

#define JSON_STEERWHEEL_DEADBAND "steering_middle_deadband"

/*******************************************************************
 * Steering wheel
 *******************************************************************/
extern int STEERWHEEL_get_left(void);
extern void STEERWHEEL_set_left(int value);
extern int STEERWHEEL_get_right(void);
extern void STEERWHEEL_set_right(int value);
extern int STEERWHEEL_get_middle(void);
extern void STEERWHEEL_set_middle(int value);
extern int STEERWHEEL_get_actual(void);
extern int STEERWHEEL_get_deadband(void);
extern void STEERWHEEL_set_deadband(int value);


/********************************************************************
 * @brief Saves the calibration values for the steering wheel.
 *
 * This function retrieves the calibration values from the `doc` JSON object
 * and stores them using the `STORAGE_get_int` function.
 *******************************************************************/
extern void STEERWHEEL_calibration_save(void);

/********************************************************************
 * @brief Restores the calibration values for the steering wheel.
 * 
 * This function retrieves the calibration values for the left, right, 
 * and middle positions of the steering wheel from the storage and 
 * sets them using the corresponding setter functions.
 * 
 * @note This function assumes that the calibration values have 
 *       been previously stored in the storage.
 * 
 * @see STEERWHEEL_set_left
 * @see STEERWHEEL_set_right
 * @see STEERWHEEL_set_middle
 *******************************************************************/
extern void STEERWHEEL_calibration_restore(void);

/********************************************************************
 * Stops the steering wheel and aborts any ongoing calibration process.
 *******************************************************************/
extern void STEERINGWHEEL_stop();

/********************************************************************
 * @brief Sets up the steering wheel.
 *
 * This function initializes the necessary components and 
 * configurations for the steering wheel. It should be called once 
 * during the setup phase of the program.
 *******************************************************************/
extern void STEERINGWHEEL_setup();

/********************************************************************
 * @brief Starts the steering wheel functionality.
 *
 * This function initializes and starts the steering wheel 
 * functionality. It should be called before using any other 
 * steering wheel related functions.
 *******************************************************************/
extern void STEERINGWHEEL_start();

#endif // STEERINGWHEEL_HEADER