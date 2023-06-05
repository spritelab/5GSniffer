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

#include "symbol.h"
#include "utils.h"
#include <cstdint>
#include <spdlog/spdlog.h>
#include <volk/volk.h>

/** 
 * Constructor for symbol.
 */
symbol::symbol() {
  SPDLOG_TRACE("Construct {}", (uint64_t)this);
  is_equalized = false;
}

/** 
 * Destructor for symbol.
 */
symbol::~symbol() {
  SPDLOG_TRACE("Destruct {}", (uint64_t)this);
}

/** 
 * Copy constructor for symbol.
 */
symbol::symbol(const symbol& other) {
  SPDLOG_TRACE("Copy {} -> {}", (uint64_t)&other, (uint64_t)this);
  sample_index = other.sample_index;
  samples = other.samples;
  samples_eq = other.samples_eq;
  noise = other.noise;
  symbol_index = other.symbol_index;
  slot_index = other.slot_index;
  channel_filter = other.channel_filter;
  is_equalized = other.is_equalized;
};

/** 
 * Move constructor for symbol.
 */
symbol::symbol(symbol&& other) {
  SPDLOG_TRACE("Move {} -> {}", (uint64_t)&other, (uint64_t)this);
  sample_index = std::move(other.sample_index);
  samples = std::move(other.samples);
  samples_eq = std::move(other.samples_eq);
  noise = std::move(other.noise);
  symbol_index = other.symbol_index;
  slot_index = other.slot_index;
  channel_filter = std::move(other.channel_filter);
  is_equalized = other.is_equalized;
};

/** 
 * Copy assignment constructor for symbol.
 */
symbol& symbol::operator=(const symbol& other) {
  SPDLOG_TRACE("Copy= {} -> {}", (uint64_t)&other, (uint64_t)this);
  symbol(other).swap(*this); // Copy to tmp object and swap with this
  return *this;
}

/** 
 * Move assignment constructor for symbol.
 */
symbol& symbol::operator=(symbol&& other) {
  SPDLOG_TRACE("Move= {} -> {}", (uint64_t)&other, (uint64_t)this);
  this->swap(other);
  return *this;
}

/** 
 * Swap the contents of one symbol with another.
 */
void symbol::swap(symbol& other) {
  SPDLOG_TRACE("Swap {} -> {}", (uint64_t)&other, (uint64_t)this);
  std::swap(sample_index, other.sample_index);
  std::swap(samples, other.samples);
  std::swap(samples_eq, other.samples_eq);
  std::swap(noise, other.noise);
  std::swap(symbol_index, other.symbol_index);
  std::swap(slot_index, other.slot_index);
  std::swap(channel_filter, other.channel_filter);
  std::swap(is_equalized, other.is_equalized);
};

/** 
 * Get span of resource elements from start_index to end_index of the symbol. If
 * the symbol was previously equalized, returns the equalized resource elements.
 */
span<complex<float>> symbol::get_res(size_t start_index, size_t end_index) {
  if (is_equalized)
    return {this->samples_eq.begin() + start_index, end_index - start_index + 1};
  else
    return {this->samples.begin() + start_index, end_index - start_index + 1};
}

void symbol::channel_estimate(
  const vector<complex<float>>& dmrs_reference,
  const vector<uint64_t>& dmrs_indices,
  uint64_t subcarrier_start,
  uint64_t subcarrier_end)
{
  // SPDLOG_DEBUG("Channel estimating {} DMRS symbols (start at {}) in size {} symbol", dmrs_indices.size(), dmrs_indices.at(0), samples.size());

  // Resize vectors
  channel_filter.resize(samples.size(), complex<float>(1.0f, 0.0f));
  noise.resize(samples.size(), complex<float>(0.0f, 0.0f));
  samples_eq.resize(samples.size());

  uint64_t ref_index = 0;
  uint64_t prev = subcarrier_start;
  vector<complex<float>> test_channel_noninterp; // For testing purposes only; temp buffer to write to file
  vector<complex<float>> test_dmrs_reference;
  for(auto dmrs_index : dmrs_indices) {
    // Determine the channel filter
    channel_filter[dmrs_index] = samples[dmrs_index] * conj(dmrs_reference[ref_index]);
    test_channel_noninterp.push_back(channel_filter[dmrs_index]);
    test_dmrs_reference.push_back(dmrs_reference[ref_index]);
    SPDLOG_TRACE("Setting channel_filter[{}] = ({}, {})", dmrs_index, real(channel_filter[dmrs_index]), imag(channel_filter[dmrs_index]));
    assert(prev <= dmrs_index);

    // Interpolate between previous values
    float distance = dmrs_index - prev;
    if (distance > 1) {
      complex<float> distance_vec = (channel_filter[dmrs_index] - channel_filter[prev]) / distance;
      for(uint64_t j = prev+1; j < dmrs_index; j++) {
        channel_filter[j] = channel_filter[j-1] + distance_vec;
        SPDLOG_TRACE("Interp  channel_filter[{}] = ({}, {})", j, real(channel_filter[j]), imag(channel_filter[j]));
      }
    }
    prev = dmrs_index;
    ref_index += 1;
  }

  // TODO interpolate between prev and last?

  // Apply channel estimation
  // Computing the average channel estimate, as a complex number, not the accumulate absolute value.
  auto average_channel_estimate = get_average_channel();  
  float square_average_channel_magnitude = pow(get_average_channel_magnitude(),2);

  for(size_t i = 0; i < samples.size(); i++) {
    samples_eq[i] = (samples[i] * conj(channel_filter[i])) / square_average_channel_magnitude;
    noise[i] = channel_filter[i] - average_channel_estimate;
    SPDLOG_TRACE("Initial samples[{}] = ({}, {})", i, real(samples[i]), imag(samples[i]));
    SPDLOG_TRACE("Channel[{}] = ({}, {})", i, real(channel_filter[i]), imag(channel_filter[i]));
    SPDLOG_TRACE("Result samples[{}] = ({}, {})", i, real(samples_eq[i]), imag(samples_eq[i]));
  }
  this->is_equalized = true;
}

/** 
 * Get average magnitude of the noise over the subcarriers of the symbol.
 */
float symbol::get_average_noise_magnitude() const {
  float total = 0.0;

  for(auto n : noise) {
    total += abs(n);
  }

  return total / noise.size();
}

/** 
 * Get average magnitude of the channel over the subcarriers of the symbol.
 */
float symbol::get_average_channel_magnitude() const {
  float total = 0.0;

  for(auto c : channel_filter) {
    total += abs(c);
  }

  return total / channel_filter.size();
}

/** 
 * Get average -complex value- of the channel over the subcarriers of the symbol.
 */
complex<float> symbol::get_average_channel() const {
  complex<float> total = 0.0;

  for(auto c : channel_filter) {
    total += (c);
  }

  return total / float(channel_filter.size());
}


/** 
 * Get average magnitude over the subcarriers of the symbol.
 */
float symbol::get_average_magnitude() const {
  float total = 0.0;

  for(auto s : samples) {
    total += abs(s);
  }

  return total / samples.size();
}

/** 
 * Normalize the magnitude of the symbol.
 */
void symbol::normalize() {
  uint32_t max_index = 0;
  volk_32fc_index_max_32u(&max_index, samples.data(), samples.size());

  float max_value = abs(samples[max_index]);
  if(max_value > 0.0) {
    for(size_t i = 0; i < samples.size(); i++) {
      samples[i] /= max_value;
    }
  }
}