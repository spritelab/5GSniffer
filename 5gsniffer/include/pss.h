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
#ifndef PSS_H
#define PSS_H

#include <array>
#include <liquid/liquid.h>
#include <complex>
#include <utility>
#include "common_checks.h"

class pss
{
public:
/*PSS/SSB Constants*/
	static constexpr uint8_t pss_length = 127;

/*Constructor/Destructor*/
	pss();
	pss(uint8_t nid_2_);
    ~pss() = default; 	

/*Setters*/
    // question, should we do return 0, return -1, or bool? or nothing
	int set_nid_2(uint8_t nid_2_);
	void set_pss_seq_f(std::array<std::complex<float>,pss::pss_length> pss_seq_f);
	void set_pss_seq_t(std::array<std::complex<float>,ssb_nfft> pss_seq_t);

/*Getters*/
	uint8_t get_nid_2();

	std::array<std::complex<float>,pss::pss_length> get_pss_seq_f();
	std::array<std::complex<float>,ssb_nfft> get_pss_seq_t();

	std::array <std::array<std::complex<float>,pss::pss_length>,nid_2_max + 1> get_pss_seq_f_matrix();
	std::array <std::array<std::complex<float>,ssb_nfft>,nid_2_max + 1> get_pss_seq_t_matrix();

/*Functions to generate PSS in frequency and time 	*/
	std::array <std::complex<float>,pss::pss_length>  generate_pss_seq(uint8_t nid_2);
	std::array <std::complex<float>,ssb_nfft>  convert_pss_t(std::array <std::complex<float>,pss::pss_length> pss_seq_f);
	void init_all_seq();

private:
    uint8_t nid_2;

	std::array <std::complex<float>,pss::pss_length> pss_seq_f;
	std::array <std::complex<float>,ssb_nfft> pss_seq_t;
	std::array <std::array<std::complex<float>,pss::pss_length>,nid_2_max + 1>  pss_seq_f_matrix;
	std::array <std::array<std::complex<float>,ssb_nfft>,nid_2_max + 1>  pss_seq_t_matrix;

};

#endif