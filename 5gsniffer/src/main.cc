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

#include <cstdint>
#include <iostream>
#include <cstdlib>
#include <unistd.h>

#include "spdlog/spdlog.h"
#include "spdlog/cfg/env.h"
#include "file_sink.h"
#include "sniffer.h"
#include "exceptions.h"
#include "config.h"

using namespace std;
extern struct config config;

static void usage() {
  cout << "Usage: 5g_sniffer <path_to_config.toml>" << endl;
}

/** 
 * Main function of the 5G sniffer.
 *
 * @param argc 
 * @param argv 
 */
int main(int argc, char** argv) {
  string config_path;

  #ifdef DEBUG_BUILD
    SPDLOG_INFO("=== This is a debug mode build ===");
  #endif

  // Load spdlog level from environment variable
  // For example: export SPDLOG_LEVEL=debug
  spdlog::cfg::load_env_levels();

  // Set logger pattern
  spdlog::set_pattern("[%^%l%$] [%H:%M:%S.%f thread %t] [%s:%#] %v");

  // Get config path from command-line args
  if (argc == 1) {
    config_path = string("config.toml");
  } else if (argc == 2) {
    config_path = string(argv[1]);
  } else {
    usage();
    exit(1);
  }

  try {
    // Load the config
    config = config::load(config_path);

    // Create sniffer
    if(config.file_path.compare("") == 0) {
      sniffer sniffer(config.sample_rate, config.frequency);
      sniffer.start();  
    } else {
      sniffer sniffer(config.sample_rate, config.file_path.data());
      sniffer.start();
    }
  } catch (sniffer_exception& e) {
    SPDLOG_ERROR(e.what());
    return 1;
  } catch (const toml::parse_error& err) {
    std::cerr << "Error parsing sniffer configuration file '" << *err.source().path << "':\n" << err.description() << "\n  (" << err.source().begin << ")\n";
    return 1;
  }

  return 0;
}
