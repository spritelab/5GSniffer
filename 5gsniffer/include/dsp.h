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

#ifndef DSP_H
#define DSP_H

#include <complex>
#include <cstdint>
#include <vector>
#include <span>

using namespace std;

void correlate(vector<complex<float>>& output, span<complex<float>> a, span<complex<float>> b);
void correlate(vector<complex<float>>& output, span<complex<float>> a, span<complex<float>> b, int step_size);
void moving_correlate(vector<complex<float>>& output, span<complex<float>> a, span<complex<float>> b, size_t window_size);
void correlate_magnitude(vector<float>& output, span<complex<float>> a, span<complex<float>> b);
void correlate_magnitude(vector<float>& output, span<complex<float>> a, span<complex<float>> b, int step_size);
void correlate_magnitude_normalized(vector<float>& output, span<complex<float>> a, span<complex<float>> b);
void magnitude(vector<float>& output, span<complex<float>> input);
float frobenius_norm(span<complex<float>> input);
void rotate(vector<complex<float>>& output, span<complex<float>> input, float frequency, uint32_t sample_rate);


#endif // DSP_H