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

#ifndef OFDM_H
#define OFDM_H

#include <cstdint>
#include <memory>
#include <vector>
#include <complex>
#include <liquid/liquid.h>
#include "worker.h"
#include "bandwidth_part.h"

using namespace std;

/**
 * Class for OFDM modulation and demodulation.
 */
class ofdm : public worker {
  public:
    uint8_t symbol_index; ///< Index of the current symbol in the subframe
    uint8_t slot_index;   ///< Index of the current slot in the frame
    
    ofdm(shared_ptr<bandwidth_part> bwp, float cyclic_prefix_fraction = 0.5);
    virtual ~ofdm();
    void process(shared_ptr<vector<complex<float>>>& samples, int64_t metadata) override;
    vector<complex<float>> modulate(vector<symbol>& symbols);
    void reset() override;
  private:
    shared_ptr<bandwidth_part> bwp;
    float cyclic_prefix_fraction;
    uint64_t samples_processed;
    std::vector<std::complex<float>> leftover_samples;


    string get_symbol_dump_path_name();
};


#endif // OFDM_H