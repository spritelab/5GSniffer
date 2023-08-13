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

#include "shifter.h"
#include "spdlog/spdlog.h"
#include <complex>

using namespace std;

/** 
 * Constructor for shifter.
 *
 * @param num_samples number of samples to shift
 */
shifter::shifter(int64_t num_samples) : 
  num_samples(num_samples) {
}

/** 
 * Shift the input samples.
 * @param samples shared_ptr to sample buffer
 */
void shifter::process(shared_ptr<vector<complex<float>>>& samples, int64_t metadata) {
  auto result = make_shared<vector<complex<float>>>(std::move(*samples));

  if(num_samples < 0) {
    if(abs(num_samples) <= result->size())
      result->erase(result->begin(), result->begin() + abs(num_samples));
  } else if (num_samples > 0) {
    vector<complex<float>> tmp(num_samples, 0);
    result->insert(result->begin(), tmp.begin(), tmp.end());
  }
  SPDLOG_DEBUG("Shifer block, shifting {} samples by num samples {}, metadata {}",samples->size(),num_samples, metadata);

  send_to_next_workers(result, metadata);
}