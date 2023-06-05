/*
 * Copyright (c) 2021.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 */

#include "rotator.h"
#include "spdlog/spdlog.h"
#include "dsp.h"

using namespace std;

/** 
 * Constructor for rotator.
 *
 * @param frequency frequency to rotate
 */
rotator::rotator(uint32_t sample_rate, float frequency) : 
  frequency(frequency),
  sample_rate(sample_rate) {
}

/** 
 * Rotate the input samples.
 * @param samples shared_ptr to sample buffer
 */
void rotator::process(shared_ptr<vector<complex<float>>>& samples, int64_t metadata) {
  rotate(*samples, *samples, frequency, sample_rate);
  send_to_next_workers(samples, metadata);
}