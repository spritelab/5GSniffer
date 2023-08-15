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

#include <cmath>
#include <cstdint>
#include "exceptions.h"
#include "phy.h"
#include "spdlog/spdlog.h"
#include "phy_params_common.h"
#include <iostream>


using namespace std;

namespace nr {
  /** 
  * Constructor for phy.
  */
  phy::phy() {
    in_synch = false;
  }

  uint16_t phy::get_cell_id() const {
    return (3*nid1) + nid2;
  }

  shared_ptr<bandwidth_part> phy::get_initial_dl_bandwidth_part() {
    if(bandwidth_parts.size() > 0) {
      return bandwidth_parts.at(0);
    } else {
      throw sniffer_exception("Tried to retrieve initial DL bandwidth part before receiving MIB.");
    }
  }
}