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
#include "channel_mapper.h"
#include "config.h"
#include "spdlog/spdlog.h"
#include "sss.h"
#include "dsp.h"
#include "utils.h"
#include "phy.h"

using namespace std;

extern struct config config;

/** 
 * Constructor for channel_mapper.
 */
channel_mapper::channel_mapper(shared_ptr<nr::phy> phy, pdcch_config pdcch_config) :
  phy(phy) {

  // Configure the PDCCH
  pdcch.subcarrier_offset = pdcch_config.subcarrier_offset;
  pdcch.scrambling_id_start = pdcch_config.scrambling_id_start;
  pdcch.scrambling_id_end = pdcch_config.scrambling_id_end;
  pdcch.rnti_start = pdcch_config.rnti_start;
  pdcch.rnti_end = pdcch_config.rnti_end;
  pdcch.dci_sizes_list = pdcch_config.dci_sizes_list;
  pdcch.AL_corr_thresholds = pdcch_config.AL_corr_thresholds;
  pdcch.max_rnti_queue_size = pdcch_config.max_rnti_queue_size;
  pdcch.sc_power_decision = pdcch_config.sc_power_decision;
  pdcch.sample_rate_time = pdcch_config.sample_rate_time;
  pdcch.rnti_list_length = pdcch_config.rnti_list_length;
  std::vector<uint8_t> num_candidates_per_AL = pdcch_config.num_candidates_per_AL;  

  coreset coreset_info_(pdcch_config.coreset_id,
                        pdcch_config.num_prbs,
                        pdcch_config.coreset_duration,
                        pdcch_config.coreset_interleaving_pattern,
                        pdcch_config.coreset_reg_bundle_size,
                        pdcch_config.coreset_interleaver_size,
                        pdcch_config.coreset_nshift,
                        phy->get_cell_id(),
                        pdcch_config.coreset_ofdm_symbol_start,
                        14,
                        phy->get_initial_dl_bandwidth_part()->slots_per_frame,
                        pdcch_config.num_candidates_per_AL);
  pdcch.set_coreset_info(coreset_info_);

  // Initialize RNTI list and DMRS sequences
  pdcch.initialize_RNTI_list();
  pdcch.initialize_dmrs_seq();
}

/** 
 * Destructor for channel_mapper.
 */
channel_mapper::~channel_mapper() {

}

void channel_mapper::process(shared_ptr<vector<symbol>>& symbols, int64_t metadata) {
  SPDLOG_DEBUG("Got {} symbols", symbols->size());

  // Get symbols that belong to PDCCH according to search space set and CORESET config, e.g. duration.
  uint8_t coreset_duration = pdcch.get_coreset_info().get_duration();
  // This is configured by monitoringSymbolsWithinSlot in RRC
  uint8_t coreset_ofdm_symbol_start = pdcch.get_coreset_info().get_starting_ofdm_symbol_within_slot();

  // We pass to PDCCH the subcarriers aggregated over the CORESET duration starting from CORESET starting ofdm symbol config
  auto to_process = make_shared<vector<symbol>>();
  for (int idx = 0; idx < symbols->size() - (coreset_duration - 1); idx++) {
    symbol s = symbols->at(idx);
    if(s.symbol_index == coreset_ofdm_symbol_start) {
      std::vector<std::complex<float>> samples_aggregated;
      for (int jdx = 0; jdx < coreset_duration; jdx++){
        samples_aggregated.insert(samples_aggregated.end(), ((symbols->at(idx+jdx)).samples).begin(), ((symbols->at(idx+jdx)).samples).end());
      }
      idx = idx + coreset_duration - 1;
      s.samples = samples_aggregated; 
      to_process->push_back(s);
    }
  }

  pdcch.process(to_process, metadata);
}