#ifndef DMC_HEADER
#define DMC_HEADER

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