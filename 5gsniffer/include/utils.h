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

#ifndef UTILS_H
#define UTILS_H

#include <cstdint>
#include <vector>
#include <span>
#include <complex>
#include <fstream>
#include <iostream>
#include <chrono>
#include <queue>
#include <spdlog/spdlog.h>
#include <zmq.hpp>
#include <condition_variable>
#include <thread>

using namespace std;

template <class T>
void dump_to_file(string path, span<T> input, bool append = false) {
  size_t size_bytes = input.size() * sizeof(T);
  std::ios::openmode open_mode = std::ios::binary;
  if(append)
    open_mode |= std::ios::app;
  
  ofstream f(path, open_mode);
  f.write(reinterpret_cast<const char*>(input.data()), size_bytes);
  f.close();
}

template <class T>
void print_vector(vector<T>& input) {
  for(auto e : input) {
    std::cout << e << " ";
  }
  std::cout << std::endl;
}

inline chrono::system_clock::time_point time_profile_start(void) {
  #ifdef DEBUG_BUILD
    return chrono::high_resolution_clock::now();
  #else
    return chrono::system_clock::time_point(); // Arbitrary value
  #endif
}

inline void time_profile_end(chrono::system_clock::time_point start_time_point, string function_name) {
  #ifdef DEBUG_BUILD
    auto end_time_point = std::chrono::high_resolution_clock::now();
    chrono::duration<double> time_difference = end_time_point - start_time_point;
    chrono::microseconds microseconds = chrono::duration_cast<chrono::microseconds>( time_difference );
    SPDLOG_DEBUG("{} took {} microseconds", function_name, microseconds.count());
  #endif
}

// From https://codereview.stackexchange.com/questions/267847/thread-safe-message-queue
template <typename T>
class thread_safe_queue {
    std::mutex mutex;
    std::condition_variable cond_var;
    std::queue<T> queue;

public:
  void push(T&& item) {
    {
      std::lock_guard lock(mutex);
      queue.push(std::move(item));
    }

    cond_var.notify_one();
  }

  T& front() {
    std::unique_lock lock(mutex);
    cond_var.wait(lock, [&]{ return !queue.empty(); });
    return queue.front();
  }

  T pop() {
    std::lock_guard lock(mutex);
    T item = std::move(queue.front());
    queue.pop();
    return item;
  }

  size_t size() {
    std::lock_guard lock(mutex);
    return queue.size();
  }
};

inline void zmq_send_reliable(zmq::socket_ref zmq_socket, zmq::message_t& msg, std::optional<std::reference_wrapper<zmq::message_t>> identifier, uint32_t retry_interval_us = 10000) {
  bool success = false;
  do {
    try {
      if(identifier.has_value()) {
        zmq_socket.send(identifier.value(), zmq::send_flags::sndmore);
      }
      zmq_socket.send(msg, zmq::send_flags::none);
      success = true;
    } catch(zmq::error_t e) {
      if (e.num() != EHOSTUNREACH) {
        SPDLOG_ERROR("ZMQ error: {}", e.what());
      } else {
        usleep(retry_interval_us); // Wait 10 ms before trying to resend the message
        SPDLOG_ERROR("Failed to send ZMQ message; retrying...");
      }
    }
  } while(!success);
}

#endif // UTILS_H
