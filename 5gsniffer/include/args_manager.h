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

#ifndef ARGS_MANAGER_H
#define ARGS_MANAGER_H

#include <stdint.h>
#include <string>

struct args_t {
  int force_N_id_2;
  std::string input_file_name = "";
  std::string rf_args;
  std::string rf_dev;
  double rf_freq;
  double rf_gain;


  };

class args_manager {
public:
  static void default_args(args_t& args);
  static void usage(args_t& args, const std::string& prog);
  static void parse_args(args_t& args, int argc, char **argv);
private:
  args_manager() = delete;
};

#endif