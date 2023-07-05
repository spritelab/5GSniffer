/**
 * Copyright 2022-2023 SpriteLab @ Northeastern University
 *
 * This file is part of 5GSniffer.
 *
 * 5GSniffer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * 5GSniffer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 */

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
