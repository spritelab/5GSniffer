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

#ifndef COMMON_CHECKS_H
#define COMMON_CHECKS_H

#include <iostream>
#include <algorithm>
#include <string>
#include "spdlog/spdlog.h"
#include "phy_params_common.h"


bool valid_nid_2(uint8_t n_id_2);

bool valid_nid_1(uint16_t n_id_1);

bool valid_scs_ssb(uint8_t scs_ssb);

bool isvalid_coreset_duration(uint8_t duration);

bool isvalid_coreset_cce_reg_mapping_type(std::string cce_reg_mapping_type);

bool isvalid_coreset_reg_bundlesize(uint8_t reg_bundlesize, std::string cce_reg_mapping_type, uint8_t duration);

bool isvalid_interleaver_size(uint8_t interleaver_size);

bool isvalid_shift_index(uint16_t shift_index);

bool isvalid_pdcch_DMRS_scrambling_id(uint16_t pdcch_DMRS_scrambling_id);

bool isvalid_RNTI(uint16_t RNTI);

#endif