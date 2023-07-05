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

#ifndef DCI_H
#define DCI_H

#include <vector>
#include <string>
#include "common_checks.h"

class dci
{
  public:

    /*Constructor/Destructor*/
    dci();
    ~dci() = default;
    dci(bool found_possible_dci_, uint8_t found_aggregation_level_, uint8_t found_candidate_, uint8_t max_num_candidate_, uint8_t coreset_id_,
    uint8_t coreset_start_rb_, std::string rnti_type_, std::string dci_format_, uint16_t rnti_, std::vector<uint8_t> payload_, uint16_t nof_bits_, 
    uint16_t pdcch_scrambling_id_, uint8_t n_slot_, uint8_t n_ofdm_, float correlation_);
    // dci(bool, uint8_t , uint8_t , uint8_t, uint8_t, std::string , std::string , uint16_t , std::vector<uint8_t>, uint16_t, uint16_t);

    /*Getters and Setters*/
    void set_found_possible_dci(bool);
    void set_found_aggregation_level(uint8_t);
    void set_found_candidate(uint8_t);
    void set_max_num_candidate(uint8_t);
    void set_coreset_id(uint8_t);
    void set_coreset_start_rb(uint8_t);
    void set_rnti_type(std::string);
    void set_dci_format(std::string);
    void set_rnti(uint16_t);
    void set_payload(std::vector<uint8_t>);
    void set_nof_bits(uint16_t);
    void set_pdcch_scrambling_id(uint16_t);
    void set_n_slot(uint8_t);
    void set_n_ofdm(uint8_t);
    void set_num_symbols_per_slot(uint8_t);
    void set_correlation(float);

    bool get_found_possible_dci();
    uint8_t get_found_aggregation_level();
    uint8_t get_found_candidate();
    uint8_t get_max_num_candidate();
    uint8_t get_coreset_id();
    uint8_t get_coreset_start_rb();
    std::string get_rnti_type();
    std::string get_dci_format();
    uint16_t get_rnti();
    std::vector<uint8_t> get_payload();
    uint16_t get_nof_bits();
    uint16_t get_pdcch_scrambling_id();
    uint8_t get_n_slot();
    uint8_t get_n_ofdm();
    uint8_t get_num_symbols_per_slot(uint8_t);
    float get_correlation();

  private:
    bool found_possible_dci;         
    uint8_t found_aggregation_level;///< DCI config
    uint8_t found_candidate;
    uint8_t max_num_candidate;
    uint32_t coreset_id;       ///< CORESET identifier
    uint32_t coreset_start_rb; ///< CORESET lowest RB index in the resource grid
    std::string rnti_type;        ///< RNTI type
    std::string dci_format;           ///< DCI format
    uint16_t rnti;             ///< UE temporal RNTI
    std::vector<uint8_t> payload;
    uint16_t nof_bits;
    uint16_t pdcch_scrambling_id;
    uint8_t n_slot; 
    uint8_t n_ofdm;
    uint8_t num_symbols_per_slot;
    float correlation;

};

#endif