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

#ifndef PBCH_H
#define PBCH_H

#include <cstdint>
#include <vector>
#include <complex>
#include <memory>
#include <srsran/srsran.h>
#include <span>
#include "worker.h"
#include "phy.h"

using namespace std;

/**
 * Class for decoding PBCH.
 */
class pbch : public worker {
  public:
    pbch(shared_ptr<nr::phy> phy);
    virtual ~pbch();
    std::function<void(srsran_mib_nr_t&, bool)> on_mib_found = [](srsran_mib_nr_t& mib, bool found) {};
    void process(shared_ptr<vector<symbol>>& symbols, int64_t metadata) override;
    float get_ibar_ssb_snr(uint8_t i_ssb, uint8_t n_hf, vector<symbol> symbols);
    float channel_estimate(uint8_t i_ssb, uint8_t n_hf, vector<symbol>& symbols);
    static vector<uint64_t> get_dmrs_indices(uint8_t ofdm_symbol_number, uint16_t cell_id);
    static vector<uint64_t> get_data_indices(uint8_t ofdm_symbol_number, uint16_t cell_id);
    void initialize_dmrs_seq();
    // Tables for optimization
    std::unordered_map<string, std::vector<std::complex<float>> > dmrs_seq_table;
    std::unordered_map<string, std::vector<uint64_t> > dmrs_sc_indices_table;

  private:
    void decode(vector<complex<float>>& soft_symbols);
    
    shared_ptr<nr::phy> phy;
};

#endif // PBCH_H