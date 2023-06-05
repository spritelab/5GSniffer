/*
 * Copyright (c) 2022.
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

#include "flow_pool.h"
#include <cstdint>
#include <exception>
#include <memory>
#include <spdlog/spdlog.h>

/** 
 * Constructor for flow_pool.
 */
flow_pool::flow_pool(uint64_t max_flows) :
  max_flows(max_flows) {
  // Create available flows semaphore
  available_flows = make_shared<counting_semaphore<>>(0);

  // Create zmq server socket
  zmq_socket = zmq::socket_t(main_ctx, zmq::socket_type::router);
  zmq_socket.set(zmq::sockopt::router_mandatory, 1);
  zmq_socket.bind("tcp://127.0.0.1:23501");

  this->pool.reserve(max_flows);
  for(uint64_t i = 0; i < max_flows; i++) {
    this->pool.push_back(make_shared<flow>(i, zmq_socket, available_flows));
  }
}

flow_pool::~flow_pool() {
  this->release_flows();
  
  // Wait for all flows to finish
  for(uint64_t i = 0; i < max_flows; i++) {
    available_flows->acquire();
  }

  // Set sniffer_finished to true so all available flows stop
  SPDLOG_DEBUG("All flows finished. Stopping threads...");
  for(uint64_t i = 0; i < max_flows; i++) {
    this->pool.at(i)->sniffer_finished = true;
    this->pool.at(i)->finish();
  }
}

shared_ptr<flow> flow_pool::acquire_flow() {
  // Wait for at least one flow to become available
  available_flows->acquire();

  // Find the first available flow
  for (vector<shared_ptr<flow>>::iterator it = this->pool.begin(); it != this->pool.end(); ++it) {
    if((*it)->available) {
      (*it)->available = false;
      this->acquired_flows.push_back(*it);
      return *it;
    }
  }

  throw sniffer_exception("Flow pool semaphore indicated a flow is available, but this was not the case.");
}

void flow_pool::release_flows() {
  for (vector<shared_ptr<flow>>::iterator it = this->acquired_flows.begin(); it != this->acquired_flows.end(); ++it) {
    (*it)->finish();
  }
  this->acquired_flows.clear();
}

/** 
 * 
 *
 * @param samples shared_ptr to sample buffer
 */
void flow_pool::process(shared_ptr<vector<complex<float>>>& samples, int64_t metadata_) {
  // Send samples to all acquired flows
  for (vector<shared_ptr<flow>>::iterator it = this->acquired_flows.begin(); it != this->acquired_flows.end(); ++it) {
    (*it)->process(samples, metadata_);
  }
}