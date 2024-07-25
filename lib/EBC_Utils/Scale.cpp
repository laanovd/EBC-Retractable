/*******************************************************************
 * Scale.cpp
 *
 * Miscellaneous utility functions.
 *
 *******************************************************************/
#include <Arduino.h>

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
float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
  if ((in_max - in_min) != 0) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  }
  return 0.0;
}

/*******************************************************************
 * Maps a value from one range to another range.
 *
 * This function takes an input value `x` and maps it from the input range defined by `in_min` and `in_max`
 * to the output range defined by `out_min` and `out_max`. The mapped value is then returned.
 *
 * @param x The value to be mapped.
 * @param in_min The minimum value of the input range.
 * @param in_max The maximum value of the input range.
 * @param out_min The minimum value of the output range.
 * @param out_max The maximum value of the output range.
 * @return The mapped value.
 *******************************************************************/
long mapl(long x, long in_min, long in_max, long out_min, long out_max) {
  if ((in_max - in_min) != 0) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  }
  return 0L;
}


