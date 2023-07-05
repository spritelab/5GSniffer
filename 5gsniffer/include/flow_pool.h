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

#ifndef FLOW_POOL_H
#define FLOW_POOL_H

#include <cstdint>
#include <vector>
#include <memory>
#include <zmq.hpp>
#include <semaphore>
#include "flow.h"

using namespace std;

/**
 * A flow_pool is a collection of processing flows (i.e. threads) that can be
 * reserved by a worker to execute a certain flow graph.
 */
class flow_pool : public worker {
  public:
    flow_pool(uint64_t max_flows);
    virtual ~flow_pool();
    void process(shared_ptr<vector<complex<float>>>& samples, int64_t metadata) override;
    shared_ptr<flow> acquire_flow();
    void release_flows();
  private:
    vector<shared_ptr<flow>> pool;
    vector<shared_ptr<flow>> acquired_flows;
    zmq::context_t main_ctx;
    zmq::socket_t zmq_socket;
    shared_ptr<counting_semaphore<>> available_flows;
    uint64_t max_flows;
};

#endif // FLOW_POOL_H