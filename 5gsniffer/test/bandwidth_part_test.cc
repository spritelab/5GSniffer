#include "gtest/gtest.h"
#include <cstdint>
#include "bandwidth_part.h"

class bandwidth_part_test : public ::testing::Test {
 protected:
  bandwidth_part_test() {
  }
};

TEST_F(bandwidth_part_test, symbol_lengths) {
  bandwidth_part num_zero(3'840'000, 0, 20, false);
  bandwidth_part num_two(15'360'000, 2, 20, false);

  EXPECT_EQ(num_zero.samples_per_cp(0), 20);
  EXPECT_EQ(num_zero.samples_per_cp(1), 18);
  EXPECT_EQ(num_zero.samples_per_cp(7), 20);
  
  EXPECT_FLOAT_EQ(num_zero.seconds_per_symbol[0], 0.000071875L);
  EXPECT_FLOAT_EQ(num_zero.seconds_per_symbol[1], 0.000071354167L);
  EXPECT_FLOAT_EQ(num_two.seconds_per_symbol[0], 0.000018359375L);
  EXPECT_FLOAT_EQ(num_two.seconds_per_symbol[28], 0.000018359375L);
  EXPECT_FLOAT_EQ(num_two.seconds_per_symbol[29], 0.000017838542L);
}
