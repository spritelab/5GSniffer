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

#ifndef SSS_H
#define SSS_H

#include <iostream>
#include <array>
#include <liquid/liquid.h>
#include <complex>
#include <algorithm>
#include <utility>
#include <memory>
#include "common_checks.h"

class sss
{
public:
/*SSS/SSB Constants*/

/*Constructor/Destructor*/
  sss();
  sss(uint16_t nid_1_, uint8_t nid_2_);
  ~sss() = default; 	

/*Setters*/
  int set_nid_1(uint16_t nid_1_);
  int set_nid_2(uint8_t nid_2_);
  void set_sss_seq_f(std::array<std::complex<float>,sss_length> sss_seq_f);
  void set_sss_seq_t(std::array<std::complex<float>,ssb_nfft> sss_seq_t);

/*Getters*/
  uint16_t get_nid_1();
  uint8_t get_nid_2();
  std::array<std::complex<float>,sss_length> get_sss_seq_f();
  std::array<std::complex<float>,ssb_nfft> get_sss_seq_t();

/*Functions to generate PSS in frequency and time by performing IFFT */
  std::array <std::complex<float>,sss_length>  generate_sss_seq(uint16_t nid_1, uint8_t nid_2);
  std::array <std::complex<float>,ssb_nfft>  convert_sss_t(std::array <std::complex<float>,sss_length> sss_seq_f);
  std::array <std::array<std::complex<float>,sss_length>,nid_max+1> generate_all_sss_seq();
  void init_all_seq();

private:
  uint16_t nid_1;
  uint8_t nid_2;
  std::array <std::complex<float>,sss_length>  sss_seq_f;
  std::array <std::complex<float>,ssb_nfft>  sss_seq_t;
};

#endif