/*******************************************************************
 * test_lift.cpp
 *
 * EBC Retractable lift unittest
 *
 *******************************************************************/
#include <unity.h>
#include "Lift.h"
#include "Storage.h"

/*******************************************************************
 * SUITE SetUp
 *******************************************************************/
void setUp(void) {
  // set stuff up here
  TEST_ASSERT_TRUE(I2C_setup(SDA0, SCL0));
}

/*******************************************************************
 * SUITE TearDown
 *******************************************************************/
void tearDown(void) {
  // clean stuff up here
}

/*******************************************************************
 * TC Initialise variables
 *******************************************************************/
void test_LIFT_setup_variables(void) {
  // Set initial values
  STORAGE_set_int(JSON_LIFT_MOVE_TIMEOUT, 10);
  STORAGE_set_int(JSON_RETRACTED_COUNT, 5);
  STORAGE_set_int(JSON_EXTENDED_COUNT, 3);

  // Call the function
  LIFT_setup_variables();

  // Check if the values are reset to default
  TEST_ASSERT_EQUAL(30, LIFT_move_timeout());
  TEST_ASSERT_EQUAL(0, LIFT_data[JSON_RETRACTED_COUNT].as<int>());
  TEST_ASSERT_EQUAL(0, LIFT_data[JSON_EXTENDED_COUNT].as<int>());
}

/*******************************************************************
 * TC Lift move timeout
 *******************************************************************/
void test_LIFT_move_timeout(void) {
  // Set the move timeout to 20
  STORAGE_set_int(JSON_LIFT_MOVE_TIMEOUT, 20);

  // Call the function
  int timeout = LIFT_move_timeout();

  // Check if the timeout is correct
  TEST_ASSERT_EQUAL(20, timeout);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_LIFT_setup_variables);
  RUN_TEST(test_LIFT_move_timeout);
  UNITY_END();

  return 0;
}
