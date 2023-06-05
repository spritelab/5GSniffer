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

#include <complex>
#include <cstdint>
#include <vector>
#include <span>
#include <spdlog/spdlog.h>
#include "ofdm.h"
#include "utils.h"
#include "symbol.h"

/** 
 * Constructor for ofdm.
 */
ofdm::ofdm(shared_ptr<bandwidth_part> bwp, float cyclic_prefix_fraction) :
  cyclic_prefix_fraction(cyclic_prefix_fraction),
  samples_processed(0),
  symbol_index(0),
  slot_index(0) {
  this->bwp = bwp;
  leftover_samples.reserve((bwp->samples_per_symbol(1)) - 1); // Worst case scenario, almost 1 symbol with extended CP
  SPDLOG_DEBUG("Creating OFDM block with nfft={}", bwp->fft_size);
  std::remove(get_symbol_dump_path_name().c_str());
}


/** 
 * Destructor for ofdm.
 */
ofdm::~ofdm() {
  
}


// /** 
//  * Transform samples into OFDM symbols. More efficient implementation. Does not support fractional CP removal.
//  *
//  * @param samples shared_ptr to sample buffer to process
//  * @param metadata for future work
//  */
void ofdm::process(shared_ptr<vector<complex<float>>>& samples, int64_t metadata) {
  SPDLOG_DEBUG("Starting OFDM demodulation");

  vector<symbol> produced_symbols;
  produced_symbols.reserve(samples->size() / bwp->samples_per_symbol(1)); // Reserve space for worst case number of symbols
  int symbol_ctr = 0;
  vector<complex<float>> input_data(bwp->fft_size);
  vector<complex<float>> symbol_fft_full(bwp->fft_size);

  int flags = 0;
  fftplan q = fft_create_plan(bwp->fft_size, (input_data.data()), symbol_fft_full.data(), LIQUID_FFT_FORWARD, flags);

  int num_leftover_samp = leftover_samples.size();
  // If there are leftover samples from a previous buffer, we should process that OFDM symbol first, as pre-appending the leftover
  // samples to the new incoming buffer is expensive.
  
  while (samples->size() - symbol_ctr >= bwp->samples_per_symbol(symbol_index)) {
    // Keep track of OFDM symbol start, and copy new symbol
    int curr_position = symbol_ctr + bwp->samples_per_cp(symbol_index);
    vector<complex<float>> symbol_fft;
    symbol_fft.reserve(bwp->num_subcarriers);

    if (leftover_samples.size() > 0){
      leftover_samples.insert(leftover_samples.end(), samples->begin(), samples->begin() + (bwp->samples_per_symbol(symbol_index) - leftover_samples.size()));
      std::copy(leftover_samples.begin() + curr_position, leftover_samples.begin() + curr_position + bwp->fft_size, input_data.begin());
      symbol_ctr = symbol_ctr - num_leftover_samp;
      leftover_samples.clear();
    }else{
      std::copy(samples->begin() + curr_position, samples->begin() + curr_position + bwp->fft_size, input_data.begin());
    }
    // Perform FFT  
    fft_execute(q);
    
    // FFT shift + extract subcarriers
    symbol_fft.insert(symbol_fft.end(), symbol_fft_full.end() - (bwp->num_subcarriers/2), symbol_fft_full.end());
    symbol_fft.insert(symbol_fft.end(), symbol_fft_full.begin(), symbol_fft_full.begin() + (bwp->num_subcarriers/2));

    // Create symbol class and add to vector
    symbol s;
    s.sample_index = this->samples_processed;
    s.samples = std::move(symbol_fft);
    s.symbol_index = symbol_index;
    s.slot_index = slot_index;
    produced_symbols.push_back(std::move(s));

    // Counter for OFDM symbol and slot number
    symbol_ctr = symbol_ctr + bwp->samples_per_symbol(symbol_index);

    symbol_index += 1;
    if(symbol_index == bwp->symbols_per_slot) {
      slot_index = (slot_index + 1) % bwp->slots_per_frame;
    }
    symbol_index %= bwp->symbols_per_subframe;

    // Keep track of the total number of samples processed.
    samples_processed += bwp->samples_per_symbol(symbol_index);
  }
  
  // Keeping leftover samples for next input
  leftover_samples.insert(leftover_samples.begin(),(*samples.get()).begin() + symbol_ctr - 1, (*samples.get()).end());
  leftover_samples.resize((*samples).size() - symbol_ctr);
  // Pass produced symbols on to symbol workers
  fft_destroy_plan(q);

  if (produced_symbols.size() > 0)
    send_to_next_workers(make_shared<vector<symbol>>(std::move(produced_symbols)), metadata);
}


/** 
 * OFDM modulation. Could be used to generate the complete SSB in time domain for fine time sync. Currently unused.
 */
vector<complex<float>> ofdm::modulate(vector<symbol>& symbols) {
  vector<complex<float>> time_samples;
  time_samples.reserve(symbols.size() * (bwp->fft_size + bwp->samples_per_cp(0))); // Reserve space for worst case number of symbols

  //Compute IFFT of each symbol, add CP at the end, and concatenate all OFDM symbols. Input Grid assumed padded to FFT size.
  for (symbol symbol: symbols) {
    vector<complex<float>> symbol_freq = symbol.samples;
    vector<complex<float>> symbol_time(bwp->fft_size + bwp->samples_per_cp(symbol.symbol_index),0);

    /*Perform ifft shift before the transform*/
    std::rotate(symbol_freq.begin(), symbol_freq.begin() + (bwp->fft_size)/2, symbol_freq.end());
    
    /*Create liquid FFT plan*/
    int flags = 0;
    fftplan q = fft_create_plan(bwp->fft_size, symbol_freq.data(), symbol_time.data(), LIQUID_FFT_BACKWARD, flags);

    /*execute IFFT*/ 
    fft_execute(q);
    /*destroy FFT plan*/
    fft_destroy_plan(q);

    // Adding Cyclic Prefix samples
    vector<complex<float>> CP(symbol_time.end() - bwp->samples_per_cp(symbol.symbol_index), symbol_time.end());
    CP.insert(CP.end(), symbol_time.begin(), symbol_time.end()); // Appending the symbol itself to the CP so we avoid inserting at the beginning (inefficient)
    time_samples.insert(time_samples.end(), CP.begin(), CP.end());
  }
  return time_samples;
}


string ofdm::get_symbol_dump_path_name() {
  stringstream ss;
  ss << "/tmp/symbols_" << this->bwp->num_subcarriers;
  return ss.str();
}