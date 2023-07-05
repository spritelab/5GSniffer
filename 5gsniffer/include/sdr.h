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

#ifndef SDR_H
#define SDR_H

#include <vector>
#include <complex>
#include <memory>
#include "srsran/phy/rf/rf.h"
#include "worker.h"

using namespace std;

/**
 * Implementation of a SDR.
 *
 * The SDR class produces samples for other workers to process, using the srsRAN
 * RF libraries.
 */
class sdr : public worker {
  public:
    sdr(
      double sample_rate = 23'040'000,
      double frequency = 627'750'000,
      double rx_gain = 40.0,
      double tx_gain = 0.0
    );
    virtual ~sdr();
    shared_ptr<vector<complex<float>>> produce_samples(size_t num_samples) override;

  private:
    double sample_rate;
    double frequency;
    double rx_gain;
    double tx_gain;
    srsran_rf_t rf;

    shared_ptr<vector<complex<float>>> receive(size_t num_samples);

    time_t secs_prev;
    double frac_secs_prev;

};

#endif
