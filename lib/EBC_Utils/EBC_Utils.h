/*******************************************************************
 * EBC_Utils.h
 *
 * Miscellaneous utility functions.
 *
 *******************************************************************/
#ifndef EBC_UTILS_HEADER
#define EBC_UTILS_HEADER

/*******************************************************************
 * Maps a value from one range to another range.
 *
 * This function takes an input value `x` and maps it from the input 
 * range defined by `in_min` and `in_max` to the output range defined 
 * by `out_min` and `out_max`. The mapped value is then returned.
 *
 * @param x The value to be mapped.
 * @param in_min The minimum value of the input range.
 * @param in_max The maximum value of the input range.
 * @param out_min The minimum value of the output range.
 * @param out_max The maximum value of the output range.
 * @return The mapped value.
 *******************************************************************/
extern long mapl(long x, long in_min, long in_max, long out_min, long out_max);

/*******************************************************************
 * Maps a value from one range to another range.
 *
 * @param x The value to be mapped.
 * @param in_min The minimum value of the input range.
 * @param in_max The maximum value of the input range.
 * @param out_min The minimum value of the output range.
 * @param out_max The maximum value of the output range.
 * @return The mapped value.
 *******************************************************************/
extern float mapf(float x, float in_min, float in_max, float out_min, float out_max);

#endif // EBC_UTILS_HEADER