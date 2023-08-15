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

#ifndef PHY_H
#define PHY_H

#include <cstdint>
#include <vector>
#include <memory>
#include "bandwidth_part.h"
#include "channel_mapper.h"

using namespace std;

/**
 * Class for holding 5G NR physical-layer properties.
 */
namespace nr {
  class phy {
    public:
      uint8_t nid1;
      uint16_t nid2;
      uint8_t i_ssb;
      uint8_t n_hf;
      bool in_synch;
      shared_ptr<bandwidth_part> ssb_bwp;                 ///< Special bandwidth part used for the SSB only
      vector<shared_ptr<bandwidth_part>> bandwidth_parts;
      vector<shared_ptr<channel_mapper>> channel_mappers;
      std::array<std::array<std::complex<float>,sss_length>,nid_max+1> ssss;

      phy();
      uint16_t get_cell_id() const;
      shared_ptr<bandwidth_part> get_initial_dl_bandwidth_part();
  };
}

#endif // PHY_H