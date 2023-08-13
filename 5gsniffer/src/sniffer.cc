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

#include "sniffer.h"
#include "bandwidth_part.h"
#include "sdr.h"
#include "file_source.h"
#include "file_sink.h"
#include "spdlog/spdlog.h"
#include "phy_params_common.h"
#include "utils.h"
#include <cstdint>
#include <memory>

using namespace std;

/** 
 * Constructor for sniffer when using SDR.
 *
 * @param sample_rate
 * @param frequency
 */
sniffer::sniffer(uint64_t sample_rate, uint64_t frequency) :
  sample_rate(sample_rate),
  device(make_unique<sdr>(sample_rate, frequency)) {
  init();
}

/** 
 * Constructor for sniffer when using file source.
 *
 * @param sample_rate
 * @param path
 */
sniffer::sniffer(uint64_t sample_rate, string path) :
  sample_rate(sample_rate),
  device(make_unique<file_source>(sample_rate, path)) {
  init();
}

/** 
 * Common initializer helper function shared amongst constructors.
 */
void sniffer::init() {
  // Create blocks
  auto phy = make_shared<nr::phy>();  
  phy->ssb_bwp = make_unique<bandwidth_part>(3'840'000, 0, ssb_rb); // Default bandwidth part that captures at least 256 subcarriers (240 needed for SSB).
  auto syncer = make_shared<class syncer>(sample_rate, phy);

  // Callbacks
  device->on_end = std::bind(&sniffer::stop, this);

  device->connect(syncer);
}

void sniffer::start() {
  running = true;
  float seconds_per_chunk = 0.0080;
    
  uint32_t num_samples_per_chunk = static_cast<uint32_t>(sample_rate * seconds_per_chunk);

  while(running) {
    SPDLOG_DEBUG("Calling device work for SDR");

    auto sniffer_work_t0 = time_profile_start();
    device->work(num_samples_per_chunk);
    time_profile_end(sniffer_work_t0, "sniffer::work");
  }

  SPDLOG_DEBUG("Terminating sniffer");
}

void sniffer::stop() {
  SPDLOG_DEBUG("Received signal to stop sniffer");
  running = false;
}

/** 
 * Destructor for sniffer.
 */
sniffer::~sniffer() {
  
}