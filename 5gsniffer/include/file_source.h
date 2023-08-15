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

#ifndef FILE_SOURCE_H
#define FILE_SOURCE_H

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>
#include <complex>
#include <memory>
#include "worker.h"

using namespace std;

/**
 * A sample_worker that produces complex samples by reading them from a file.
 */
class file_source : public worker {
  public:
    int size_bytes;
    uint64_t sample_rate;

    file_source(uint64_t sample_rate, string path, bool repeat = false);
    virtual ~file_source();
    shared_ptr<vector<complex<float>>> produce_samples(size_t num_samples) override;
  private:
    ifstream f;
    bool repeat;
};

#endif