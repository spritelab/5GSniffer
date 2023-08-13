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

#include "file_source.h"
#include "spdlog/spdlog.h"
#include <cstdint>

using namespace std;

/** 
 * Constructor for file_source.
 *
 * @param path path to the file to read
 * @param sample_rate sample rate at which the file was recorded
 */
file_source::file_source(uint64_t sample_rate, string path, bool repeat) :
  repeat(repeat),
  f{path, ifstream::binary},
  size_bytes(0) {
  SPDLOG_DEBUG("Opening file_source from {} ({} sps)", path, sample_rate);
  if(f) {
    f.seekg(0, f.end);
    this->size_bytes = f.tellg();
    f.seekg(0, f.beg);
    SPDLOG_DEBUG("Size of file_source is {}", this->size_bytes);
  } else {
    throw sniffer_exception("File could not be opened");
  }
}

/** 
 * Destructor for file_source.
 */
file_source::~file_source() {
  f.close();
}

/** 
 * Reads vector of num_samples complex samples held in shared_ptr from a file.
 *
 * @param num_samples number of samples to read
 */
shared_ptr<vector<complex<float>>> file_source::produce_samples(size_t num_samples) {
  vector<complex<float>> buffer(num_samples);

  f.read(reinterpret_cast<char*>(buffer.data()), num_samples * sizeof(complex<float>));

  if(f.eof()) {
    buffer.resize(f.gcount() / sizeof(complex<float>)); // Resize buffer to the number of samples that were read successfully

    if(repeat) {
      f.seekg(0, f.beg);
    } else {
      this->on_end();
    }
  }

  size_t size_bytes = buffer.size() * sizeof(complex<float>);
  SPDLOG_DEBUG("Read {} samples ({} bytes)", buffer.size(), size_bytes);
  total_produced_samples += buffer.size();

  return make_shared<vector<complex<float>>>(std::move(buffer));
}