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

#include "pdcch.h"
#include "coreset.h"
#include "dsp.h"
#include "utils.h"
#include <cstdint>
#include <spdlog/spdlog.h>
#include <string>
#include <numeric>
#include <execution>
#include <unordered_map>

std::binary_semaphore rnti_list_mutex(1);

pdcch::pdcch(){
  RNTI = 0;
  coreset_info = {};
  scrambling_id_start = 0;
  scrambling_id_end = 0xffff;
  rnti_start = 0;
  rnti_end = 0xffff;
  dci_sizes_list = {};
  sc_power_decision = false;
  max_rnti_queue_size = 65535;
  AL_corr_thresholds = {0.9, 0.8, 0.7, 0.15, 0.15};
  found_RNTI_list.reserve(1<<15);
}

pdcch::pdcch(uint16_t RNTI_, coreset coreset_info_) : pdcch() {
  set_RNTI(RNTI_);
  set_coreset_info(coreset_info_);
}

/** 
 * Destructor for pdcch.
 */
pdcch::~pdcch() {

}

void pdcch::set_RNTI(uint16_t RNTI_){
    RNTI = RNTI_;
}

uint16_t pdcch::get_RNTI(){
  return RNTI;
}

void pdcch::set_coreset_info(coreset coreset_info_){
  coreset_info = coreset_info_;
}

coreset pdcch::get_coreset_info(){
  return coreset_info;
}


void pdcch::initialize_dmrs_seq(){ 

  auto init_dmrs_t0 = time_profile_start();
  dmrs dmrs_pdcch;
  uint8_t symbol_index = coreset_info.get_starting_ofdm_symbol_within_slot();
  uint16_t scrambling_id = scrambling_id_start;
  bool user_search_space = false;
  uint8_t coreset_duration = coreset_info.get_duration();

  for (uint8_t slot_index = 0 ; slot_index < coreset_info.get_num_slots_per_frame(); slot_index++){
    for (int agg_level = 0; agg_level < NUM_ALs; agg_level++){
       /* For all possible candidates*/
      uint8_t max_num_candidate = coreset_info.get_candidates_search_space().at(agg_level);

      for (int candidate_idx = 0; candidate_idx < max_num_candidate; candidate_idx++){
        std::vector<uint16_t> pdcch_dmrs_rb_indices;
        std::vector<uint64_t> pdcch_dmrs_sc_indices;
        std::vector<uint16_t> pdcch_data_sc_indices;

        pdcch_dmrs_rb_indices = get_dmrs_rb_indices(1<<agg_level, candidate_idx, max_num_candidate, slot_index, user_search_space);
        pdcch_dmrs_sc_indices = get_dmrs_sc_indices(1<<agg_level, candidate_idx, max_num_candidate, slot_index, user_search_space);
        pdcch_data_sc_indices = get_data_sc_indices(1<<agg_level, candidate_idx, max_num_candidate, slot_index, user_search_space);

        std::vector<std::complex<float>> pdcch_dmrs_symbols = {};
        pdcch_dmrs_symbols.reserve(pdcch_dmrs_rb_indices.size());

        for (uint8_t dur_idx = 0; dur_idx < coreset_duration ; dur_idx++){
          std::vector<std::complex<float>> pdcch_dmrs_symbols_al_max = dmrs_pdcch.generate_pdcch_dmrs_symb(scrambling_id, slot_index, symbol_index+dur_idx, coreset_info.get_num_symbols_per_slot(), 2*AL_16*18);
          for (int i_idx = 0 ; i_idx < (pdcch_dmrs_rb_indices.size() / coreset_duration); i_idx++){
            pdcch_dmrs_symbols.push_back(pdcch_dmrs_symbols_al_max.at(pdcch_dmrs_rb_indices.at(i_idx)));
          }
        }
        std::string key = std::to_string(scrambling_id) + std::to_string(agg_level) + std::to_string(slot_index) + std::to_string(candidate_idx);
        
        this->dmrs_seq_table.emplace(key, pdcch_dmrs_symbols);
        this->dmrs_sc_indices_table.emplace(key, pdcch_dmrs_sc_indices);
        this->data_sc_indices_table.emplace(key, pdcch_data_sc_indices);
      }
    }
  }

  time_profile_end(init_dmrs_t0, "pdcch::initialize_dmrs_seq");
}

// We initialize the list of RNTI with no specific order, just ascending RNTIs
void pdcch::initialize_RNTI_list(){
  for (int i = rnti_start; i<=rnti_end; i++ ){
    found_RNTI_list.push_back(i);
  }
  SPDLOG_DEBUG("Initialized RNTI list between {} and {}", rnti_start, rnti_end);
}

// Once we find an RNTI, we reorder the list of RNTIs to put that RNTI first in the vector.
bool pdcch::update_RNTI_list(uint16_t found_RNTI){
  bool rnti_in_list = false;

  rnti_list_mutex.acquire();
  std::vector<uint16_t>::iterator position = std::find(found_RNTI_list.begin(), found_RNTI_list.end(), found_RNTI);
  if (position != found_RNTI_list.end()){ // element not found, this might happen for SI-RNTI, 65535.
    found_RNTI_list.erase(position);
    found_RNTI_list.insert(found_RNTI_list.begin(), found_RNTI);
    rnti_in_list = true;
  }
  rnti_list_mutex.release();
  
  return rnti_in_list;
}

/* Look for DCIs across the whole PDCCH region. CORESET duration indicates how many OFDM symbols contain 
PDCCH and starting OFDM symbol in CORESET indicates where does the PDCCH region start within a slot.
The DMRS Sequence depends on the OFDM symbol, slot number, scramblingID,  and number of symbols per slot */

void pdcch::process(shared_ptr<vector<symbol>>& symbols, int64_t metadata) {
  coreset_info.set_num_symbols_per_slot(14);
  srsran_pdcch_nr_res_t res        = {};

  bool user_search_space = false;
  bool found_possible_dci = false;
  int symbol_in_chunk = 0;
  for (symbol& symbol: *symbols) {
    symbol_in_chunk++;
    auto process_symbol_time = time_profile_start();
  
    std::vector<dci> found_dci_list;

    auto correlate_dmrs_t0 = time_profile_start();
    found_possible_dci = correlate_DMRS(symbol, found_dci_list);

    time_profile_end(correlate_dmrs_t0, "pdcch::correlate_DMRS (correlations for one symbol)");

    if (found_possible_dci) {
      srsran_pdcch_nr_res_t res = {};
      dci aux_dci;
      uint8_t dci_size;
      // Decode the DCI list in descendent order of AL to delete lower ALs as we find DCIs
      for (uint8_t AL = NUM_ALs; AL > 0; AL--){
        std::vector<dci> found_dcis = get_found_dci_list_per_AL(1<<(AL-1),found_dci_list);
        for (int i = 0; i < found_dcis.size(); i ++){
          for (int dci_idx = 0; dci_idx < dci_sizes_list.size(); dci_idx++){
            dci_size = dci_sizes_list.at(dci_idx);

            // Estimate the channel first, rather than for each RNTI. 
            auto chest_dci = time_profile_start();
            aux_dci = found_dcis.at(i);
            aux_dci.set_nof_bits(dci_size);
            std::vector<std::complex<float>> equalized_symbols = estimate_channel_dci(symbol, aux_dci);
            time_profile_end(chest_dci, "pdcch::estimate_channel_dci (equalization for one DCI)");

            // For now we only use the optimized repetition mode for AL above 3 (8).
            // The dci size will determine which ALs will have repetition, with K, E and N variables.
            // If the rate matched output is longer than the data, there will be repetition. In this case, we can infer the RNTI without decoding. 
            auto decode_pdcch_t0 = time_profile_start();
  
            bool found_dci_ = false;
            if (AL>3 ){
              int outp = 0;
              // SI or RA
              if (rnti_start < 65520 & rnti_end > 100){
                aux_dci.set_rnti(0);
                outp = decode_pdcch(symbol, equalized_symbols, aux_dci, &res, true, metadata, symbol_in_chunk);  
              }else{
                for (int rnti_i = 0; rnti_i < (int)found_RNTI_list.size(); rnti_i++){
                  auto rnti = found_RNTI_list.at(rnti_i);
                  aux_dci.set_rnti(rnti);
                  outp = decode_pdcch(symbol, equalized_symbols, aux_dci, &res, true, metadata, symbol_in_chunk);
                }
              }     
              if (outp == 1 && (aux_dci.get_found_aggregation_level() > 1)){
                int deleted_dcis = delete_lower_AL_dcis(aux_dci.get_pdcch_scrambling_id(), aux_dci.get_n_slot(), aux_dci.get_n_ofdm(), aux_dci.get_found_candidate(), aux_dci.get_found_aggregation_level(),found_dci_list);
                break;
              }
            }
            else{
              // TODO: Add config in config file to limit over how many of the most recent RNTI list to look for in AL below 8, tradeoff between speed and missing some DCIs.
              // int rnti_list_length = 100;
              // if (AL<=3)
              //   rnti_list_length = max_rnti_queue_size;
              for (int rnti_i = 0; rnti_i < std::min(rnti_list_length,(int)found_RNTI_list.size()); rnti_i++){
                auto rnti = found_RNTI_list.at(rnti_i);
                aux_dci.set_rnti(rnti);
                int outp = decode_pdcch(symbol, equalized_symbols, aux_dci, &res, false, metadata, symbol_in_chunk);
                // If the decoding succeeds, delete from the list of Possible DCIs the ones that that have a lower AL and correspond to the DCI just decoded.
                // To check this, we check that its the same scrambling_id, slot number, and symbol index, and the candidate_idx is a subset of the decoded candidate
                // Also, break after finding a correct RNTI, no need to explore the same DCI for other RNTIs or dci sizes.
                if (outp == 1 && (aux_dci.get_found_aggregation_level() > 1)){
                  int deleted_dcis = delete_lower_AL_dcis(aux_dci.get_pdcch_scrambling_id(), aux_dci.get_n_slot(), aux_dci.get_n_ofdm(), aux_dci.get_found_candidate(), aux_dci.get_found_aggregation_level(), found_dci_list);
                  SPDLOG_DEBUG("deleted {} DCIs", deleted_dcis);
                  // Do not look for more RNTIs in this found_DCI
                  found_dci_ = true; // Flag to break also dci_size.
                  break;
                }
              }
            }

            string decode_pdcch_profile_msg = "pdcch::decode_pdcch (decode of " + to_string(found_RNTI_list.size()) + " RNTIs)";
            time_profile_end(decode_pdcch_t0, decode_pdcch_profile_msg);

            // Do not look for more DCI sizes in this found_DCI if one was already found
            if (found_dci_)
              break;
          }
        }

      }
    }

    found_dci_list.clear(); // Do not process the same DCIs next time

    time_profile_end(process_symbol_time, "pdcch::process (for one symbol)");
  }

}


// Function to estimate channel for a given DCI and return the equalized symbols
std::vector<std::complex<float>> pdcch::estimate_channel_dci(symbol& symbol, dci dci_){

    bool user_search_space = false;
    
    std::string key = std::to_string(dci_.get_pdcch_scrambling_id()) + std::to_string((uint8_t)log2(dci_.get_found_aggregation_level())) + std::to_string(dci_.get_n_slot()) + std::to_string(dci_.get_found_candidate());

    std::vector<uint64_t> pdcch_dmrs_sc_indices = dmrs_sc_indices_table[key];
    std::vector<std::complex<float>> pdcch_dmrs_symbols = dmrs_seq_table[key];
    std::vector<uint16_t> pdcch_data_sc_indices = data_sc_indices_table[key];

    symbol.channel_estimate(pdcch_dmrs_symbols, pdcch_dmrs_sc_indices, pdcch_data_sc_indices.at(0), pdcch_data_sc_indices.at(pdcch_data_sc_indices.size() - 1 ));

    std::vector<complex<float>> pdcch_rx_symbols = symbol.samples_eq;
    
    std::vector<std::complex<float>> pdcch_symbols;
    pdcch_symbols.reserve(pdcch_data_sc_indices.size());

    for (int i =0; i < pdcch_data_sc_indices.size(); i++){
      pdcch_symbols.push_back(pdcch_rx_symbols.at(pdcch_data_sc_indices.at(i)));
    }

    return pdcch_symbols;

}


int pdcch::delete_lower_AL_dcis(uint16_t scrambling_id, uint8_t n_slot, uint8_t n_ofdm , uint8_t candidate_idx, uint8_t AL, std::vector<dci>& found_dci_list)
{
  int counter = 0;
  bool user_search_space = false;
  if (coreset_info.get_candidates_search_space().at(1>>AL) > 0){
    std::vector<uint16_t> cce_indices = get_candidates(AL, candidate_idx, coreset_info.get_candidates_search_space().at(1>>AL), n_slot, user_search_space);
    for (auto it = found_dci_list.begin(); it != found_dci_list.end(); it++){
      if (it->get_found_aggregation_level() < AL)
        if ((it->get_n_slot() == n_slot) && (it->get_n_ofdm() == n_ofdm)){
          std::vector<uint16_t> lower_AL_cce = get_candidates(it->get_found_aggregation_level(), it->get_found_candidate(), coreset_info.get_candidates_search_space().at(1>>(it->get_found_aggregation_level())), n_slot, user_search_space);
          if(includes(cce_indices.begin(), cce_indices.end(), lower_AL_cce.begin(), lower_AL_cce.end())) {
            found_dci_list.erase(it--);
            counter = counter + 1;
          }
        }
    }
  }
  return counter;
}

std::vector<dci> pdcch::get_found_dci_list_per_AL(uint8_t AL, std::vector<dci>& found_dci_list)
{
  std::vector<dci> AL_dci_list;
  AL_dci_list.reserve(found_dci_list.size());

  for (int i = 0; i < found_dci_list.size(); i++){
    if ((found_dci_list.at(i)).get_found_aggregation_level() == (AL))
      AL_dci_list.push_back(found_dci_list.at(i));
  }

  return AL_dci_list;
}


int pdcch::decode_pdcch(symbol& symbol, std::vector<std::complex<float>>& pdcch_symbols, dci dci_, srsran_pdcch_nr_res_t* res, bool rep_opt, int64_t metadata, int symbol_in_chunk)
{
  bool user_search_space = false;

  srsran_pdcch_nr_args_t args = {};
  args.disable_simd           = false;
  args.measure_evm            = false;
  args.measure_time           = false;

  srsran_pdcch_nr_t q = {};
  if (srsran_pdcch_nr_init_rx(&q, &args) < SRSRAN_SUCCESS) {
    SPDLOG_ERROR("Error init pdcch_rx");
  }

  // DCI decoding as in srsRAN library
  q.K = dci_.get_nof_bits() + 24U;                                  // Payload size including CRC
  q.M = (dci_.get_found_aggregation_level()) * (PRB_RE - 3U) * CCE_REG; // Number of RE
  q.E = q.M * 2;                                                 // Number of Rate-Matched bits

  // Get polar code
  if (srsran_polar_code_get(&q.code, q.K, q.E, 9U) < SRSRAN_SUCCESS) {
    SPDLOG_ERROR("Error init");
  }

  // Demodulation
  int8_t* llr = (int8_t*)q.f;
  srsran_demod_soft_demodulate_b(SRSRAN_MOD_QPSK, (const cf_t *)pdcch_symbols.data(), llr, q.M);

  // Measure EVM if configured
  if (q.evm_buffer != NULL) {
    res->evm = srsran_evm_run_b(q.evm_buffer, &q.modem_table, (const cf_t *)pdcch_symbols.data(), llr, q.E);
  } else {
    res->evm = -10;
  }

  // Negate all LLR
  for (uint32_t i = 0; i < q.E; i++) {
    llr[i] *= -1;
  }

  if (rep_opt){
    int N_length = 1<<q.code.n;
    float total_sum = 0.0;
    float max_value = 0;
    int max_pos = -1;
    if (q.E > N_length){
      int8_t* llr_aux = (int8_t*)malloc(q.E * sizeof(int8_t));  
      auto rep_opt_t0 = time_profile_start();

      // This could be parallelized
      for (auto N_RNTI : found_RNTI_list){
        srsran_sequence_apply_c(llr, llr_aux, q.E, pdcch_nr_c_init_scrambler(N_RNTI, dci_.get_pdcch_scrambling_id()));   
        // Adding up repetition
        int sum = 0;      
        for (int i = 0; i< (q.E - N_length); i++){
          sum += std::abs(llr_aux[i] + llr_aux[i + N_length]);
        }

        total_sum += sum;
        if (sum > max_value){
          max_value = sum;
          max_pos = N_RNTI;
        }
      }
      if (max_value > 1.05 * (total_sum/(found_RNTI_list.size()))){
        SPDLOG_DEBUG("Possible Repetition optimized max value {}, RNTI {}, average {}", max_value, max_pos, (total_sum/(found_RNTI_list.size())));
          dci_.set_rnti(max_pos);
      }

      time_profile_end(rep_opt_t0, "pdcch::decode_pdcch (repetition optimization)");
    }
  }

  // Descrambling. If SI-RNTI, or pdcch-DMRS-ScramblingID is not set, the UE should use RNTI 0 and scramblingID as CellID, based on TS 38.211 7.3.2.3.
  if((dci_.get_rnti() < 65520 & dci_.get_rnti() > 100) && (dci_.get_pdcch_scrambling_id() != get_coreset_info().get_cell_id()) ){
    srsran_sequence_apply_c(llr, llr, q.E, pdcch_nr_c_init_scrambler(dci_.get_rnti(), dci_.get_pdcch_scrambling_id()));
  }else{
    srsran_sequence_apply_c(llr, llr, q.E, pdcch_nr_c_init_scrambler(0, get_coreset_info().get_cell_id()));
  }   

  uint8_t PDCCH_NR_POLAR_RM_IBIL = 0;
  // Un-rate matching
  int8_t* d = (int8_t*)q.d;
  if (srsran_polar_rm_rx_c(&q.rm, llr, d, q.E, q.code.n, q.K, PDCCH_NR_POLAR_RM_IBIL) < SRSRAN_SUCCESS) {
    SPDLOG_ERROR("Polar decoder rate un-matching failed");  
  }

  // Decode
  if (srsran_polar_decoder_decode_c(&q.decoder, d, q.allocated, q.code.n, q.code.F_set, q.code.F_set_size) < SRSRAN_SUCCESS) {
    SPDLOG_ERROR("Polar decoder failed");
  }
  uint8_t SRSRAN_POLAR_INTERLEAVER_K_MAX_IL = 164;
  // De-allocate channel
  uint8_t c_prime[SRSRAN_POLAR_INTERLEAVER_K_MAX_IL];
  srsran_polar_chanalloc_rx(q.allocated, c_prime, q.code.K, q.code.nPC, q.code.K_set, q.code.PC_set);

  // Set first L bits to ones, c will have an offset of 24 bits
  uint8_t* c = q.c;
  srsran_bit_unpack(UINT32_MAX, &c, 24U);

  // De-interleave
  srsran_polar_interleaver_run(c_prime, c,(uint32_t)sizeof(uint8_t), q.K, false);

  // Unpack RNTI
  uint8_t  unpacked_rnti[16] = {};
  uint8_t* ptr               = unpacked_rnti;
  srsran_bit_unpack(dci_.get_rnti(), &ptr, 16);

  // De-Scramble CRC with RNTI
  srsran_vec_xor_bbb(unpacked_rnti, &c[q.K - 16], &c[q.K - 16], 16);

  // // Check CRC
  ptr                = &c[q.K - 24];
  uint32_t checksum1 = srsran_crc_checksum(&q.crc24c, q.c, q.K);

  uint32_t checksum2 = srsran_bit_pack(&ptr, 24);
  res->crc           = checksum1 == checksum2;

  if (res->crc){
    srsran_vec_fprint_hex(stdout, c, dci_.get_nof_bits());
    char dci_msg_bin[dci_.get_nof_bits() + 1];
    srsran_vec_sprint_bin(dci_msg_bin, dci_.get_nof_bits()+1, c,dci_.get_nof_bits());

    if (!update_RNTI_list(dci_.get_rnti())){
      SPDLOG_ERROR("Failed to update RNTI list");
    }
    // Copy DCI message
    std::vector<uint8_t> dci_payload(c, c + dci_.get_nof_bits());
    dci_.set_payload(dci_payload);
    std::string dci_string = dci_msg_bin;

    float sample_time = ((float) metadata )/sample_rate_time;
    SPDLOG_INFO("Found DCI PDCCH DCI: RNTI = {}, AL = {}, DCI size {}, Time = {}, Samples from start = {}, Slots from samples from start = {} Slot within frame = {}, Symbol within slot = {}, binary dci is {}, correlation is {}",
    dci_.get_rnti(), dci_.get_found_aggregation_level(), dci_.get_nof_bits(), sample_time + symbol_in_chunk* 0.001, sample_time, symbol_in_chunk, symbol.slot_index, symbol.symbol_index, dci_string, dci_.get_correlation());

  }
    srsran_pdcch_nr_free(&q);
  return res->crc;
}


float pdcch::compute_correlation_DMRS(symbol& symbol, std::vector<std::complex<float>>& pdcch_dmrs_symbols, std::vector<uint64_t>& pdcch_dmrs_sc_indices){

  std::vector<float> correlation_outputs;
  vector<complex<float>> pdcch_rx_symbols(symbol.samples.size(),0);
  std::vector<std::complex<float>> rx_dmrs_symbols(pdcch_dmrs_sc_indices.size());

  pdcch_rx_symbols = symbol.samples;

  for (size_t i = 0; i < pdcch_dmrs_sc_indices.size(); i++ ){
    rx_dmrs_symbols.at(i) = pdcch_rx_symbols.at(pdcch_dmrs_sc_indices.at(i));
  }

    std::vector<float> correlation_output(1);

    correlate_magnitude_normalized(correlation_output, rx_dmrs_symbols, {pdcch_dmrs_symbols.data(), pdcch_dmrs_symbols.size()});

    return correlation_output.at(0);
}


bool pdcch::correlate_DMRS(symbol& symbol, std::vector<dci>& found_dci_list){

  dmrs dmrs_pdcch;  

  int num_candidate = 0;
  int max_num_candidate = (coreset_info.get_frequency_domain_resources()*coreset_info.get_duration())/6;
  bool user_search_space = false;
  bool dci_found = false;

  std::vector<float> correlation_value_per_candidate(max_num_candidate);
  std::vector<float> correlation_value_per_AL(NUM_ALs);
  std::vector<int> correlation_index_per_AL(NUM_ALs);
  std::vector<float> threshold_per_AL = AL_corr_thresholds;


  std::vector<std::complex<float>> pdcch_dmrs_symbols;
  std::vector<uint64_t> pdcch_dmrs_sc_indices;

  /* Compute correlation for all possible scrambling IDs*/
  for (uint32_t pdcch_scrambling_id = scrambling_id_start; pdcch_scrambling_id <= scrambling_id_end; pdcch_scrambling_id++){
    /* For all possible Aggregation levels*/
    for (int agg_level = 0; agg_level < NUM_ALs; agg_level++){
     /* For all possible candidates*/
      max_num_candidate = coreset_info.get_candidates_search_space().at(agg_level);
      for (int candidate_idx = 0; candidate_idx < max_num_candidate; candidate_idx++){
        std::string key = std::to_string(pdcch_scrambling_id) + std::to_string(agg_level) + std::to_string(symbol.slot_index) + std::to_string(candidate_idx);
        pdcch_dmrs_sc_indices = dmrs_sc_indices_table[key];
        pdcch_dmrs_symbols = dmrs_seq_table[key];

        correlation_value_per_candidate.at(candidate_idx) = compute_correlation_DMRS(symbol, pdcch_dmrs_symbols, pdcch_dmrs_sc_indices);
        if ((agg_level == 0 && correlation_value_per_candidate.at(candidate_idx) > threshold_per_AL.at(0)) | (agg_level == 1 && correlation_value_per_candidate.at(candidate_idx) > threshold_per_AL.at(1)) | 
          (agg_level == 2  && correlation_value_per_candidate.at(candidate_idx) > threshold_per_AL.at(2)) | (agg_level == 3  && correlation_value_per_candidate.at(candidate_idx) > threshold_per_AL.at(3)) |
          (agg_level == 4  && correlation_value_per_candidate.at(candidate_idx) > threshold_per_AL.at(4))){
          SPDLOG_DEBUG("Possible DCI with correlation {} at scrambling ID {} and aggregation level AL {} at Cand. Index {} in slot {}", correlation_value_per_candidate.at(candidate_idx),pdcch_scrambling_id, 1<<agg_level, candidate_idx, symbol.slot_index);
          // We save the possible DCI to decode it in the next step.
          dci dci_info(true, 1<<agg_level, candidate_idx, max_num_candidate, 0, 0, "ue-rnti", "dci-unknown", 0, {}, 0, pdcch_scrambling_id, symbol.slot_index, symbol.symbol_index, correlation_value_per_candidate.at(candidate_idx));
          found_dci_list.push_back(dci_info);
          dci_found = true;
        }

      } 
    }
  }
  return dci_found;
}


/*Function that returns the interleaver function based on the CORESET configuration, i.e. BundleSize, Duration, BW...
I am not using the duration because basically you just repeat everything per symbol, the only thing that changes is the c_init of the DMRS*/
std::vector<uint16_t> pdcch::cce_reg_interleaving(){
  coreset_info = get_coreset_info();
  uint8_t L = coreset_info.get_reg_bundlesize();
  uint8_t R = coreset_info.get_interleaver_size();
  uint8_t M = coreset_info.get_duration();
  uint16_t N_CORESET_RB = coreset_info.get_frequency_domain_resources();
  uint16_t N_CORESET_REG = N_CORESET_RB * M;
  uint16_t nshift = coreset_info.get_shift_index();
  uint16_t C = N_CORESET_REG/( R * L );
 
  std::vector<uint16_t> interleaved_f((C-1) *R + (R-1) + 1);
 
  if (coreset_info.get_cce_reg_mapping_type().compare("interleaved") == 0){
    for (int C_idx = 0; C_idx < C; C_idx++){
      for (int R_idx = 0; R_idx < R; R_idx++){
        interleaved_f.at(C_idx * R + R_idx) = ((R_idx*C + C_idx + nshift)%(N_CORESET_REG/L));
      }
    }
  }else{
    for (int i = 0; i < ((C-1) *R + (R-1) + 1); i++){
      interleaved_f.at(i) = i;
    }
  }
  return interleaved_f;
}  

 
/*Returns the RB complete list interleaved by the cce-reg mapping interleaving function
 Actually for duration parameter, the OFDM symbols work independently, the indices are
 the indices + the RB/SC per Bandwidth. */
std::vector<uint16_t> pdcch::get_rb_interleaved(uint8_t aggregation_level){
  std::vector<uint16_t> interleaving_f = cce_reg_interleaving();
  std::vector<uint16_t> rb_complete_list = {};
  std::vector<uint16_t> rb_interleaved = {};

  rb_complete_list.reserve(get_coreset_info().get_frequency_domain_resources() * get_coreset_info().get_duration());
  rb_interleaved.reserve(rb_complete_list.size());

  /*Create a vector with all the possible RBs for a given duration and BW*/
  for (int i = 0; i < get_coreset_info().get_frequency_domain_resources(); i ++){
    for (int j = 0 ; j < get_coreset_info().get_duration(); j++){
      rb_complete_list.push_back(i + j*get_coreset_info().get_frequency_domain_resources());
    }
  }

  /*Interleave the RBs based on the RegBundleSize*/
  for (int i = 0; i < interleaving_f.size(); i++){
    rb_interleaved.insert(rb_interleaved.end(), rb_complete_list.begin() + get_coreset_info().get_reg_bundlesize() * (interleaving_f.at(i)),rb_complete_list.begin() + get_coreset_info().get_reg_bundlesize()*(interleaving_f.at(i) + 1));
  }

  return rb_interleaved;
}

/**/
std::vector<uint16_t> pdcch::get_rb_candidates(uint8_t aggregation_level, uint8_t candidate_idx, uint8_t num_candidates, uint8_t slotNum, bool user_search_space){
  std::vector<uint16_t> cce_indices = get_candidates(aggregation_level, candidate_idx, num_candidates, slotNum, user_search_space);
  std::vector<uint16_t> rb_interleaved = get_rb_interleaved(aggregation_level);
  std::vector<uint16_t> rb_idx_candidates = {};
  rb_idx_candidates.reserve(CCE_REG*cce_indices.size());

  /*Select the RBs associated with the CCE indices*/
  for (int cce_i = 0; cce_i < cce_indices.size() ; cce_i++){
    for (int rb_idx = 0; rb_idx < CCE_REG; rb_idx++){
      rb_idx_candidates.push_back((rb_interleaved.at(cce_indices.at(cce_i)*CCE_REG + rb_idx)));    
    }    
  }
  std::sort (rb_idx_candidates.begin(), rb_idx_candidates.end()); 

  return rb_idx_candidates;
}

/*When interleaving, the gold sequence is generated based on creating the full-length sequence and then 
selecting the indexes for the given RB. e.g. assuming AL 4 and CCEs 2,3,5,6 which is PRBs 12:24 and 36:48,
you create the gold sequence assuming 48 RBs, which is a sequence of length PRB_size * 3,
as there is 3 DMRS per RB, 144 length DMRS sequence, and then you select the indices
that correspond to the DMRS symbols in PRBs 12:24 and 36:48, which would be 3 per each PRB.
In the case of Duration > 1, the indices are just the ones in the first OFDM symbol.
This function returns the indices in the Gold Sequence for that RB candidate.*/

std::vector<uint16_t> pdcch::get_dmrs_rb_indices(uint8_t aggregation_level, uint8_t candidate_idx, uint8_t num_candidates, uint8_t slotNum, bool user_search_space){
  std::vector<uint16_t> rb_indices = get_rb_candidates(aggregation_level, candidate_idx, num_candidates, slotNum, user_search_space);
  std::vector<uint16_t> rb_dmrs_idx = {};
  rb_dmrs_idx.reserve(3*rb_indices.size());

    for (int rb_idx = 0; rb_idx < rb_indices.size(); rb_idx++){
      for (int dmrs_rb = 0; dmrs_rb < DMRS_RE_PRB; dmrs_rb++){
        rb_dmrs_idx.push_back(3*rb_indices.at(rb_idx) + dmrs_rb);
      }
    }
  return rb_dmrs_idx;
}


/*Function that computes the DMRS resource grid subcarrier indices. For duration spanning multiple OFDM symbols, the subcarrier
values in OFDM symbols 0 +1 and 0+2 will be based as distance with the subcarrier in OFDM symbol 0 at position 0. 
E.g. for 48 RBs, maximum sc in OFDM symbol 0 is 575 (0-based), values for OFDM symbol 2 would go from 576 to 576 + 575.
Interestingly, the DMRS sc indices for OFDM symbols 0+1 and 0+2 are 0 + (duration*BW*12), so computing the values for the
first OFDM symbol and then  would suffice.*/

std::vector<uint64_t> pdcch::get_dmrs_sc_indices(uint8_t aggregation_level, uint8_t candidate_idx, uint8_t num_candidates, uint8_t slotNum, bool user_search_space){
  std::vector<uint16_t> dmrs_per_rb = {1, 5, 9}; // In MATLAB is 2 6 10, but is not 0-based.
  std::vector<uint16_t> rb_dmrs_idx = get_rb_candidates(aggregation_level, candidate_idx, num_candidates, slotNum, user_search_space);
  std::vector<uint64_t> sc_dmrs_idx = {};
  sc_dmrs_idx.reserve(aggregation_level * DMRS_SC_CCE);

  for (int rb_idx = 0; rb_idx < rb_dmrs_idx.size(); rb_idx++){
    for (int dmrs_rb = 0; dmrs_rb < dmrs_per_rb.size(); dmrs_rb++){
        sc_dmrs_idx.push_back(12*(rb_dmrs_idx.at(rb_idx)) + dmrs_per_rb.at(dmrs_rb));
    }
  }

  return sc_dmrs_idx;
}

/*Function that computes the PDCCH data resource grid subcarrier indices.*/

std::vector<uint16_t> pdcch::get_data_sc_indices(uint8_t aggregation_level, uint8_t candidate_idx, uint8_t num_candidates, uint8_t slotNum, bool user_search_space){
  std::vector<uint16_t> data_per_rb = {0,2,3,4,6,7,8,10,11}; // In MATLAB is 2 6 10, but is not 0-based.
  std::vector<uint16_t> rb_data_idx = get_rb_candidates(aggregation_level, candidate_idx, num_candidates, slotNum, user_search_space);
  std::vector<uint16_t> sc_data_idx = {};
  sc_data_idx.reserve(aggregation_level * 3 * DMRS_SC_CCE);

  for (int rb_idx = 0; rb_idx < rb_data_idx.size(); rb_idx++){
    for (int data_rb = 0; data_rb < data_per_rb.size(); data_rb++){
        sc_data_idx.push_back(12*(rb_data_idx.at(rb_idx)) + data_per_rb.at(data_rb));
    }
  }

  return sc_data_idx;
}

std::vector<uint16_t> pdcch::get_candidates(uint8_t aggregation_level, uint8_t candidate_idx, uint8_t num_candidates, uint8_t slotNum, bool user_search_space){
  uint8_t num_CCE = (get_coreset_info().get_frequency_domain_resources()* get_coreset_info().get_duration()) /CCE_REG;
  uint8_t nCI = 0; // carrier indicator field (?)
  std::vector<uint16_t> cce_indices = {};
  cce_indices.reserve(aggregation_level);
  int Yp = compute_Yp(slotNum, user_search_space);

  for (int cce_idx = 0 ; cce_idx < aggregation_level ; cce_idx ++  ){
    cce_indices.push_back(aggregation_level * ((Yp + int(floor((candidate_idx * num_CCE)/(aggregation_level * num_candidates))) + nCI )%(int(floor(num_CCE/aggregation_level)))) + cce_idx);
  } 
  return cce_indices; 
}    

/*Computing Yp, as in Matlab. For the sniffer, we use CSS to look for all possible positions.
From TechPlayon CORESET : "The controlResourceSetId is unique among the BWPs of a ServingCell."
RNTI 0 for CSS and others for USSS */
int pdcch::compute_Yp(uint8_t slotNum, bool USS){
  int Yp = 0;
  int D = 65537;  
  uint16_t Ap = 0;

  switch(get_coreset_info().get_control_resourceset_id()%3){
    case 0:
      Ap = 39827;
      break;
    case 1:
      Ap = 39829;
      break;
    default:
      Ap = 39839;
  }

  if(USS){
    Yp = get_RNTI();
    for (int i = 0; i<= slotNum; i ++){
      Yp = (Ap*Yp) % D;
    }
  }else{
    Yp = 0;
  }

  return Yp;

}

uint32_t pdcch::pdcch_nr_c_init_scrambler(uint16_t RNTI, uint16_t pdcch_DMRS_scrambling_id){
  return ((RNTI << 16U) + pdcch_DMRS_scrambling_id) & 0x7fffffffU;
}

void pdcch::write_pdcch_symbol_metadata(uint64_t sample_index, uint16_t scrambling_id, uint8_t aggregation_level, uint8_t candidate_idx, float correlation) {
  std::ios::openmode open_mode = std::ios::app;

  std::string pdcch_debug_meta = "/tmp/pdcch_" + std::to_string(coreset_info.get_control_resourceset_id()) + ".csv";
  ofstream f(pdcch_debug_meta.c_str(), open_mode);
  f << (uint64_t)sample_index << ","  \
    << (uint64_t)scrambling_id << "," \
    << (uint64_t)aggregation_level << "," \
    << (uint64_t)candidate_idx << "," \
    << (double)correlation \
    << std::endl;
  f.close();
}