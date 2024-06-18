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

#include <cstdint>
#include <vector>
#include <complex>
#include "gtest/gtest.h"
#include "dsp.h"

using namespace std;

class dsp_test : public ::testing::Test {
 protected:
  dsp_test() {
  }
};

TEST_F(dsp_test, correlation_magnitude_normalized) {
  vector<complex<float>> ref = {1+1j, 1+0j, 0+1j, 0+0.1j};
  vector<complex<float>> inc_mag_but_corr = {4+4j, 4+0j, 0+4j, 0+0.4j};
  vector<complex<float>> inc_mag_but_inv_corr = {-4-4j, -4+0j, 0-4j, 0-0.4j};
  vector<complex<float>> random_corr = {1-5j, 5+0j, -12-5j, 1-4j};

  vector<float> result;

  correlate_magnitude_normalized(result, ref, inc_mag_but_corr);
  EXPECT_FLOAT_EQ(result.at(0), 0.9999999999999998);
  correlate_magnitude_normalized(result, ref, inc_mag_but_inv_corr);
  EXPECT_FLOAT_EQ(result.at(0), 0.9999999999999998);
  correlate_magnitude_normalized(result, ref, random_corr);
  EXPECT_FLOAT_EQ(result.at(0), 0.238744325750948);
}
