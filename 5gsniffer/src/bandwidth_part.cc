/*
 * Copyright (c) 2021.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
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


// CORESET0 Tables, provisionally here.

std::array<uint8_t,4> bandwidth_part::get_pdcch_coreset0(uint16_t min_chann_bw, uint32_t ssb_scs, uint32_t pdcch_scs, uint8_t coreset0_idx)
{
  // TS38.213 Table 13-1. {SS/PBCH block, PDCCH} SCS is {15, 15} kHz and minimum channel bandwidth 5 MHz or 10 MHz.
  static const std::array<std::array<uint8_t,4>, 15> TABLE_13_1 = {{
      {1, 24, 2, 0},
      {1, 24, 2, 2},
      {1, 24, 2, 4},
      {1, 24, 3, 0},
      {1, 24, 3, 2},
      {1, 24, 3, 4},
      {1, 48, 1, 12},
      {1, 48, 1, 16},
      {1, 48, 2, 12},
      {1, 48, 2, 16},
      {1, 48, 3, 12},
      {1, 48, 3, 16},
      {1, 96, 1, 38},
      {1, 96, 2, 38},
      {1, 96, 3, 38},
  }};
  // TS38.213 Table 13-2. {SS/PBCH block, PDCCH} SCS is {15, 30} kHz and minimum channel bandwidth 5 MHz or 10 MHz.
  static const std::array<std::array<uint8_t,4>, 14> TABLE_13_2 = {{
      {1, 24, 2, 5},
      {1, 24, 2, 6},
      {1, 24, 2, 7},
      {1, 24, 2, 8},
      {1, 24, 3, 5},
      {1, 24, 3, 6},
      {1, 24, 3, 7},
      {1, 24, 3, 8},
      {1, 48, 1, 18},
      {1, 48, 1, 20},
      {1, 48, 2, 18},
      {1, 48, 2, 20},
      {1, 48, 3, 18},
      {1, 48, 3, 20},
  }};
  // TS38.213 Table 13-3. {SS/PBCH block, PDCCH} SCS is {30, 15} kHz and minimum channel bandwidth 5 MHz or 10 MHz.
  static const std::array<std::array<uint8_t,4>, 9> TABLE_13_3 = {{
      {1, 48, 1, 2},
      {1, 48, 1, 6},
      {1, 48, 2, 2},
      {1, 48, 2, 6},
      {1, 48, 3, 2},
      {1, 48, 3, 6},
      {1, 96, 1, 28},
      {1, 96, 2, 28},
      {1, 96, 3, 28},
  }};
  // TS38.213 Table 13-4. {SS/PBCH block, PDCCH} SCS is {30, 30} kHz and minimum channel bandwidth 5 MHz or 10 MHz.
  static const std::array<std::array<uint8_t,4>, 16> TABLE_13_4 = {{
      {1, 24, 2, 0},
      {1, 24, 2, 1},
      {1, 24, 2, 2},
      {1, 24, 2, 3},
      {1, 24, 2, 4},
      {1, 24, 3, 0},
      {1, 24, 3, 1},
      {1, 24, 3, 2},
      {1, 24, 3, 3},
      {1, 24, 3, 4},
      {1, 48, 1, 12},
      {1, 48, 1, 14},
      {1, 48, 1, 16},
      {1, 48, 2, 12},
      {1, 48, 2, 14},
      {1, 48, 2, 16},
  }};
  // TS38.213 Table 13-5. {SS/PBCH block, PDCCH} SCS is {30, 15} kHz and minimum channel bandwidth 40 MHz.
  static const std::array<std::array<uint8_t,4>, 9> TABLE_13_5 = {{
      {1, 48, 1, 4},
      {1, 48, 2, 4},
      {1, 48, 3, 4},
      {1, 96, 1, 0},
      {1, 96, 1, 56},
      {1, 96, 2, 0},
      {1, 96, 2, 56},
      {1, 96, 3, 0},
      {1, 96, 3, 56},
  }};
  // TS38.213 Table 13-6. {SS/PBCH block, PDCCH} SCS is {30, 30} kHz and minimum channel bandwidth 40 MHz.
  static const std::array<std::array<uint8_t,4>, 10> TABLE_13_6 = {{
      {1, 24, 2, 0},
      {1, 24, 2, 4},
      {1, 24, 3, 0},
      {1, 24, 3, 4},
      {1, 48, 1, 0},
      {1, 48, 1, 28},
      {1, 48, 2, 0},
      {1, 48, 2, 28},
      {1, 48, 3, 0},
      {1, 48, 3, 28},
  }};

  std::array<uint8_t,4> coreset0_config = {0,0,0,0};

  if (min_chann_bw == 5 || min_chann_bw == 10){
    if (ssb_scs == 15000 && pdcch_scs == 15000){
      coreset0_config = TABLE_13_1.at(coreset0_idx);

    } else if (ssb_scs == 15000 && pdcch_scs == 30000){
      coreset0_config = TABLE_13_2.at(coreset0_idx);

    } else if (ssb_scs == 30000 && pdcch_scs == 15000){
      coreset0_config =  TABLE_13_3.at(coreset0_idx);

    } else if (ssb_scs == 30000 && pdcch_scs == 30000){
      coreset0_config =  TABLE_13_4.at(coreset0_idx);

    } else{
      SPDLOG_ERROR("Wrong input to PDCCH Coreset Type 0 table");
    }
  } else if (min_chann_bw == 40000) {
    if (ssb_scs == 30000 && pdcch_scs == 15000){
      coreset0_config =  TABLE_13_5.at(coreset0_idx);

    } else if (ssb_scs == 30000 && pdcch_scs == 30000){
      coreset0_config = TABLE_13_6.at(coreset0_idx);
      
    } else{
      SPDLOG_ERROR("Wrong input to PDCCH Coreset Type 0 table");
    }

  } else {
    SPDLOG_ERROR("Wrong input to PDCCH Coreset Type 0 table");

  }

  return coreset0_config;

}

