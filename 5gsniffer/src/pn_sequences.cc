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

#include "pn_sequences.h"

pn_sequences::pn_sequences(){
  c_init = 0;
}

std::vector<uint8_t> pn_sequences::pseudo_random_sequence(uint16_t seq_length, int c_init){

  uint16_t size_x = seq_length + gold_sequence_length + Nc;
  
  std::vector<uint8_t> x1 = {};
  std::vector<uint8_t> x2 = {};
  std::vector<uint8_t> c = {};

  x1.assign(size_x, 0);
  x2.assign(size_x, 0);
  c.assign(seq_length, 0);
  
  x1.at(0) = 1;  /* init first m sequence */  

  for (int n = 0; n < gold_sequence_length; n++) {
    x2.at(n) = (c_init >> n) & 0x1;
  }

  for (int n = 0; n < (Nc + seq_length); n++) {
    x1.at(n+31) = (x1.at(n+3) + x1.at(n))%2;
    x2.at(n+31) = (x2.at(n+3) + x2.at(n+2) + x2.at(n+1) + x2.at(n))%2;
  }     

  for (int n = 0; n < seq_length; n++) {
    c.at(n) = (x1.at(n+Nc) + x2.at(n+Nc))%2;
  }
  
  return c;

}