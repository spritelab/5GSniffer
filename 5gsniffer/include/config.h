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

#ifndef CONFIG_H
#define CONFIG_H

#include "toml.hpp"
#include <cstdint>
#include <iostream>
#include <spdlog/spdlog.h>
#include "exceptions.h"

using namespace std;

typedef struct pdcch_config {
  bool si_dci_only;
  bool use_config_from_mib;
  uint8_t coreset_id;
  int32_t subcarrier_offset;
  uint8_t numerology;
  uint16_t num_prbs;
  bool extended_prefix;
  uint16_t rnti_start;
  uint16_t rnti_end;
  uint16_t max_rnti_queue_size;
  uint16_t scrambling_id_start;
  uint16_t scrambling_id_end;
  std::vector<uint8_t> dci_sizes_list;
  std::vector<float> AL_corr_thresholds;    
  std::vector<uint8_t> num_candidates_per_AL;

  // uint8_t dci_size_end;
  bool sc_power_decision;
  std::string coreset_interleaving_pattern;
  uint8_t coreset_duration;
  uint8_t coreset_ofdm_symbol_start;
  uint16_t coreset_nshift;
  uint8_t coreset_reg_bundle_size;
  uint8_t coreset_interleaver_size;
  int64_t sample_rate_time;
  int rnti_list_length;
} pdcch_config;

struct config {
  string file_path;
  uint64_t sample_rate;
  double frequency;
  uint8_t nid_2;
  string rf_args;
  uint16_t ssb_numerology;

  vector<pdcch_config> pdcch_configs;

  static struct config load(string config_path) {
    struct config conf;
    double default_frequency = 627750000;
    uint64_t default_sample_rate = 23040000;
    // Parse configuration file
    SPDLOG_INFO("Loading configuration file {}", config_path);
    toml::table toml = toml::parse_file(config_path);
    conf.file_path = toml["sniffer"]["file_path"].value_or(""sv).data();
    conf.sample_rate = toml["sniffer"]["sample_rate"].value_or(default_sample_rate);
    conf.frequency = toml["sniffer"]["frequency"].value_or(default_frequency);
    conf.nid_2 = toml["sniffer"]["nid_2"].value_or(4);
    conf.rf_args = toml["sniffer"]["rf_args"].value_or(""sv).data();
    conf.ssb_numerology = toml["sniffer"]["ssb_numerology"].value_or(0);
    if(!toml["pdcch"].is_array_of_tables())
      throw config_exception("PDCCH TOML config should be an array of tables, e.g. [[pdcch]]");
    
    toml::array* pdcch_tables = toml["pdcch"].as<toml::array>();
    SPDLOG_DEBUG("Config contains {} PDCCHs", pdcch_tables->size());
    for(toml::node& node : *pdcch_tables) {
      if (node.is_table()) {
        toml::table pdcch_table = *node.as_table();
        pdcch_config pdcch_cfg;

        pdcch_cfg.si_dci_only = pdcch_table["si_dci_only"].value_or(false);
        pdcch_cfg.use_config_from_mib = pdcch_table["use_config_from_mib"].value_or(false);
        pdcch_cfg.coreset_id = pdcch_table["coreset_id"].value_or(0);
        pdcch_cfg.subcarrier_offset = pdcch_table["subcarrier_offset"].value_or(0);
        pdcch_cfg.numerology = pdcch_table["numerology"].value_or(0);
        pdcch_cfg.num_prbs = pdcch_table["num_prbs"].value_or(52);
        pdcch_cfg.extended_prefix = pdcch_table["extended_prefix"].value_or(false);
        pdcch_cfg.rnti_start = pdcch_table["rnti_start"].value_or(0);
        pdcch_cfg.rnti_end = pdcch_table["rnti_end"].value_or(0xffff);
        pdcch_cfg.scrambling_id_start = pdcch_table["scrambling_id_start"].value_or(0);
        pdcch_cfg.scrambling_id_end = pdcch_table["scrambling_id_end"].value_or(0xffff);
        pdcch_cfg.max_rnti_queue_size = pdcch_table["max_rnti_queue_size"].value_or(0xffff);
        pdcch_cfg.sample_rate_time = conf.sample_rate;
        pdcch_cfg.rnti_list_length = pdcch_table["rnti_list_length"].value_or(0xffff);
        toml::array* dci_array = pdcch_table["dci_sizes_list"].as<toml::array>();
        // Parse the DCI array list and if is not included, add 39 by default (e.g. System Information)
        if(dci_array){
          for (auto&& elem : *dci_array)
          {
            pdcch_cfg.dci_sizes_list.push_back(elem.value_or(39));
          }
        }else{
          pdcch_cfg.dci_sizes_list.push_back(39);
        }

        toml::array* AL_thresholds_array = pdcch_table["AL_corr_thresholds"].as<toml::array>();        
        if(AL_thresholds_array){
          SPDLOG_DEBUG("AL corr in the toml");
          for (auto&& elem : *AL_thresholds_array)
          {
            SPDLOG_DEBUG("AL corr in the toml element {}", elem.value_or(1.0));
            pdcch_cfg.AL_corr_thresholds.push_back(elem.value_or(1.0));
          }
        }else{
          SPDLOG_DEBUG("AL corr not in the toml");
          pdcch_cfg.AL_corr_thresholds = {0.9, 0.8, 0.7, 0.2, 0.2};
        }

        toml::array* num_candidates_per_AL_array = pdcch_table["num_candidates_per_AL"].as<toml::array>();
        if(num_candidates_per_AL_array){
          SPDLOG_DEBUG("num_candidates_per_AL in the toml, size is {}", num_candidates_per_AL_array->size());
          for (auto&& elem : *num_candidates_per_AL_array)
          {
            SPDLOG_DEBUG("num_candidates_per_AL in the toml element {}", elem.value_or(1));
            pdcch_cfg.num_candidates_per_AL.push_back(elem.value_or(1));
          }
        }else{
          SPDLOG_DEBUG("num_candidates_per_AL not in the toml");
          pdcch_cfg.num_candidates_per_AL = {8, 4, 2, 1, 0};
        }

        pdcch_cfg.sc_power_decision = pdcch_table["sc_power_decision"].value_or(false);
        pdcch_cfg.coreset_interleaving_pattern = pdcch_table["coreset_interleaving_pattern"].value_or("non-interleaved");
        pdcch_cfg.coreset_duration = pdcch_table["coreset_duration"].value_or(1);
        pdcch_cfg.coreset_ofdm_symbol_start = pdcch_table["coreset_ofdm_symbol_start"].value_or(0);
        pdcch_cfg.coreset_nshift = pdcch_table["coreset_nshift"].value_or(0);
        pdcch_cfg.coreset_reg_bundle_size = pdcch_table["coreset_reg_bundle_size"].value_or(6);
        pdcch_cfg.coreset_interleaver_size = pdcch_table["coreset_interleaver_size"].value_or(2);
        conf.pdcch_configs.push_back(pdcch_cfg);
      } else {
        SPDLOG_ERROR("Unexpected config file format");
        exit(1);
      }
    }

    return conf;
  }
};

#endif // CONFIG_H