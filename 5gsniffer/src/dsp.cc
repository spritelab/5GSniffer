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

#include "dsp.h"
#include "bandwidth_part.h"
#include <cmath>
#include <complex>
#include <cstdint>
#include <spdlog/spdlog.h>
#include <volk/volk.h>
#include <algorithm>

void correlate(vector<complex<float>>& output, span<complex<float>> a, span<complex<float>> b) {
  if (a.size() >= b.size()) {
    int64_t iterations = a.size() - b.size() + 1;
    output.resize(iterations);

    // Dot product at specific intervals
    for (size_t i = 0; i < iterations; ++i) {
      volk_32fc_x2_conjugate_dot_prod_32fc(output.data()+i, a.data()+i, b.data(), b.size());
    }
  } else {
    SPDLOG_ERROR("Invalid sizes for correlation: size of a must be >= b");
  }
}

void correlate(vector<complex<float>>& output, span<complex<float>> a, span<complex<float>> b, int step_size) {
  if (a.size() >= b.size()) {
    int64_t iterations = std::floor((a.size() - b.size() + 1)/step_size);
    output.resize(iterations);

    // Dot product at specific intervals
    for (size_t i = 0; i < iterations; ++i) {
      volk_32fc_x2_conjugate_dot_prod_32fc(output.data()+i, a.data()+(i*step_size), b.data(), b.size());
    }
  } else {
    SPDLOG_ERROR("Invalid sizes for correlation: size of a must be >= b");
  }
}


float frobenius_norm(span<complex<float>> input) {
  vector<float> magsq(input.size());
  float mag_sum = 0.0f;
  volk_32fc_magnitude_squared_32f(magsq.data(), input.data(), input.size());
  volk_32f_accumulator_s32f(&mag_sum, magsq.data(), magsq.size());
  return std::sqrt(mag_sum);
}

void correlate_magnitude_normalized(vector<float>& output, span<complex<float>> a, span<complex<float>> b) {
  if (a.size() >= b.size()) {
    int64_t iterations = a.size() - b.size() + 1;
    output.resize(iterations);

    float b_norm = frobenius_norm(b);
    for (size_t i = 0; i < iterations; ++i) {
      complex<float> dot_product = 0;
      volk_32fc_x2_conjugate_dot_prod_32fc(&dot_product, a.data()+i, b.data(), b.size());

      float a_norm = frobenius_norm({a.data()+i, b.size()});
      output.at(i) = std::abs(dot_product) / (a_norm * b_norm);
    }
  } else {
    SPDLOG_ERROR("Invalid sizes for correlation: size of a must be >= b");
  }
}

void moving_correlate(vector<complex<float>>& output, span<complex<float>> a, span<complex<float>> b, size_t window_size) {
  if (a.size() == b.size() && window_size <= a.size()) {
    size_t iterations = a.size();
    output.resize(iterations);

    if (window_size > INT_MAX) {
      SPDLOG_ERROR("Invalid window size");
      return;
    }

    int32_t window_size_int = static_cast<int32_t>(window_size);
    for(int32_t i = 0; i < iterations; i++) {
      int32_t start = std::max(i+1 - window_size_int, 0);
      int32_t length = std::min(i+1, window_size_int);
      volk_32fc_x2_conjugate_dot_prod_32fc(output.data()+i, a.data()+start, b.data()+start, length);
    }
  } else {
    SPDLOG_ERROR("Invalid sizes for correlation: size of a must be == b");
  }
}

void correlate_magnitude(vector<float>& output, span<complex<float>> a, span<complex<float>> b) {
  if (a.size() >= b.size()) {
    int64_t iterations = a.size() - b.size() + 1;
    output.resize(iterations);
    vector<complex<float>> correlations;

    // Regular cross correlation
    correlate(correlations, a, b);

    // Get the magnitude
    volk_32fc_magnitude_32f(output.data(), correlations.data(), correlations.size());
  } else {
    SPDLOG_ERROR("Invalid sizes for correlation: size of a must be >= b");
  }
}

void correlate_magnitude(vector<float>& output, span<complex<float>> a, span<complex<float>> b, int step_size) {
  if (a.size() >= b.size()) {
    int64_t iterations = std::floor((a.size() - b.size() + 1)/step_size);
    output.resize(iterations);
    vector<complex<float>> correlations;

    // Regular cross correlation
    correlate(correlations, a, b, step_size);

    // Get the magnitude
    volk_32fc_magnitude_32f(output.data(), correlations.data(), correlations.size());
  } else {
    SPDLOG_ERROR("Invalid sizes for correlation: size of a must be >= b");
  }
}

void magnitude(vector<float>& output, span<complex<float>> input) {
  volk_32fc_magnitude_32f(output.data(), input.data(), input.size());
}

void rotate(vector<complex<float>>& output, span<complex<float>> input, float frequency, uint32_t sample_rate) {
  float phase_rotation_per_t = (frequency * (2*std::numbers::pi)) / (float)sample_rate;
  complex<float> phase_start(1.0, 0.0);
  complex<float> complex_phase_rotation_per_t(std::cos(phase_rotation_per_t), std::sin(phase_rotation_per_t));

  volk_32fc_s32fc_x2_rotator_32fc(output.data(), input.data(), complex_phase_rotation_per_t, &phase_start, input.size()); 
}