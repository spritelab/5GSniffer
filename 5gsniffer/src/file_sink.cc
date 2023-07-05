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

#include "file_sink.h"
#include "spdlog/spdlog.h"

using namespace std;

/** 
 * Constructor for file_sink.
 *
 * @param path path to the file to write
 */
file_sink::file_sink(string path) :
  f{path, ofstream::binary} {
    SPDLOG_DEBUG("Opening file_sink to {}", path);
}

/** 
 * Destructor for file_sink.
 */
file_sink::~file_sink() {
  f.close();
}

/** 
 * Writes vector of complex samples held in shared_ptr to a file.
 *
 * @param samples shared_ptr to sample buffer to write
 */
void file_sink::process(shared_ptr<vector<complex<float>>>& samples, int64_t metadata) {
  size_t size_bytes = samples->size() * sizeof(complex<float>);
  f.write(reinterpret_cast<char*>(samples->data()), size_bytes);
  SPDLOG_DEBUG("Wrote {} samples ({} bytes)", samples->size(), size_bytes);
}