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
  EXPECT_FLOAT_EQ(result.at(0), -0.9999999999999998);
  correlate_magnitude_normalized(result, ref, random_corr);
  EXPECT_FLOAT_EQ(result.at(0), -0.14272705908405414);
}
