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

#ifndef CHANNEL_MAPPER_H
#define CHANNEL_MAPPER_H

#include <cstdint>
#include <vector>
#include <memory>
#include "symbol.h"
#include "worker.h"
#include "pdcch.h"
#include "config.h"

namespace nr {
  class phy;
}

using namespace std;

/**
 * Mapper that given a list of symbols can determine the channel to which these
 * symbols belong.
 */
class channel_mapper : public worker {
  public:
    channel_mapper(uint16_t cell_id, uint32_t slots_per_frame, pdcch_config pdcch_config);
    virtual ~channel_mapper();
    void process(shared_ptr<vector<symbol>>& symbols, int64_t metadata) override;

    pdcch_config pdcch_cfg;
    
    // Sublayers
    pdcch pdcch;
};

#endif // CHANNEL_MAPPER_H