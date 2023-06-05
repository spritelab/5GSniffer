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

#include "flow.h"
#include "spdlog/spdlog.h"
#include <asm-generic/errno.h>
#include <unistd.h>
#include <zmq.hpp>

using namespace std;

/** 
 * Constructor for flow.
 *
 * @param path path to the file to write
 */
flow::flow(uint64_t flow_id, zmq::socket_ref send_socket, shared_ptr<counting_semaphore<>> available_flows) :
  flow_id(flow_id),
  available_flows(available_flows) {
  finished = false;
  sniffer_finished = false;
  available = false;

  stringstream ss;
  ss << "flow_" << flow_id;
  routing_id = ss.str();
  metadata = 0;

  this->send_socket = send_socket;

  t = thread(&flow::handle_messages, this);
  SPDLOG_DEBUG("Created flow with thread id {}", std::hash<thread::id>{}(t.get_id()));

  // Block until we receive the start message acknowledgement from the flow thread
  wait_for_start_message_ack(send_socket);
}

flow::~flow() {
  // Wait for thread to finish
  t.join();
  SPDLOG_DEBUG("Flow {} shutdown", routing_id);
}

void flow::set_available() {
  assert(this->num_next_workers() == 0); // A flow that is considered available shouldn't already be busy processing something else
  SPDLOG_DEBUG("Flow {} became available", routing_id);
  available = true;
  this->available_flows->release();
}

void flow::wait_for_start_message_ack(zmq::socket_ref send_socket) {
  SPDLOG_DEBUG("Waiting for {} start message acknowledgement", routing_id);
  zmq::message_t identifier(routing_id);
  zmq::message_t empty;

  bool success = false;
  do {
    try {
      send_socket.send(identifier, zmq::send_flags::sndmore);
      send_socket.send(empty, zmq::send_flags::none);
      success = true;
    } catch(zmq::error_t e) {
      if (e.num() != EHOSTUNREACH) {
        SPDLOG_ERROR("ZMQ error: {}", e.what());
      } else {
        usleep(1000); // Wait a millisecond before trying to connect to the flow again
      }
    }
  } while(!success);
}

void flow::wait_for_start_message(zmq::socket_ref receive_socket) {
  zmq::message_t msg;

  SPDLOG_DEBUG("Waiting for {} start message", routing_id);
  auto result = receive_socket.recv(msg, zmq::recv_flags::none);
  assert(*result == 0);
  SPDLOG_DEBUG("Start message received!");

  this->set_available();
}

void flow::finish() {
  // Send stop event to thread (empty message)
  SPDLOG_DEBUG("Sending stop message to {}", routing_id);
  try {
    zmq::message_t identifier(routing_id);
    zmq::message_t empty;
    send_socket.send(std::move(identifier), zmq::send_flags::sndmore);
    auto result = send_socket.send(std::move(empty), zmq::send_flags::none);
  } catch(zmq::error_t e) {  // Ignore stop message that don't arrive because the flow already stopped
    if (e.num() != EHOSTUNREACH) {
      SPDLOG_ERROR("ZMQ error: {}", e.what());
    }
  }
}

/** 
 * 
 *
 * @param samples shared_ptr to sample buffer
 */
void flow::process(shared_ptr<vector<complex<float>>>& samples, int64_t metadata_) {
  zmq::message_t identifier(routing_id);
  zmq::message_t payload(samples->begin(), samples->end()); // I MIGHT HAVE TO ADD THE METADATA HERE.
  // payload.append(metadata_);
  metadata = metadata_;
  // zmq::message_t meta(metadata_);
  
  send_socket.send(std::move(identifier), zmq::send_flags::sndmore);
  // send_socket.send(std::move(meta), zmq::send_flags::sndmore);
  auto result = send_socket.send(std::move(payload), zmq::send_flags::none);
  SPDLOG_DEBUG("Sent {} bytes to {}", *result, routing_id);
  SPDLOG_DEBUG("Metadata is {}", metadata_);
}

void flow::handle_messages() {
  zmq::context_t thread_ctx;
  zmq::socket_t receive_socket(thread_ctx, zmq::socket_type::dealer);

  receive_socket.set(zmq::sockopt::routing_id, routing_id);
  receive_socket.set(zmq::sockopt::linger, 0);
  receive_socket.connect("tcp://127.0.0.1:23501");

  // Wait until we are connected to the sender
  wait_for_start_message(receive_socket);

  while(!sniffer_finished) {
    finished = false;

    while(!finished) {
      // Receive a new message
      zmq::message_t msg;
      auto result = receive_socket.recv(msg, zmq::recv_flags::none);
      SPDLOG_DEBUG("Received {} bytes from {} metadata {}", *result, routing_id, metadata);
      if(*result == 0) {
        finished = true;
        continue;
      }

      // Convert to vector
      complex<float>* msg_data = reinterpret_cast<complex<float>*>(msg.data());
      auto samples = make_shared<vector<complex<float>>>(msg_data, msg_data + (msg.size() / sizeof(complex<float>)));
      SPDLOG_DEBUG("Going to RoTate");
      this->send_to_next_workers(samples,metadata);
    }

    this->disconnect_all();
    this->set_available();
  }
}