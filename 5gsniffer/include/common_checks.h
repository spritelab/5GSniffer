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