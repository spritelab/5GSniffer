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

#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <span>
#include <complex>
#include <fstream>
#include <iostream>
#include <chrono>
#include <spdlog/spdlog.h>

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

#endif // UTILS_H