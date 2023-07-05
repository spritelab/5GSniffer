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

#ifndef CORESET_H
#define CORESET_H

#include <iostream>
#include <vector>
#include "common_checks.h"
#include <bitset>

class coreset
{
  public:

    /*Constructor/Destructor*/
    coreset();
    ~coreset() = default;     
    coreset(uint8_t control_resourceset_id_,uint16_t frequency_domain_resources_,
    uint8_t duration_, std::string cce_reg_mapping_type_, uint8_t reg_bundlesize_, 
    uint8_t interleaver_size_, uint16_t shift_index_, uint16_t cell_id_,
    uint8_t starting_ofdm_symbol_within_slot_, uint8_t num_symbols_per_slot_, 
    uint8_t num_slots_per_frame_, std::vector<uint8_t> candidates_search_space_);

/* Setters*/
    void set_control_resourceset_id(uint8_t control_resourceset_id_);
    void set_frequency_domain_resources(uint16_t frequency_domain_resources_);
    void set_duration(uint8_t duration_);
    void set_cce_reg_mapping_type(std::string cce_reg_mapping_type_);
    void set_reg_bundlesize(uint8_t reg_bundlesize_);
    void set_interleaver_size(uint8_t interleaver_size_);
    void set_shift_index(uint16_t shift_index_);
    void set_cell_id(uint16_t cell_id_);
    void set_starting_ofdm_symbol_within_slot(uint8_t starting_ofdm_symbol_within_slot_);
    void set_num_symbols_per_slot(uint8_t num_symbols_per_slot_);
    void set_num_slots_per_frame(uint8_t num_slots_per_frame_);
    void set_candidates_search_space(std::vector<uint8_t> candidates_search_space_);

/* Getters*/ 

  uint8_t get_control_resourceset_id();
  uint16_t get_frequency_domain_resources();
  uint8_t  get_duration();
  std::string get_cce_reg_mapping_type();
  uint8_t get_reg_bundlesize();
  uint8_t get_interleaver_size();
  uint16_t get_shift_index();
  uint16_t get_cell_id();
  uint8_t get_starting_ofdm_symbol_within_slot();
  uint8_t get_num_symbols_per_slot();
  uint8_t get_num_slots_per_frame();
  std::vector<uint8_t> get_candidates_search_space();


  private:
    uint8_t control_resourceset_id;
    uint16_t frequency_domain_resources;
    uint8_t duration;
    std::string cce_reg_mapping_type;
    uint8_t reg_bundlesize;
    uint8_t interleaver_size;
    uint16_t shift_index;
    uint16_t cell_id;
    uint8_t starting_ofdm_symbol_within_slot;
    uint8_t num_symbols_per_slot;
    uint8_t num_slots_per_frame;
    std::vector<uint8_t> candidates_search_space;
};

#endif    
