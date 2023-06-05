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

#include "args_manager.h"

#include <iostream>
#include <unistd.h>
#include <cstdio>

using namespace std;

void args_manager::default_args(args_t& args) {
  args.force_N_id_2 = -1;
  args.input_file_name = "";
  args.rf_args = "";
  args.rf_dev = "";
  args.rf_freq = -1.0;
  args.rf_gain = -1.0;
}

void args_manager::usage(args_t& args, const std::string& prog) {
  printf("Usage: %s [afgil] -f rx_frequency (in Hz) | -i input_file\n", prog.c_str());
  printf("\t-h show this help message\n");
  printf("\t-a RF args [Default %s]\n", args.rf_args.c_str());
  printf("\t-f Set RX freq [Default %.1f Hz]\n", args.rf_freq);
  printf("\t-g Set RX gain [Default %.1f dB]\n", args.rf_gain);
  printf("\t-d RF devicename [Default %s]\n", args.rf_dev.c_str());
  printf("\t-i input_file [Default use RF board]\n");
  printf("\t-l Force N_id_2 [Default find best]\n");
}

void args_manager::parse_args(args_t& args, int argc, char **argv) {
  int opt;
  default_args(args);
  while ((opt = getopt(argc, argv, "afgil")) != -1) {
    switch (opt) {
      case 'a':
        args.rf_args = argv[optind];
        break;
      case 'i':
        args.input_file_name = argv[optind];
        break;
      case 'l':
        args.force_N_id_2 = atoi(argv[optind]);
        break;
      case 'f':
        args.rf_freq = strtod(argv[optind], nullptr);
        break;
      case 'g':
        args.rf_gain = strtod(argv[optind], nullptr);
        break;
      case 'h':
      default:
        usage(args, argv[0]);
        exit(-1);
    }
  }

  if (args.rf_freq < 0 && args.input_file_name == "") {
    usage(args, argv[0]);
    exit(-1);
  }
}
