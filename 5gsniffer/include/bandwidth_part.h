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

#ifndef BANDWIDTH_PART_H
#define BANDWIDTH_PART_H

#include <cstdint>
#include <vector>
#include <memory>

using namespace std;

static constexpr uint32_t subframes_per_frame = 10;
static constexpr double seconds_per_subframe = 0.001;
static constexpr double seconds_per_frame = 0.010;
static constexpr double Tc = 1.0 / (480000.0 * 4096.0); ///< Basic time unit in 5G NR
static constexpr double K = 64.0;

static constexpr uint32_t symbols_per_slot_normal = 14;
static constexpr uint32_t symbols_per_slot_extended = 12;

/**
 * Class for holding 5G NR bandwidth part properties.
 */
class bandwidth_part {
  public:
    bandwidth_part(uint64_t sample_rate, uint8_t numerology, uint16_t num_prbs, bool extended_prefix = false);

    const uint8_t numerology;
    const uint32_t two_pow_numerology;
    const uint32_t scs;
    const uint32_t symbols_per_slot;
    const uint32_t slots_per_frame;
    const uint32_t symbols_per_frame;
    const uint32_t slots_per_subframe;
    const uint32_t symbols_per_subframe;
    const uint16_t num_prbs;
    const uint16_t num_subcarriers;
    uint64_t sample_rate;
    uint64_t fft_size;
    vector<double> seconds_per_slot;
    vector<double> seconds_per_symbol;
    vector<double> seconds_per_cp;

    uint64_t samples_per_symbol(uint64_t symbol_index_in_subframe);
    uint64_t samples_per_slot(uint64_t slot_index_in_subframe);
    uint64_t samples_per_cp(uint64_t symbol_index_in_subframe);

    std::array<uint8_t,4> get_pdcch_coreset0(uint16_t min_chann_bw, uint32_t ssb_scs, uint32_t pdcch_scs, uint8_t coreset0_idx);

  private:
    const bool extended_prefix;
};

#endif // BANDWIDTH_PART_H