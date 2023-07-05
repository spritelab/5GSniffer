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
#include <memory>
#include <string>
#include <volk/volk.h>
#include <iostream>
#include <span>
#include <zmq.hpp>
#include "bandwidth_part.h"
#include "phy_params_common.h"
#include "sniffer.h"
#include "syncer.h"
#include "spdlog/spdlog.h"
#include "dsp.h"
#include "utils.h"
#include "ofdm.h"
#include "ssb_mapper.h"
#include "flow.h"
#include "rotator.h"
#include "shifter.h"
#include "file_sink.h"
#include "channel_mapper.h"
#include "config.h"
#include <fstream>

extern struct config config;

using namespace std;

/** 
 * Constructor for syncer.
 */
syncer::syncer(uint64_t sample_rate, shared_ptr<nr::phy> phy) :
  sample_rate(sample_rate),
  phy(phy) {
  // Reserve some space for the processing queue
  processing_queue.reserve(1024*1024);

  // Generate all needed PSS and SSS signals
  psss.push_back(pss(0));
  psss.push_back(pss(1));
  psss.push_back(pss(2));

  sss sss_ref(0,0);
  std::array <std::array<std::complex<float>,sss_length>,nid_max+1> ssss = sss_ref.generate_all_sss_seq();
  phy->ssss = ssss;
  // Setup resampler
  unsigned int h_len = 51;                               // Filter semi-length (filter delay)
  resampling_rate = (float)phy->ssb_bwp->sample_rate / (float)sample_rate; // Resampling rate (output/input)
  max_resampled_samples_per_sample = (unsigned int)ceilf(resampling_rate);
  float bw = 0.08f;                                       // Resampling filter bandwidth TODO this parameter is important
  float slsl = 70.0f;                                    // Resampling filter sidelobe suppression level
  unsigned int npfb = 16;                                // Number of filters in bank (timing resolution)
  resampler = resamp_crcf_create(resampling_rate, h_len, bw, slsl, npfb);

  // Look for a given PSS index as specified in the config file
  if (config.nid_2 < 3){
    pss_start = config.nid_2;
    pss_end = config.nid_2;
  }
  // Look for all PSS indexes, nid_2 not specified in the config
  else{ 
    pss_start = 0;
    pss_end = 2;
  }
  
  state = syncer::state::find_pss;
  cfo = 0.0f;

  waiting_for_pss = 0;
  counting_samples = 0;      
  bool in_synch;
  ssb_period = 0.02; // SSB periodicity is 20 ms for initial access.
  pss_window_size = 10000;

  // Create pool of 64 flows that can process samples in parallel after synchronization
  flow_pool = make_shared<class flow_pool>(64);
  this->connect(flow_pool);
}

/** 
 * Destructor for syncer.
 */
syncer::~syncer() {
  resamp_crcf_destroy(resampler);
}

/** 
 * Aligns the received signal to the resource grid and passes samples on to the
 * next workers.
 *
 * @param samples shared_ptr to sample buffer to process
 */
void syncer::process(shared_ptr<vector<complex<float>>>& samples, int64_t metadata) {
  SPDLOG_DEBUG("Received {} samples", samples.get()->size());

  // Apply frequency correction to new samples
  SPDLOG_DEBUG("Applying CFO {} to new samples coming to the processing queue counter", -cfo);

  rotate(*samples.get(), *samples.get(), -cfo, sample_rate);   
  // Add the samples to the processing queue
  processing_queue = std::move(*samples.get());

  if (state == state::reset) {
    // Clear processing queues
    processing_queue.clear();
    downsampled_samples.clear();

    // Clear all bandwidth parts and flows
    this->phy->bandwidth_parts.clear();
    this->disconnect_all();

    // Reset sync
    cfo = 0.0f;

    // Look for PSS again
    state = state::find_pss;
  }

  if (phy->in_synch){
   if (waiting_for_pss > sample_rate * ssb_period){ // We missed the SSB
    waiting_for_pss = 0;
    phy->in_synch = false;
    SPDLOG_DEBUG("PSS tracking failed resetting");
   } else {
    waiting_for_pss += processing_queue.size();
   } 
  }

  if (state == state::wait){
     if ((waiting_for_pss > sample_rate * ssb_period) & (waiting_for_pss - processing_queue.size() < sample_rate * ssb_period)){ // We have to account for the previous chunk of 8 ms where we found SSB and we send already and then we started counting after that.
      state = state::find_pss;
     } else {
      int sent_samples_size = processing_queue.size();
      shared_ptr<vector<complex<float>>> processing_queue_ptr = make_shared<vector<complex<float>>>(std::move(processing_queue)); // TODO allocate processing_queue on the heap in the first place so we don't need to do this copy
      send_to_next_workers(processing_queue_ptr, counting_samples);
      counting_samples = counting_samples + sent_samples_size;
      processing_queue.clear();
     }
  }

  if (state == state::find_pss) {
    SPDLOG_DEBUG("Looking for PSS");
    auto find_pss_t0 = time_profile_start();
    find_pss();
    time_profile_end(find_pss_t0, "syncer::find_pss");
  } 

  if (state == state::fine_sync) {
    SPDLOG_DEBUG("Performing fine synch on downsampled SSB");
    fine_sync();
  } 
  
  if(state == state::find_sss) {
    SPDLOG_DEBUG("Looking for SSS");
    auto find_sss_t0 = time_profile_start();
    find_sss();
    time_profile_end(find_sss_t0, "syncer::find_sss");
  } 
  
  if(state == state::relay) {
    waiting_for_pss = processing_queue.size();
    int sent_samples_size = processing_queue.size();
    shared_ptr<vector<complex<float>>> processing_queue_ptr = make_shared<vector<complex<float>>>(std::move(processing_queue)); // TODO allocate processing_queue on the heap in the first place so we don't need to do this copy
    send_to_next_workers(processing_queue_ptr, counting_samples);
    counting_samples = counting_samples + sent_samples_size;

    processing_queue.clear();
    state = state::wait;
  }
}

/**
 * Downsample the signal given by the samples currently in the processing queue.
 *
 * @param num_samples number of samples to downsample
 */
void syncer::downsample(uint64_t num_samples, int64_t start_sample, int64_t end_sample) {
  for(int64_t i = start_sample; i < end_sample; i++) {
    complex<float> temp[max_resampled_samples_per_sample]; // Temporary buffer for each resampling operation

    uint32_t num_written;
    resamp_crcf_execute(resampler, processing_queue.at(i), temp, &num_written);
    for(int64_t j = 0; j < num_written; j++) {
      downsampled_samples.push_back(temp[j]);
    }
  }
}

/**
 * Sync to the SSB by performing cross-correlation with the PSS reference
 * signals on a downsampled signal.
 */
void syncer::find_pss() {
  uint64_t ssb_num_samples_downsampled = ssb_nfft * 10.0;
  uint64_t ssb_num_samples = ceilf(ssb_num_samples_downsampled / resampling_rate);

  // We want 10 symbols to have sufficient size for the fine sync correlation
  if (processing_queue.size() > ssb_num_samples) {
    // Downsample the signal to span the SSB subcarriers and push to internal buffer
    downsampled_samples.clear();
    downsampled_samples.reserve(processing_queue.size() - ssb_num_samples); 
    int64_t start_sample, end_sample;
    int64_t downsampled_offset;

    // Only look for PSS in a window of samples, as we already have a coarse estimation where it will be
    if (phy->in_synch){
      start_sample = max(int64_t(0),int64_t((sample_rate * ssb_period - (waiting_for_pss - processing_queue.size()) - pss_window_size)));
      end_sample =  min(int64_t(processing_queue.size()),int64_t(start_sample + pss_window_size*2 + 1)); 
      downsampled_offset = (start_sample+1)*resampling_rate;
    }else{
      start_sample = 0;
      end_sample = processing_queue.size(); // Look for SSB only in the first half for speed during initial synch.
      downsampled_offset = 0;
     }
    downsample(processing_queue.size() - ssb_num_samples, start_sample, end_sample);
    // Dot product with the possible PSS
    bool pss_found = false;
    int64_t timing_error = 0;
    int64_t timing_error_downsampled_offset = 0;
    float max_corr = 0.0;
    uint8_t nid2 = 0;
    for(uint8_t pss_idx = pss_start; pss_idx <= pss_end; pss_idx++) {
      vector<float> correlation_magnitudes;
      // Correlate
      int step_size = 1;
      correlate_magnitude(correlation_magnitudes, downsampled_samples, {psss[pss_idx].get_pss_seq_t().data(), psss[pss_idx].get_pss_seq_t().size()}, step_size);

      // Get the average
      float avg_correlation = 0.0f;
      volk_32f_accumulator_s32f(&avg_correlation, correlation_magnitudes.data(), correlation_magnitudes.size());
      avg_correlation /= (float)correlation_magnitudes.size();

      // Find a point larger than x correlation. 0.1 for normalized often appears to sync to other cells but is still a reliable indicator of PSS being present, so kept it for now.
      float threshold = pss_sss_times_avg_threshold*avg_correlation;
      for(int64_t i = 0; i < correlation_magnitudes.size(); i++) {
        float abs_correlation = std::abs(correlation_magnitudes.at(i)); // We allow correlation to be negative since we are not synchronized at this point. 
        if(abs_correlation > threshold) {
          if (abs_correlation > max_corr) {
            timing_error = i*step_size;
            pss_found = true;
            max_corr = abs_correlation;
            nid2 = pss_idx;
            timing_error_downsampled_offset = i + downsampled_offset;
          }
        }
      }
    }

    if (pss_found) {
      // Set PHY NID2
      phy->nid2 = nid2;
      SPDLOG_DEBUG("PSS found max corr {} Setting PHY NID2 to {:}",max_corr, phy->nid2);

      // Cut the received samples at the correct position (both normal and downsampled)
      // We want to cut right before the CP of the PSS (second symbol).
      if(timing_error < phy->ssb_bwp->samples_per_cp(2)){
        SPDLOG_ERROR("Timing error smaller than cp");
        // state = state::find_pss;
      }
      timing_error -= phy->ssb_bwp->samples_per_cp(2);
      timing_error_downsampled_offset = timing_error + downsampled_offset;
      downsampled_samples.erase(downsampled_samples.begin(), downsampled_samples.begin() + timing_error);
      downsampled_samples.erase(downsampled_samples.begin() + ssb_num_samples_downsampled, downsampled_samples.end());
      assert(downsampled_samples.size() == ssb_num_samples_downsampled);

      // Determine approximate location of SSS in full-rate signal. This will be
      // used by fine_time_sync to find the exact location of the SSS.
      uint64_t sss_start_offset = phy->ssb_bwp->samples_per_symbol(2) + phy->ssb_bwp->samples_per_symbol(3) + phy->ssb_bwp->samples_per_cp(4);
      int64_t downsampled_sss_hint = timing_error_downsampled_offset + sss_start_offset;
      sss_hint = static_cast<uint64_t>(((float)downsampled_sss_hint / resampling_rate));
      state = state::fine_sync;
    } else {
      SPDLOG_DEBUG("PSS Not Found, sending to next worker all except a window at the end in case PSS is in between chunks");
      counting_samples = counting_samples + processing_queue.size();

    }
  }
} 

/**
 * Perform fine synchronization by looking at the average phase offset at the
 * cyclic prefix locations within the signal.
 */
void syncer::fine_sync() {
  uint16_t useful_length = phy->ssb_bwp->fft_size;
  uint16_t cp_length = phy->ssb_bwp->samples_per_cp(1); // Get normal CP length
  uint16_t symbol_length = useful_length + cp_length;
  vector<complex<float>> correlations;

  vector<complex<float>> delayed_samples(downsampled_samples);
  vector<complex<float>> zeros(useful_length);
  delayed_samples.insert(delayed_samples.begin(), zeros.begin(), zeros.end());
  
  moving_correlate(correlations, {downsampled_samples.data(), downsampled_samples.size()}, {delayed_samples.data(), downsampled_samples.size()}, cp_length);

  // Since we are aligned to the PSS, the locations of normal cyclic
  // prefixes will be at symbol length *1,*2,*3, and *4 after correlation with
  // the delayed samples. Sum these to get the average angle. Division is not 
  // needed because we are not interested in their magnitudes.
  complex<float> average = correlations[symbol_length*1] + correlations[symbol_length*2] + correlations[symbol_length*3] + correlations[symbol_length*4];

  new_cfo_fine = phy->ssb_bwp->scs * (std::arg(average) / (2*std::numbers::pi));
  SPDLOG_DEBUG("NEW CFO fine (Hz): {}", new_cfo_fine);

  // Perform the new CFO immediately.
  rotate(downsampled_samples, downsampled_samples, -new_cfo_fine, phy->ssb_bwp->sample_rate);
  rotate(processing_queue, processing_queue, -new_cfo_fine, sample_rate);

  state = state::find_sss;
}

void syncer::find_sss() {
  // Create blocks for demodulating SSB
  ofdm ofdm(phy->ssb_bwp);
  ofdm.symbol_index = 2; // We are aligned to the PSS, which is the 3rd symbol in the SSB.
  auto ssb_mapper = make_shared<class ssb_mapper>(phy);

  // Set block callbacks
  ssb_mapper->on_sss_found = std::bind(&syncer::on_sss_found, this, std::placeholders::_1);
  ssb_mapper->on_sss_not_found = std::bind(&syncer::on_sync_lost, this);
  ssb_mapper->pbch.on_mib_found = std::bind(&syncer::on_mib_found, this, std::placeholders::_1,std::placeholders::_2);

  // Make connections
  ofdm.connect(ssb_mapper);

  // Process downsampled SSB block
  auto downsampled_samples_ptr = make_shared<vector<complex<float>>>(downsampled_samples);  // TODO allocate downsampled_samples on the heap in the first place so we don't need to do this copy
  ofdm.process(downsampled_samples_ptr, 0);
}

void syncer::on_sss_found(uint16_t nid1) {
  SPDLOG_DEBUG("Setting PHY NID1 to {}", nid1);
  this->phy->nid1 = nid1;
}

void syncer::on_sync_lost() {
  SPDLOG_DEBUG("Lost sync! Retrying to find PSS.");
  state = state::find_pss; // TODO if it failed multiple times, do a full reset?
  //state = state::reset;
  counting_samples = counting_samples + processing_queue.size();
}

void syncer::on_mib_found(srsran_mib_nr_t& mib, bool found) {
  if (found == false){
    on_sync_lost();
  }else{
  SPDLOG_INFO("Got MIB\nSSB \n Cell ID: {} \n MIB: SFN: {}, SCS: {} ",phy->get_cell_id(), mib.sfn, mib.scs_common);
  mib_id++;
  char mib_str[512] = {};
  srsran_pbch_msg_nr_mib_info(&mib, mib_str, sizeof(mib_str));
  SPDLOG_DEBUG("{}", mib_str);

  // Update the total CFO so it is applied next time
  cfo = new_cfo_fine;
  rotate(processing_queue, processing_queue, -new_cfo_fine, sample_rate);

  SPDLOG_DEBUG("CFO fine (Hz) applied after finding MIB: {}", cfo);

  // If the initial downlink bandwidth part doesn't exist yet, create it
  // Also create any other bandwidth parts specified in the config file
  if(phy->bandwidth_parts.size() == 0) {
    uint32_t flow_index = 0;

    for(pdcch_config pdcch_cfg : config.pdcch_configs) {
      // Override config with MIB
      if(pdcch_cfg.use_config_from_mib) {
        assert(mib.scs_common <= max_numerology);
        pdcch_cfg.numerology = (uint8_t)mib.scs_common;
        std::array<uint8_t, 4> coreset0_config = phy->ssb_bwp->get_pdcch_coreset0(5, phy->ssb_bwp->scs,  15000U << mib.scs_common, mib.coreset0_idx);
        pdcch_cfg.num_prbs = coreset0_config.at(1);
        pdcch_cfg.coreset_duration = coreset0_config.at(2);
        pdcch_cfg.subcarrier_offset = mib.ssb_offset + (pdcch_cfg.num_prbs/2);
        pdcch_cfg.extended_prefix = false;
      }

      // Simplify brute force if we are looking only for SI DCI
      if (pdcch_cfg.si_dci_only) {
        pdcch_cfg.scrambling_id_start = phy->get_cell_id(); // Scrambling ID for SI DCI is always the cell ID
        pdcch_cfg.scrambling_id_end = phy->get_cell_id();
        pdcch_cfg.rnti_start = 65535; // SI-RNTI is always 65535
        pdcch_cfg.rnti_end = 65535;
        pdcch_cfg.coreset_interleaving_pattern = "interleaved";
        pdcch_cfg.coreset_reg_bundle_size = 6;
        pdcch_cfg.coreset_interleaver_size = 2;
        pdcch_cfg.coreset_nshift = phy->get_cell_id();

      }

      auto new_bwp = make_shared<bandwidth_part>(this->sample_rate, pdcch_cfg.numerology, pdcch_cfg.num_prbs, pdcch_cfg.extended_prefix);
      phy->bandwidth_parts.push_back(new_bwp);
      
      auto mapper = make_shared<channel_mapper>(phy, pdcch_cfg);
      phy->channel_mappers.push_back(mapper);
    }
  }

  // If we had at least one BWP, perform fine time synchronization on the first BWP added (full-rate)
  if(phy->bandwidth_parts.size() > 0) {
    fine_time_sync();
  }

  // Now that we are synced, create a processing flow for each BWP
  
  assert(phy->bandwidth_parts.size() == phy->channel_mappers.size());
  for(int i = 0; i < phy->bandwidth_parts.size(); i++) {
      auto bwp = phy->bandwidth_parts.at(i);
      auto mapper = phy->channel_mappers.at(i);
      auto flow = flow_pool->acquire_flow();
      auto rotator = make_shared<class rotator>(this->sample_rate, (float)mapper->pdcch.subcarrier_offset*(float)bwp->scs);
      auto ofdm = make_shared<class ofdm>(bwp);
      flow->connect(rotator);
      rotator->connect(ofdm);
      ofdm->connect(mapper); // Connect OFDM block to the shared PHY-layer channel mapper
  }

  // Locking the PSS and SSS to the synch'ed values
  phy->in_synch = true;
  pss_start = phy->nid2;
  pss_end = phy->nid2;
  SPDLOG_DEBUG("In synch, locking PSS = {}, SSS = {}, Cell ID = {} \n ",phy->nid2, phy->nid1, phy->get_cell_id());
  this->state = state::relay;
  }
}

void syncer::fine_time_sync() {
  auto initial_bwp = phy->get_initial_dl_bandwidth_part();
  vector<complex<float>> sss_full_rate(initial_bwp->fft_size, 0);
  uint64_t num_zeros = initial_bwp->fft_size - sss_length;
  uint64_t start_offset = num_zeros / 2;

  // TODO this is actually OFDM modulation. Make OFDM modulator block and add PSS / PBCH DMRS as well to improve correlation
  auto sss_samples_f = phy->ssss.at((phy->nid1)*3 + phy->nid2);
  sss_full_rate.insert(sss_full_rate.begin()+start_offset, sss_samples_f.begin(), sss_samples_f.end());
  sss_full_rate.resize(initial_bwp->fft_size);
  
  // FFT shift
  std::rotate(sss_full_rate.begin(), sss_full_rate.begin() + sss_full_rate.size() / 2, sss_full_rate.end());
  
  // IFFT
  int flags = 0;
  vector<complex<float>> sss_full_rate_time(initial_bwp->fft_size, 0);
  fftplan q = fft_create_plan(initial_bwp->fft_size, sss_full_rate.data(), sss_full_rate_time.data(), LIQUID_FFT_BACKWARD, flags);
  fft_execute(q);
  fft_destroy_plan(q);

  // Correlate
  vector<float> correlation_magnitudes;
  float correlation_max = 0;
  uint64_t sss_position = 0;
  int64_t search_space_start = sss_hint - initial_bwp->samples_per_symbol(4);
  assert(search_space_start >= 0);
  uint64_t search_space_size = initial_bwp->samples_per_symbol(4) * 2; // Search max 2 symbols

  correlate_magnitude(correlation_magnitudes, {processing_queue.data() + search_space_start, search_space_size}, sss_full_rate_time);
  for(int64_t i = 0; i < correlation_magnitudes.size(); i++) {
    if(correlation_magnitudes.at(i) >= correlation_max) {
      sss_position = i;
      correlation_max = correlation_magnitudes.at(i);
    }
  }
  sss_position += search_space_start; // Offset is relative to search space start, so true position needs to account for this

  uint64_t sss_ref_position = initial_bwp->samples_per_symbol(0) + initial_bwp->samples_per_symbol(1) + initial_bwp->samples_per_symbol(2) + initial_bwp->samples_per_symbol(3) + initial_bwp->samples_per_cp(4);
  // uint64_t sss_ref_position = initial_bwp->samples_per_symbol(0) + initial_bwp->samples_per_symbol(1) + initial_bwp->samples_per_symbol(2) + initial_bwp->samples_per_symbol(3); // If I get rid of CP I should add + initial_bwp->samples_per_cp(4);
  int64_t timing_error = sss_position - sss_ref_position; // I am yet to understand this. If I move the timing error by 5 samples I get almost 200 more DCIs. 
  SPDLOG_DEBUG("Full-rate samples per symbol: {}", initial_bwp->samples_per_symbol(1));
  SPDLOG_DEBUG("Full-rate SSS hint: {}", sss_hint);
  SPDLOG_DEBUG("Full-rate SSS should be at: {}", sss_ref_position);
  SPDLOG_DEBUG("Full-rate SSS is actually at: {}", sss_position);
  SPDLOG_DEBUG("Fine timing offset based on full-rate SSS: {}", timing_error);

  if(timing_error > 0) {
    /* This could be optimized */
    vector<complex<float>> queue_remainder(timing_error);
    std::copy(processing_queue.begin(), processing_queue.begin() + timing_error, queue_remainder.begin());

    // Send the part that will be erased from the processing queue to any existing flows, so they can still process these samples
    shared_ptr<vector<complex<float>>> processing_queue_remainder = make_shared<vector<complex<float>>>(std::move(queue_remainder));
    send_to_next_workers(processing_queue_remainder, counting_samples);
    counting_samples = counting_samples + processing_queue_remainder->size();
      

    // Tell all currently existing flows to finish processing after the workload they received now
    this->flow_pool->release_flows();

    // Cut the processing queue
    processing_queue.erase(processing_queue.begin(), processing_queue.begin()+timing_error);
  } else {
    // If the correlation is higher for negative offsets, we need to append samples
    for(int i = 0; i < abs(timing_error); i++) {
      processing_queue.insert(processing_queue.begin(), complex<float>(0));
    }
  }
}
