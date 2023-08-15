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

// #include <iostream>
// #include <cstdlib>
// #include <unistd.h>
// #include "args_manager.h"

// // #ifdef __cplusplus
// // extern "C" {
// // #undef I // Fix complex.h #define I nastiness when using C++
// #include "srsran/srsran.h"
// #include "srsran/phy/rf/rf_utils.h"
// #include "srsran/phy/rf/rf.h"
// // }

// using namespace std;

// char rf_dev[]  = "";
// char rf_args[]  = "";

// int main(int argc, char** argv) {

//   args_t args;
//   args_manager::parse_args(args, argc, argv);

//   int                           n;
//   srsran_rf_t                   rf;
//   srsran_ue_cellsearch_t        cs;
//   srsran_ue_cellsearch_result_t found_cells[3];
//   int                           nof_freqs;
//   // srsran_earfcn_t               channels[MAX_EARFCN];
//   uint32_t                      freq;
//   uint32_t                      n_found_cells = 0;

//   printf("Opening RF device...\n");

//   if (srsran_rf_open_devname(&rf, rf_dev, rf_args, 1)) {
//     // ERROR("Error opening rf");
//     cout << "error opening rf" << endl;
//     exit(-1);
//   }
//   cout << "opened" << endl;

//   // DEBUG(" ----  Receive %d samples  ---- ", nsamples);
//   // return srsran_rf_recv_with_time((srsran_rf_t*)h, data, nsamples, 1, NULL, NULL);
//   srsran_rf_close(&rf);
// }
