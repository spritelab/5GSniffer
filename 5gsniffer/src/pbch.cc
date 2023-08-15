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

#include <cstdint>
#include <spdlog/spdlog.h>
#include "phy_params_common.h"
#include "srsran_exports.h"
#include "pbch.h"
#include "utils.h"
#include "dmrs.h"

/** 
 * Constructor for pbch.
 */
pbch::pbch(shared_ptr<nr::phy> phy) {
  this->phy = phy;
}

/** 
 * Destructor for pbch.
 */
pbch::~pbch() {

}

/** 
 * Initializes all PBCH DMRS sequences
 */
void pbch::initialize_dmrs_seq(){
  // Find ibar_ssb
  dmrs tmp;
  for(uint8_t i_ssb = 0; i_ssb <= 4; i_ssb++) {
    for(uint8_t n_hf = 0; n_hf <= 1; n_hf++) {
      auto dmrs_reference_all = tmp.generate_pbch_dmrs_symb(i_ssb, n_hf, phy->get_cell_id());
      uint64_t count = 0;
      for(uint8_t symbol_index = 1; symbol_index <= 3; symbol_index++) {
        // Get DMRS indices
        auto dmrs_indices = pbch::get_dmrs_indices(symbol_index, phy->get_cell_id());
        // Get subset of the DMRS sequence for corresponding to the current symbol
        vector<complex<float>> dmrs_reference;
        dmrs_reference.reserve(60);
        for(uint64_t i = 0; i < dmrs_indices.size(); i++) {
          dmrs_reference.push_back(dmrs_reference_all.at(count));
          count += 1;
        }
        std::string key = std::to_string(symbol_index) + std::to_string(i_ssb) + std::to_string(n_hf);      
        this->dmrs_seq_table.emplace(key, dmrs_reference);
        this->dmrs_sc_indices_table.emplace(key, dmrs_indices);
      }
    }
  }
}

/** 
 * Use srsRAN to perform decoding of equalized PBCH data symbols.
 * TODO clean this up and ultimately remove srsRAN dependency.
 */
void pbch::decode(vector<complex<float>>& soft_symbols) {
  srsran_ssb_t ssb = {};
  
  srsran_pbch_nr_args_t args = {};
  args.enable_encode         = false;
  args.enable_decode         = true;
  args.disable_simd          = false;
  srsran_pbch_nr_t* q = &ssb.pbch;

  if (srsran_pbch_nr_init(q, &args) < SRSRAN_SUCCESS) {
    ERROR("Error init NR PBCH");
    return;
  }

  srsran_pbch_nr_cfg_t cfg = {};
  srsran_pbch_msg_nr_t msg = {};
  cfg.N_id                 = phy->get_cell_id();
  cfg.n_hf                 = 0;
  cfg.ssb_idx              = 0;
  cfg.Lmax                 = 8;
  cfg.beta                 = 0.0f;

  // TS 38.211 7.3.3 Physical broadcast channel
  // 7.3.3.2 Modulation
  SPDLOG_DEBUG("Demodulating");
  int8_t llr[PBCH_NR_E];
  srsran_demod_soft_demodulate_b(SRSRAN_MOD_QPSK, (const cf_t *)soft_symbols.data(), llr, PBCH_NR_M);

  // 7.3.3.1 Scrambling
  SPDLOG_DEBUG("Scrambling");
  pbch_nr_scramble_rx(&cfg, cfg.ssb_idx, (const int8_t *)&llr, (int8_t *)&llr);

  // 7.1.5 Rate matching
  SPDLOG_DEBUG("Rate matching");
  int8_t d[PBCH_NR_N];
  if (pbch_nr_polar_rm_rx(q, llr, d) < SRSRAN_SUCCESS) {
    return;
  }

  // TS 38.212 7.1 Broadcast channel
  // 7.1.4 Channel coding
  SPDLOG_DEBUG("Polar decoding");
  uint8_t b[PBCH_NR_K];
  if (pbch_nr_polar_decode(q, d, b) < SRSRAN_SUCCESS) {
    return;
  }

  // 7.1.3 Transport block CRC attachment
  msg.crc = srsran_crc_match(&q->crc, b, PBCH_NR_A);
  SPDLOG_DEBUG("crc={}", msg.crc ? "OK" : "KO");

  if(msg.crc) {
    // 7.1.2 Scrambling
    uint8_t a[PBCH_NR_A];
    pbch_nr_scramble(&cfg, b, a);

    // 7.1.1  PBCH payload generation
    pbch_nr_pbch_msg_unpack(&cfg, a, &msg);

    srsran_mib_nr_t mib = {};
    srsran_pbch_msg_nr_mib_unpack(&msg, &mib);

    char str[512] = {};
    srsran_pbch_msg_info(&msg, str, sizeof(str));
    SPDLOG_DEBUG("{}", str);

    this->on_mib_found(mib,true);
  }else{
    srsran_mib_nr_t mib = {};
    this->on_mib_found(mib,false);
  }

  srsran_pbch_nr_free(q);
}

/** 
 * Demodulate the PBCH symbols.
 */
void pbch::process(shared_ptr<vector<symbol>>& symbols, int64_t metadata) {
  SPDLOG_DEBUG("Processing PBCH symbols for cell id {}", phy->get_cell_id());
  uint8_t best_i_ssb = 0;
  uint8_t best_n_hf = 0;
  float best_snr = -1000.0;

  if(phy->in_synch){
    best_i_ssb = phy->i_ssb;
    best_n_hf = phy->n_hf;
  } else {
    // Find ibar_ssb
    for(uint8_t i_ssb = 0; i_ssb <= 4; i_ssb++) {
      for(uint8_t n_hf = 0; n_hf <= 1; n_hf++) {
        float snr = get_ibar_ssb_snr(i_ssb, n_hf, *symbols);
        if (snr > best_snr) {
          best_i_ssb = i_ssb;
          best_n_hf = n_hf;
          best_snr = snr;
        }
      }
    }
  }

  // Apply the channel estimation for the best i_ssb and n_hf
  channel_estimate(best_i_ssb, best_n_hf, *symbols);
  SPDLOG_DEBUG("Locking to best estimate i_ssb {}, n_hf {}, snr {} for cell id {} phy_in_synch {}", best_i_ssb, best_n_hf, best_snr, phy->get_cell_id(), phy->in_synch);
  phy->i_ssb = best_i_ssb;
  phy->n_hf = best_n_hf;

  // Collect the PBCH data symbols
  vector<complex<float>> pbch_symbols;
  for(uint8_t symbol_index = 1; symbol_index <= 3; symbol_index++) {
    auto data_indices = pbch::get_data_indices(symbol_index, phy->get_cell_id());
    for(auto data_index : data_indices) {
      pbch_symbols.push_back(symbols->at(symbol_index).samples_eq[data_index]);
    }
  }

  SPDLOG_DEBUG("Got {} PBCH data symbols", pbch_symbols.size());

  // Decode PBCH
  decode(pbch_symbols);
}

/** 
 * Channel estimate on a copy of symbols and return the obtained SNR.
 *
 * @param i_ssb SSB index
 * @param n_hf Half-frame
 * @param symbols Symbol vector to copy
 * @return SNR after channel estimation with the specified parameters
 */
float pbch::get_ibar_ssb_snr(uint8_t i_ssb, uint8_t n_hf, vector<symbol> symbols) {
  return channel_estimate(i_ssb, n_hf, symbols);
}

/** 
 * Channel estimate the given symbols and return the obtained SNR.
 *
 * @param i_ssb SSB index
 * @param n_hf Half-frame
 * @param symbols Symbol vector to copy
 * @return SNR after channel estimation with the specified parameters
 */
float pbch::channel_estimate(uint8_t i_ssb, uint8_t n_hf, vector<symbol>& symbols) {

  float total_noise = 0.0f;
  float total_signal = 0.0f;

  for(uint8_t symbol_index = 1; symbol_index <= 3; symbol_index++) {
    // Get DMRS indices and sequence per symbol
    std::string key = std::to_string(symbol_index) + std::to_string(i_ssb) + std::to_string(n_hf);
    
    auto dmrs_indices = dmrs_sc_indices_table[key];
    auto dmrs_reference = dmrs_seq_table[key];

    // Get signal power
    // float signal_power = pow(symbols.at(symbol_index).get_average_magnitude(), 2);
    // total_signal += signal_power;

    // Perform channel estimation and equalization
    symbols.at(symbol_index).channel_estimate(dmrs_reference, dmrs_indices, 0, ssb_sc);
    // dump_to_file("/tmp/symbols_eq", span<complex<float>>{symbols.at(symbol_index).samples_eq}, true);


    // Get power of noise
    // float noise_power = pow(symbols.at(symbol_index).get_average_noise_magnitude(), 2);
    // total_noise += noise_power;

    /*In case we want to compute only the cross-correlation, we can get sum of the abs of the average_channel (complex value) 
     and then we do not use the estimated noise in the SNR computation */

  float signal_power = abs(symbols.at(symbol_index).get_average_channel());
  total_signal += signal_power;
  // float snr = 10.0 * log10(total_signal);

  }
  float snr = 10.0 * log10(total_signal);

  // float snr = 10.0 * log10(total_signal / total_noise);
  SPDLOG_DEBUG("i_ssb {} n_hf {} SNR: {}", i_ssb, n_hf, snr);

  return snr;
}

/** 
 * Get indices of the PBCH DM-RS, which depend on the OFDM symbol number and Cell ID.
 */
vector<uint64_t> pbch::get_dmrs_indices(uint8_t ofdm_symbol_number, uint16_t cell_id) {
  vector<uint64_t> indices;

  uint8_t v = cell_id % 4;

  if (ofdm_symbol_number == 1 || ofdm_symbol_number == 3) {
    for(uint8_t i = 0; i <= 236; i += 4) {
      indices.push_back(i + v);
    }
  } else if (ofdm_symbol_number == 2) {
    for(uint8_t i = 0; i <= 44; i += 4) {
      indices.push_back(i + v);
    }
    for(uint8_t i = 192; i <= 236; i += 4) {
      indices.push_back(i + v);
    }
  } else {
    SPDLOG_WARN("Tried to request PBCH indices for symbol number {}, which is invalid for PBCH", ofdm_symbol_number);
  }

  return indices;
}

/** 
 * Get indices of the PBCH data subcarriers, which depend on the OFDM symbol number and Cell ID.
 */
vector<uint64_t> pbch::get_data_indices(uint8_t ofdm_symbol_number, uint16_t cell_id) {
  vector<bool> is_data_index(ssb_sc, true);
  vector<uint64_t> data_indices;

  auto dmrs_indices = pbch::get_dmrs_indices(ofdm_symbol_number, cell_id);
  // DMRS indices
  for (auto dmrs_index : dmrs_indices)
    is_data_index[dmrs_index] = false;

  // SSS and empty symbols
  if (ofdm_symbol_number == 2) {
    for(uint64_t i = 48; i < 192; i++) {
      is_data_index[i] = false;
    }
  }

  for(uint64_t i = 0; i < is_data_index.size(); i++) {
    if(is_data_index[i])
      data_indices.push_back(i);
  }

  return data_indices;
}