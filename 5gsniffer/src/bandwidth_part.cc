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

#include <cmath>
#include <cstdint>
#include "bandwidth_part.h"
#include "spdlog/spdlog.h"
#include "phy_params_common.h"
#include <iostream>


using namespace std;


/** 
* Constructor for bandwidth_part.
*/
bandwidth_part::bandwidth_part(uint64_t sample_rate, uint8_t numerology, uint16_t num_prbs, bool extended_prefix) :
  sample_rate(sample_rate),
  numerology(numerology),
  two_pow_numerology(1 << numerology),
  scs(subcarrier_spacing_15khz * two_pow_numerology),
  symbols_per_slot(extended_prefix ? symbols_per_slot_extended : symbols_per_slot_normal),
  slots_per_frame(subframes_per_frame * two_pow_numerology),
  symbols_per_frame(slots_per_frame * symbols_per_slot),
  slots_per_subframe(two_pow_numerology),
  symbols_per_subframe(slots_per_subframe * symbols_per_slot),
  extended_prefix(extended_prefix),
  num_prbs(num_prbs),
  num_subcarriers(num_prbs * 12) {

  assert(numerology <= max_numerology);

  // Calculate minimum FFT size
  fft_size = sample_rate / scs;
  assert(fft_size >= num_subcarriers);    

  // TS 38 211 Sec. 5.3.1
  double useful_length = 2048.0 * K * 1.0 / two_pow_numerology;
  double normal_cp_length = 144.0 * K * 1.0 / two_pow_numerology;
  double extended_cp_length = 512.0 * K * 1.0 / two_pow_numerology;
  double useful_duration = useful_length * Tc;
  double normal_cp_duration = normal_cp_length * Tc;
  double normal_cp_duration_long = (normal_cp_length + (16.0 * K)) * Tc;
  double extended_cp_duration = extended_cp_length * Tc;

  if (!extended_prefix) {
    // Initialize with normal cyclic prefix durations
    seconds_per_cp.resize(symbols_per_subframe, normal_cp_duration);
    seconds_per_symbol.resize(symbols_per_subframe, normal_cp_duration + useful_duration);

    // At indices l = 0 and l = 7*2^u in the subframe we have a longer cyclic prefix
    seconds_per_cp[0] = normal_cp_duration_long;
    seconds_per_symbol[0] = normal_cp_duration_long + useful_duration;
    seconds_per_cp[7*two_pow_numerology] = normal_cp_duration_long;
    seconds_per_symbol[7*two_pow_numerology] = normal_cp_duration_long + useful_duration;
  } else {
    // Initialize with extended cyclic prefix durations
    seconds_per_cp.resize(symbols_per_subframe, extended_cp_duration);
    seconds_per_symbol.resize(symbols_per_subframe, extended_cp_duration + useful_duration);
  }
}

uint64_t bandwidth_part::samples_per_symbol(uint64_t symbol_index_in_subframe) {
  if (symbol_index_in_subframe < seconds_per_symbol.size())
    return floor(seconds_per_symbol.at(symbol_index_in_subframe) * sample_rate);
  throw std::out_of_range("Tried to access symbol index outside subframe bounds");
}

uint64_t bandwidth_part::samples_per_slot(uint64_t slot_index_in_subframe) {
  if (slot_index_in_subframe < slots_per_subframe) {
    double result = 0.0;
    for (int64_t i = 0; i < symbols_per_slot; i++) {
      result += seconds_per_symbol.at((slot_index_in_subframe*symbols_per_slot) + i);
    }
    return result * sample_rate;
  }
  throw std::out_of_range("Tried to access slot index outside subframe bounds");
}

uint64_t bandwidth_part::samples_per_cp(uint64_t symbol_index_in_subframe) {
  if (symbol_index_in_subframe < seconds_per_cp.size())
    return floor(seconds_per_cp.at(symbol_index_in_subframe) * sample_rate);
  throw std::out_of_range("Tried to access symbol index outside subframe bounds");
}
