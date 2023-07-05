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

#ifndef ROTATOR_H
#define ROTATOR_H

#include <string>
#include <vector>
#include <complex>
#include <memory>
#include "worker.h"

using namespace std;

/**
 * A rotator is a worker that rotates the input samples according to specified
 * frequency.
 */
class rotator : public worker {
  public:
    rotator(uint32_t sample_rate, float frequency);
    void process(shared_ptr<vector<complex<float>>>& samples, int64_t metadata) override;
  private:
    float frequency;
    uint32_t sample_rate;
};

#endif // ROTATOR_H