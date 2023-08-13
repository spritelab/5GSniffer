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

#ifndef FLOW_H
#define FLOW_H

#include <fstream>
#include <string>
#include <vector>
#include <complex>
#include <memory>
#include <zmq.hpp>
#include "worker.h"
#include "bandwidth_part.h"
#include "channel_mapper.h"
#include "config.h"

using namespace std;

/**
 * A flow is a worker that processes only synchronized samples, potentially from a remote machine
 */
class flow : public worker {
  public:
    flow();
    virtual ~flow();
    void remote_connect(string ip_address = "127.0.0.1");

    bool sniffer_finished;
  private:
    void hello(string ip_address);
    void configure();

    zmq::context_t main_ctx;
    zmq::socket_t zmq_socket;
    std::string routing_id;
    struct config config;

    vector<shared_ptr<bandwidth_part>> bandwidth_parts;
    vector<shared_ptr<channel_mapper>> channel_mappers;
};

#endif // FLOW_H