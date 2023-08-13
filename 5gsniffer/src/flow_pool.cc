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

#include "flow_pool.h"
#include <cstdint>
#include <exception>
#include <memory>
#include <spdlog/spdlog.h>
#include <utility>
#include <zmq.hpp>
#include "flow.h"
#include "proto/protocol.pb.h"
#include "config.h"

extern struct config config;

/** 
 * Constructor for flow_pool.
 */
flow_pool::flow_pool() {
  stop = false;
  flow_index = 0;
  current_flow = "";
  current_flow_index = 0;

  // Create available flows semaphore
  available_flows = make_shared<counting_semaphore<>>(0);

  t = thread(&flow_pool::handle_messages, this);
}

flow_pool::~flow_pool() {
  SPDLOG_DEBUG("Sending stop message to all flows...");

  // Send stop message to all flows
  for (map<string, bool>::iterator it = this->pool.begin(); it != this->pool.end(); ++it) {
    stop_flow((*it).first);
  }

  // Stop listening for new messages
  SPDLOG_DEBUG("Stopping recv thread...");
  stop = true;
  t.join();
}

void flow_pool::handle_messages() {
  // Create zmq server socket
  zmq_socket = zmq::socket_t(main_ctx, zmq::socket_type::router);
  zmq_socket.set(zmq::sockopt::router_mandatory, 1);
  zmq_socket.set(zmq::sockopt::rcvtimeo, 100);
  zmq_socket.bind("tcp://" + config.bind_ip + ":23501");

  while(!stop) {
    zmq::message_t msg;
    //SPDLOG_DEBUG("Waiting for message");
    auto result = zmq_socket.recv(msg, zmq::recv_flags::none);
    if(result.has_value()) { // We received an ID
      const char* recv_routing_id = reinterpret_cast<const char*>(msg.data());
      string recv_routing_id_str = string(recv_routing_id, result.value());
      SPDLOG_TRACE("Message from: {} ({} bytes) {}", recv_routing_id_str, result.value(), msg.str());
      result = zmq_socket.recv(msg, zmq::recv_flags::none);
      if(result.has_value()) {
        SPDLOG_TRACE("{}", msg.str());

        FlowMessage flow_message;
        flow_message.ParseFromArray(msg.data(), *result);
        SPDLOG_DEBUG("{}", flow_message.DebugString());

        // Check which message we have and handle accordingly
        if(flow_message.has_hello_message()) {
          SPDLOG_DEBUG("Got hello message. Assigning new flow index to this flow");
          string flow_id = "flow_" + to_string(flow_index);
          flow_index += 1;
          this->pool[flow_id] = false;
          this->available_flows->release(); // Signal that we have a node that is ready to process data

          // Send HELLO response
          zmq::message_t identifier(recv_routing_id_str);
          zmq::message_t new_identifier(flow_id);
          SPDLOG_DEBUG("Sending hello response to {}, assigning it to {}", identifier.str(), new_identifier.str());
          zmq_socket.send(std::move(identifier), zmq::send_flags::sndmore);
          zmq_socket.send(std::move(new_identifier), zmq::send_flags::none);
        }
      }
    }

    // Send all queued messages
    SPDLOG_TRACE("{} messages in queue", this->zmq_send_queue.size());
    send_queued_zmq_messages();
  }
  SPDLOG_DEBUG("Recv thread stopped");

  // Wait for some time for any remaining messages to arrive
  sleep(2);
  SPDLOG_DEBUG("{} final messages in queue", this->zmq_send_queue.size());
  send_queued_zmq_messages();
  zmq_socket.close();
}

void flow_pool::send_queued_zmq_messages() {
  size_t msgs_to_send = zmq_send_queue.size();
  for(size_t i = 0; i < msgs_to_send; i++) {
    bool retry = false;
    pair<string, zmq::message_t> entry = zmq_send_queue.pop();
    zmq::message_t identifier(entry.first);
    do {
      try {
        SPDLOG_DEBUG("Sending message {} to {}: {}", i, entry.first, entry.second.str());
        zmq_socket.send(identifier, zmq::send_flags::sndmore);
        zmq_socket.send(entry.second, zmq::send_flags::none);
        retry = false;
      } catch(zmq::error_t e) {
        usleep(1000); // TODO check for other errors besides EHOSTUNREACH
        retry = true;
        SPDLOG_DEBUG("{} unreachable", entry.first);
      }
    } while(retry);
  }
}

void flow_pool::configure_flow(uint32_t sample_rate, uint16_t cell_id, uint32_t slots_per_frame, uint32_t ssb_scs, uint8_t mib_scs_common, uint32_t mib_coreset0_index, uint32_t mib_ssb_offset) {
  // Build config
  SPDLOG_DEBUG("Configuring flow");
  ConfigMessage cfg_msg;
  cfg_msg.set_config(config.config_path);
  cfg_msg.set_sample_rate(sample_rate);
  cfg_msg.set_cell_id(cell_id);
  cfg_msg.set_slots_per_frame(slots_per_frame);
  cfg_msg.set_ssb_scs(ssb_scs);
  cfg_msg.set_mib_scs_common(mib_scs_common);
  cfg_msg.set_mib_coreset0_index(mib_coreset0_index);
  cfg_msg.set_mib_ssb_offset(mib_ssb_offset);
  
  SPDLOG_DEBUG("Flow configuration: {}\n", cfg_msg.DebugString());

  // Wait for at least one flow to become available
  available_flows->acquire();

  // Find the first available unconfigured flow
  bool found_unconfigured_flow = false;
  for (map<string, bool>::iterator it = this->pool.begin(); it != this->pool.end(); ++it) {
    if(!(*it).second) {
      found_unconfigured_flow = true;
      SPDLOG_DEBUG("Found unconfigured flow {}; configuring it", (*it).first);
      zmq::message_t payload(cfg_msg.SerializeAsString());
      zmq_send_queue.push({(*it).first, std::move(payload)});

      it->second = true; // Signal the flow is ready to receive data
      current_flow = (*it).first; // Use this flow for upcoming data
      return;
    }
  }

  if(!found_unconfigured_flow) {
    SPDLOG_DEBUG("All connected flows already configured; skipping configuration");
    return;
  }

  SPDLOG_DEBUG("No flows available");
  throw sniffer_exception("Flow pool semaphore indicated a flow is available, but this was not the case.");
}

void flow_pool::release_flow(string routing_id) {
  FlowMessage flow_message;
  ResetMessage* reset_message = new ResetMessage();

  flow_message.set_allocated_reset_message(reset_message);
  zmq::message_t msg(flow_message.SerializeAsString());
  SPDLOG_DEBUG("Sending reset message to {}", routing_id);
  zmq_send_queue.push({routing_id, std::move(msg)});

  // TODO Only do this after receiving an ack from the node?
  available_flows->release();

  this->cycle_current_flow();
}
void flow_pool::release_current_flow() {
  if(current_flow.compare("") != 0) {
    release_flow(current_flow);
  }
}

void flow_pool::release_flows() {
  for (map<string, bool>::iterator it = this->pool.begin(); it != this->pool.end(); ++it) {
    if((*it).second) { // Release all configured flows
      this->release_flow((*it).first);
    }
  }
}

/** 
 * 
 * 
 * @param samples shared_ptr to sample buffer
 */
void flow_pool::process(shared_ptr<vector<complex<float>>>& samples, int64_t metadata_) {
  FlowMessage flow_message;
  flow_message.mutable_data_message()->set_data(samples->data(), samples->size() * sizeof(std::complex<float>));
  flow_message.mutable_data_message()->set_sample_index(metadata_);

  if(current_flow.compare("") == 0) {
    SPDLOG_ERROR("No flows configured yet, discarding {} samples", samples->size());
    return;
  }
  zmq::message_t msg(flow_message.SerializeAsString());
  SPDLOG_DEBUG("Sending {} samples ({} bytes) to {}", samples->size(), msg.size(),  current_flow);
  zmq_send_queue.push({current_flow, std::move(msg)});
}

void flow_pool::stop_flow(string routing_id) {
  FlowMessage flow_message;
  StopMessage* stop_message = new StopMessage();

  flow_message.set_allocated_stop_message(stop_message);
  SPDLOG_DEBUG("Sending stop message to {}: {}", routing_id, flow_message.DebugString());

  zmq::message_t msg(flow_message.SerializeAsString());
  zmq_send_queue.push({routing_id, std::move(msg)});

  this->available_flows->release();
}

void flow_pool::cycle_current_flow() {
  size_t num_flows = this->pool.size();
  if(num_flows == 0) {
    current_flow = "";
    return;
  }

  current_flow_index += 1;
  if(current_flow_index >= num_flows)
    current_flow_index = 0;

  current_flow = "flow_" + to_string(current_flow_index);
}