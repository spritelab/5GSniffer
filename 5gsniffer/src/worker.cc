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

#include "worker.h"
#include <cstddef>
#include <spdlog/spdlog.h>

/** 
 * Constructor for worker.
 */
worker::worker() {
  finished = false;
  total_produced_samples = 0;
}

/** 
 * Destructor for worker.
 */
worker::~worker() {

}

/** 
 * Produce num_symbols symbols for subsequent workers.
 *
 * @param num_symbols number of symbols to produce
 */
shared_ptr<vector<symbol>> worker::produce_symbols(size_t num_symbols) {
  // If we ask a non-overriden symbol_worker to produce symbols, just return an empty vector
  return make_shared<vector<symbol>>(vector<symbol>(num_symbols));
}

/** 
 * Function called by work() in case the worker is a producer.
 *
 * @param num_samples number of samples to produce
 */
shared_ptr<vector<complex<float>>> worker::produce_samples(size_t num_samples) {
  // If we ask a non-overriden worker to produce samples, just return an empty vector
  return make_shared<vector<complex<float>>>(vector<complex<float>>(num_samples));
}

/** 
 * Used to connect other workers to this worker. A worker will ass a
 * shared_ptr to the sample buffer to all next workers for subsequent processing.
 *
 * @param worker pointer to a worker
 */
void worker::connect(shared_ptr<worker> w) {
  this->next_workers.push_back(w);
}

/** 
 * Used to disconnect another worker from this worker.
 *
 * @param worker pointer to a worker
 */
void worker::disconnect(shared_ptr<worker> w) {
  this->next_workers.erase(find(this->next_workers.begin(), this->next_workers.end(), w));
}

/** 
 * Used to disconnect all subsequent workers from this worker.
 */
void worker::disconnect_all() {
  this->next_workers.clear();
}


/** 
 * When calling work with size_t argument, call produce_samples to create
 * samples for other workers to process.
 *
 * @param num_samples number of samples to produce.
 */
void worker::work(size_t num_samples) {
  shared_ptr<vector<complex<float>>> produced_samples = this->produce_samples(num_samples);
  this->send_to_next_workers(produced_samples, total_produced_samples);
}

/**
 * Signals that the worker is done and should not process any further samples.
 * At this point it should be safe to disonnect the worker without losing any
 * pending work. This function can be overridden to wait for a certain task
 * to complete before setting finished to true.
 */
void worker::finish() {
  finished = true;
}

/**
 * Disconnect any workers that have indicated that they are finished processing
 * their workload.
 */
void worker::disconnect_finished() {
  auto it = this->next_workers.begin();
  while(it != this->next_workers.end()) {
    if(it->get()->finished) {
      it = this->next_workers.erase(it);
    } else {
      it++;
    }
  }
}

/**
 * Tell all next workers to finish their processing tasks. If they do so
 * immediately (if they did not override finish()), they are also automatically
 * disconnected so their resources are freed.
 */
void worker::finish_next_workers() {
  for (const auto& worker : this->next_workers) {
    worker->finish();
  }
  disconnect_finished();
}

const size_t worker::num_next_workers() {
  return this->next_workers.size();
}

void worker::reset() {
  
}

void worker::reset_all() {
  this->reset();
  for (const auto& worker : this->next_workers) {
    worker->reset_all();
  }
}