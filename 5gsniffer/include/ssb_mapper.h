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

#ifndef SSB_MAPPER_H
#define SSB_MAPPER_H

#include <cstdint>
#include <array>
#include <vector>
#include <memory>
#include <functional>
#include <liquid/liquid.h>
#include "symbol.h"
#include "worker.h"
#include "phy.h"
#include "pss.h"
#include "pbch.h"

using namespace std;

/**
 * Mapper for the SSB. Assumes that the SSB is in the first 4 symbols received.
 */
class ssb_mapper : public worker {
  public:
    // Callbacks
    std::function<void(uint16_t)> on_sss_found = [](uint16_t nid1) {}; ///< Default callback when SSS is found: do nothing
    std::function<void(void)> on_sss_not_found = [](){}; ///< Callback when SSS is not found

    ssb_mapper(shared_ptr<nr::phy> phy);
    virtual ~ssb_mapper();
    void process(shared_ptr<vector<symbol>>& symbols, int64_t metadata) override;

    // Sublayers
    pbch pbch;
  private:
    shared_ptr<nr::phy> phy;
};

#endif // SSB_MAPPER_H