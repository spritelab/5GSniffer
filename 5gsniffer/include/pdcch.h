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

#ifndef PDCCH_H
#define PDCCH_H

#include "coreset.h"
#include "dmrs.h"
#include "dci.h"
#include <cmath>
#include "worker.h"
#include <semaphore>
#include <srsran/srsran.h>
#include "srsran_exports.h"

namespace nr {
  class pdcch : public worker {

    public:
      int32_t subcarrier_offset;
      uint16_t scrambling_id_start;
      uint16_t scrambling_id_end;
      uint16_t rnti_start;
      uint16_t rnti_end;
      uint16_t max_rnti_queue_size;
      std::vector<uint8_t> dci_sizes_list; 
      std::vector<float> AL_corr_thresholds;  
      bool sc_power_decision;
      // Used to compute the timing of found DCIs
      uint64_t sample_rate_time;
      int rnti_list_length;

      /*Constructor/Destructor*/
      pdcch();
      pdcch(uint16_t RNTI_, coreset coreset_info_);
      virtual ~pdcch();

    /*Getters and Setters*/
      uint16_t get_RNTI();
      coreset get_coreset_info();
      void set_RNTI(uint16_t RNTI_);
      void set_coreset_info(coreset coreset_info_);
      void initialize_RNTI_list();
      bool update_RNTI_list(uint16_t found_RNTI);
      void initialize_dmrs_seq(); 

      std::vector<dci> get_found_dci_list_per_AL(uint8_t AL, std::vector<dci>& found_dci_list);
      void add_found_dci(dci dci_info_);

      int delete_lower_AL_dcis(uint16_t scrambling_id, uint8_t n_slot, uint8_t n_ofdm , uint8_t candidate_idx, uint8_t AL, std::vector<dci>& found_dci_list);

    /*Function that processes input symbols to PDCCH, finding PDCCH/DMRS, decoding, starts here*/
      void process(shared_ptr<vector<symbol>>& symbols, int64_t metadata) override;
    
    /*Aux function that finds candidates based on subcarriers where power was found*/
      std::vector<uint8_t> find_list_candidates(std::vector<uint16_t> occupied_subcarriers, uint8_t agg_level,uint8_t nSlot, bool user_search_space);

    /*Function that correlates PDCCH DMRS*/
      bool correlate_DMRS(symbol& symbol, std::vector<dci>& found_dci_list);

    /*This overloaded version correlates DMRs with the decision from looking at the power per subcarrier*/
      bool correlate_DMRS(symbol& symbol,std::vector<uint8_t> list_candidates,uint8_t agg_level, std::vector<dci>& found_dci_list);
      // float compute_correlation_DMRS(symbol& symbol, std::vector<uint16_t> pdcch_dmrs_rb_indices, std::vector<uint64_t> pdcch_dmrs_sc_indices, std::vector<std::complex<float>> pdcch_dmrs_symbols_al_max);
      float compute_correlation_DMRS(symbol& symbol, std::vector<std::complex<float>>& pdcch_dmrs_symbols, std::vector<uint64_t>& pdcch_dmrs_sc_indices);

      std::vector<std::complex<float>> estimate_channel_dci(symbol& symbol, dci dci_);

    /*PDCCH decoder*/
    int decode_pdcch(symbol& symbol, std::vector<std::complex<float>>& pdcch_symbols, dci dci_, srsran_pdcch_nr_res_t* res, bool rep_opt, int64_t metadata, int symbol_in_chunk);

    
    /*Util functions to generate PDCCH RB/SC indices, candidates, etc*/
      std::vector<uint16_t> cce_reg_interleaving();

      std::vector<uint16_t> get_rb_interleaved(uint8_t aggregation_level);
      
      std::vector<uint16_t> get_rb_candidates(uint8_t aggregation_level, uint8_t candidate_idx, uint8_t num_candidates, uint8_t slotNum, bool user_search_space);
    
      std::vector<uint16_t> get_dmrs_rb_indices(uint8_t aggregation_level, uint8_t candidate_idx, uint8_t num_candidates, uint8_t slotNum, bool user_search_space);

      std::vector<uint64_t> get_dmrs_sc_indices(uint8_t aggregation_level, uint8_t candidate_idx, uint8_t num_candidates, uint8_t slotNum, bool user_search_space);

      std::vector<uint16_t> get_data_sc_indices(uint8_t aggregation_level, uint8_t candidate_idx, uint8_t num_candidates, uint8_t slotNum, bool user_search_space);

      std::vector<uint16_t> get_candidates(uint8_t aggregation_level, uint8_t candidate_idx, uint8_t num_candidates, uint8_t slotNum, bool user_search_space);

      int compute_Yp(uint8_t slotNum, bool user_search_space);
      
      uint32_t pdcch_nr_c_init_scrambler(uint16_t RNTI, uint16_t pdcch_scrambling_id);

      std::unordered_map<string, std::vector<std::complex<float>> > dmrs_seq_table;

      std::unordered_map<string, std::vector<uint64_t> > dmrs_sc_indices_table;

      std::unordered_map<string, std::vector<uint16_t> > data_sc_indices_table;


    private:
      uint16_t RNTI;
      coreset coreset_info;
      std::vector<uint16_t> found_RNTI_list;

      void write_pdcch_symbol_metadata(uint64_t sample_index, uint16_t scrambling_id, uint8_t aggregation_level, uint8_t candidate_idx, float correlation);
  };
} // namespace nr

#endif