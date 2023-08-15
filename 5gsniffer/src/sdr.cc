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

#include <spdlog/spdlog.h>
#include "sdr.h"
#include "exceptions.h"

/** 
 * Constructor for sdr.
 *
 * @param sample_rate 
 * @param frequency 
 * @param rx_gain 
 * @param tx_gain 
 */
sdr::sdr(
  double sample_rate,
  double frequency,
  double rx_gain,
  double tx_gain
) : sample_rate(sample_rate),
    frequency(frequency),
    rx_gain(rx_gain),
    tx_gain(tx_gain) {
  SPDLOG_INFO("Starting SDR on {} at {} sps, RX gain {}, TX gain {}.", frequency, sample_rate, rx_gain, tx_gain);

  // Try to open device
  if (srsran_rf_open_devname(&rf, "", (char*)"", 1) == SRSRAN_ERROR) {
    throw sdr_exception("Failed to open SDR");
  }

  // Configure according to settings of the SDR object
  srsran_rf_set_rx_gain(&rf, rx_gain);
  srsran_rf_set_rx_srate(&rf, sample_rate);
  srsran_rf_set_rx_freq(&rf, 0, frequency);
  srsran_rf_start_rx_stream(&rf, false);

  secs_prev = 0;
  frac_secs_prev = 0;


}

/** 
 * Destructor for sdr.
 */
sdr::~sdr() {
  srsran_rf_stop_rx_stream(&rf);
  srsran_rf_close(&rf);
}

/** 
 * Receive a vector of samples from the opened SDR.
 *
 * @param num_samples number of samples to receive
 */
shared_ptr<vector<complex<float>>> sdr::receive(size_t num_samples) {
  SPDLOG_DEBUG("RX {0} samples ({1:.3f} ms)", num_samples, num_samples / this->sample_rate * 1000.0);

  // Create a shared pointer holding the buffer of num_samples samples
  shared_ptr<vector<complex<float>>> p = make_shared<vector<complex<float>>>(vector<complex<float>>(num_samples));
  time_t secs;
  double frac_secs;

  try {
    int num_received_samples = srsran_rf_recv_with_time(&rf, p.get()->data(), num_samples, 1, &secs, &frac_secs);
    SPDLOG_DEBUG("RF recv {} samples time secs {}, {}, diff with prev {},{}", num_samples, secs,frac_secs, secs - secs_prev, frac_secs - frac_secs_prev);
    secs_prev = secs;
    frac_secs_prev = frac_secs;
    // TODO Bug in srsRAN?: even successful trials are counted as erroneous ones. So if we reach 100 trials the function returns -1 and stops receiving even though the data looks good.
    if (num_received_samples != num_samples) {
      if (num_received_samples == -1) {
        throw sdr_exception("Error receiving samples");
      } else {
        stringstream ss;
        ss << "Requested " << num_samples << " samples but only got " << num_received_samples;
        throw sdr_exception(ss.str());
      }
    }
  } catch(sdr_exception& e) {
    SPDLOG_ERROR(e.what());
  }

  return p;
}

shared_ptr<vector<complex<float>>> sdr::produce_samples(size_t num_samples) {
  return this->receive(num_samples);
}
