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

#ifndef SHIFTER_H
#define SHIFTER_H

#include <cstdint>
#include <string>
#include <vector>
#include <complex>
#include <memory>
#include "worker.h"

using namespace std;

/**
 * A shifter is a worker that performs a phase shift on the input.
 */
class shifter : public worker {
  public:
    shifter(int64_t num_samples);
    void process(shared_ptr<vector<complex<float>>>& samples, int64_t metadata) override;
  private:
    int64_t num_samples;
};

#endif // SHIFTER_H