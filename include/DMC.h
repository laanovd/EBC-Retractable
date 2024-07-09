/*******************************************************************
 * DMC.h
 *    
 * EBC Dynamic Motor Control
 * 
 *******************************************************************/
#ifndef DMC_HEADER
#define DMC_HEADER

/*******************************************************************
 * RESTfull API keys
 *******************************************************************/
#define JSON_DMC_ENABLE "dmc_enable"
#define JSON_DMC_ENABLED "dmc_enabled"

/*******************************************************************
 * DMC enable
 *******************************************************************/
extern void DMC_enable();
extern void DMC_disable();
extern bool DMC_enabled();

/*******************************************************************
 * DMC general
 *******************************************************************/
extern void DMC_setup(void);
extern void DMC_start(void);

#endif