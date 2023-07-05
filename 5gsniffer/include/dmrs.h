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

#ifndef DMRS_H
#define DMRS_H

#include "pn_sequences.h"
#include <liquid/liquid.h>

class dmrs
{
  public:

    /*Constructor/Destructor*/
    dmrs();
    ~dmrs() = default;
		
		std::vector<uint8_t> generate_pbch_dmrs_seq(uint8_t i_ssb, uint8_t n_hf, uint16_t nid_cell);
    std::vector<std::complex<float>> generate_pbch_dmrs_symb(uint8_t i_ssb, uint8_t n_hf, uint16_t nid_cell);

    std::vector<uint8_t> generate_pdcch_dmrs_seq(uint16_t n_id, uint8_t n_slot, uint8_t n_ofdm, uint8_t num_symbols_per_slot, uint16_t pdcch_dmrs_length);
		std::vector<std::complex<float>> generate_pdcch_dmrs_symb(uint16_t n_id, uint8_t n_slot, uint8_t n_ofdm, uint8_t num_symbols_per_slot, uint16_t pdcch_dmrs_length);
    
	private:
};

#endif