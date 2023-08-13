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

#include "flow.h"
#include "spdlog/spdlog.h"
#include "proto/protocol.pb.h"
#include <asm-generic/errno.h>
#include <cstdint>
#include <memory>
#include <unistd.h>
#include <zmq.hpp>

#include "rotator.h"
#include "ofdm.h"
#include "channel_mapper.h"

using namespace std;

/** 
 * Constructor for flow. A flow will connect over ZeroMQ to a remote endpoint
 * in order to receive synchronized streams of samples or symbols.
 */
flow::flow() {
  finished = false;
  sniffer_finished = false;

  routing_id = "<unassigned>";
}

/**
 * Destructor for flow.
 */
flow::~flow() {
  SPDLOG_DEBUG("Flow {} shutdown", routing_id);
}

/**
 * Sends a Hello message, which announces the flow's presence to the server.
 * The server will respond with the routing ID that the flow should use for
 * subsequent communications. The flow will reconnect to the server using this
 * new routing ID.
 */
void flow::hello(string ip_address) {
  FlowMessage flow_message;
  HelloMessage* hello = new HelloMessage();
  flow_message.set_allocated_hello_message(hello);

  SPDLOG_DEBUG("Sending hello message");
  zmq::message_t msg(flow_message.SerializeAsString());

  bool success = false;
  do {
    try {
      zmq_socket.send(msg, zmq::send_flags::none);
      success = true;
    } catch(zmq::error_t e) {
      if (e.num() != EHOSTUNREACH) {
        SPDLOG_ERROR("ZMQ error: {}", e.what());
      } else {
        usleep(1000); // Wait a millisecond before trying to connect to the flow again
      }
    }
  } while(!success);

  SPDLOG_DEBUG("Waiting for hello response...");
  auto result = zmq_socket.recv(msg, zmq::recv_flags::none);
  
  const char* routing_id_buffer = reinterpret_cast<const char*>(msg.data());

  // Reinstantiate socket with received routing ID
  routing_id = string(routing_id_buffer);
  SPDLOG_DEBUG("Hello response received! Setting routing ID to {}", routing_id);
  zmq_socket.close();
  zmq_socket = zmq::socket_t(main_ctx, zmq::socket_type::dealer);
  SPDLOG_DEBUG("Flow {} connecting to remote endpoint", routing_id);
  zmq_socket.set(zmq::sockopt::linger, 0);
  zmq_socket.set(zmq::sockopt::routing_id, routing_id);
  zmq_socket.connect("tcp://" + ip_address + ":23501");
}

void flow::configure() {
  zmq::message_t msg;

  SPDLOG_DEBUG("Waiting for configure message...");
  auto result = zmq_socket.recv(msg, zmq::recv_flags::none);
  SPDLOG_DEBUG("Got configure message: {}", msg.str());

  ConfigMessage cfg_msg;
  cfg_msg.ParseFromArray(msg.data(), msg.size());

  SPDLOG_DEBUG("Config message: {}", cfg_msg.DebugString());
  SPDLOG_DEBUG("MIB SCS Common is: {}", (uint8_t)cfg_msg.mib_scs_common());

  // Load config here
  this->config = config::load(cfg_msg.config());

  // Build the flowgraph based on the configuration
  for(pdcch_config pdcch_cfg : config.pdcch_configs) {
      // Override config with MIB
      if(pdcch_cfg.use_config_from_mib) {
        assert(cfg_msg.mib_scs_common() <= max_numerology);
        pdcch_cfg.numerology = (uint8_t)cfg_msg.mib_scs_common();
        std::array<uint8_t, 4> coreset0_config = pdcch::get_pdcch_coreset0(5, cfg_msg.ssb_scs(),  15000U << cfg_msg.mib_scs_common(), cfg_msg.mib_coreset0_index());
        pdcch_cfg.num_prbs = coreset0_config.at(1);
        pdcch_cfg.coreset_duration = coreset0_config.at(2);
        pdcch_cfg.subcarrier_offset = cfg_msg.mib_ssb_offset() + (pdcch_cfg.num_prbs/2);
        pdcch_cfg.extended_prefix = false;
      }

      // Simplify brute force if we are looking only for SI DCI
      if (pdcch_cfg.si_dci_only) { 
        pdcch_cfg.scrambling_id_start = cfg_msg.cell_id(); // Scrambling ID for SI DCI is always the cell ID
        pdcch_cfg.scrambling_id_end = cfg_msg.cell_id();
        pdcch_cfg.rnti_start = 65535; // SI-RNTI is always 65535
        pdcch_cfg.rnti_end = 65535;
        pdcch_cfg.coreset_interleaving_pattern = "interleaved";
        pdcch_cfg.coreset_reg_bundle_size = 6;
        pdcch_cfg.coreset_interleaver_size = 2;
        pdcch_cfg.coreset_nshift = cfg_msg.cell_id();
      }

      auto new_bwp = make_shared<bandwidth_part>(cfg_msg.sample_rate(), pdcch_cfg.numerology, pdcch_cfg.num_prbs, pdcch_cfg.extended_prefix);
      bandwidth_parts.push_back(new_bwp);
      
      auto mapper = make_shared<channel_mapper>(cfg_msg.cell_id(), cfg_msg.slots_per_frame(), pdcch_cfg);
      channel_mappers.push_back(mapper);
  }

  // Actually attach the bwps and mappers
  assert(bandwidth_parts.size() == channel_mappers.size());
  SPDLOG_DEBUG("Creating {} bandwidth part processing flows", bandwidth_parts.size());
  for(int i = 0; i < bandwidth_parts.size(); i++) {
      auto bwp = bandwidth_parts.at(i);
      auto mapper = channel_mappers.at(i);
      auto rotator = make_shared<class rotator>(bwp->sample_rate, (float)mapper->pdcch.subcarrier_offset*(float)bwp->scs);
      auto ofdm = make_shared<class ofdm>(bwp);
      this->connect(rotator);
      rotator->connect(ofdm);
      ofdm->connect(mapper);
  }
}

void flow::remote_connect(string ip_address) {
  zmq_socket = zmq::socket_t(main_ctx, zmq::socket_type::dealer);

  //zmq_socket.set(zmq::sockopt::routing_id, routing_id);
  zmq_socket.set(zmq::sockopt::linger, 0);
  SPDLOG_DEBUG("Flow {} connecting to remote endpoint", routing_id);
  zmq_socket.connect("tcp://" + ip_address + ":23501");

  // Wait until we are connected to the sender
  hello(ip_address);

  // Build the flowgraph from received configuration
  configure();

  while(!finished) {
    // Receive a new message
    zmq::message_t msg;
    auto result = zmq_socket.recv(msg, zmq::recv_flags::none);
    SPDLOG_DEBUG("Received {} bytes from {}", *result, routing_id);

    // Parse it
    FlowMessage flow_message;
    flow_message.ParseFromArray(msg.data(), msg.size());
    //SPDLOG_DEBUG("Flow message: {}", flow_message.DebugString());

    if(flow_message.has_data_message()) {
      // Convert to vector
      const complex<float>* msg_data = reinterpret_cast<const complex<float>*>(flow_message.data_message().data().data());
      auto samples = make_shared<vector<complex<float>>>(msg_data, msg_data + (flow_message.data_message().data().size() / sizeof(complex<float>)));
      /*for(auto x : *samples) {
        SPDLOG_DEBUG("{} {}i", x.real(), x.imag());
      }*/
      int64_t metadata = 0;
      if(flow_message.data_message().has_sample_index()) {
        metadata = flow_message.data_message().sample_index();
      }
      SPDLOG_DEBUG("Going to rotate {} samples metadata {}", samples->size(), metadata);
      this->send_to_next_workers(samples, metadata);
    } else if(flow_message.has_reset_message()) {
      SPDLOG_DEBUG("Resetting flow");
      this->reset_all();
    } else if(flow_message.has_stop_message()) {
      SPDLOG_DEBUG("Received stop message; stopping worker");
      finished = true;
    } else {
      SPDLOG_DEBUG("Unknown message received.");
    }
  }

  this->disconnect_all();
}