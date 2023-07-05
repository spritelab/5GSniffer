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

#ifndef PN_SEQUENCES_H
#define PN_SEQUENCES_H

#include <iostream>
#include <math.h>
#include <array>
#include <vector>
#include <complex>
#include "common_checks.h"

class pn_sequences
{
  public:

    /*Constructor/Destructor*/
    pn_sequences();
    ~pn_sequences() = default;     

/*    The PN_sequence is used to generate the DMRS signals but also also for scrambling de-scrambling of PDSCH/PDCCH*/
    static std::vector<uint8_t> pseudo_random_sequence(uint16_t seq_length, int c_init);
  // static std::array<int,size> pseudo_random_sequence_optimised(unsigned int size, int c_init)
    
  private:
    int c_init;
};

#endif