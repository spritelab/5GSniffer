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
#include <map>
#include <memory>
#include <zmq.hpp>
#include <semaphore>
#include "flow.h"
#include "utils.h"

using namespace std;

/**
 * A flow_pool is a collection of processing flows (i.e. threads) that can be
 * reserved by a worker to execute a certain flow graph.
 */
class flow_pool : public worker {
  public:
    flow_pool();
    virtual ~flow_pool();
    void process(shared_ptr<vector<complex<float>>>& samples, int64_t metadata) override;
    void configure_flow(uint32_t sample_rate, uint16_t cell_id, uint32_t slots_per_frame, uint32_t ssb_scs, uint8_t mib_scs_common, uint32_t mib_coreset0_index, uint32_t mib_ssb_offset);
    void stop_flow(string routing_id);
    void release_flows();
    void release_flow(string routing_id);
    void release_current_flow();
    void handle_messages();
    void send_queued_zmq_messages();
    void cycle_current_flow();

  private:
    map<string, bool> pool;
    zmq::context_t main_ctx;
    zmq::socket_t zmq_socket;
    shared_ptr<counting_semaphore<>> available_flows;
    thread t;
    bool stop;
    uint32_t flow_index;
    string current_flow;
    uint32_t current_flow_index;
    thread_safe_queue<pair<string, zmq::message_t>> zmq_send_queue;
};

#endif // FLOW_POOL_H