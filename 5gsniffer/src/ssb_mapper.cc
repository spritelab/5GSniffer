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

#include <complex>
#include <cstddef>
#include <cstdint>
#include <liquid/liquid.h>
#include "ssb_mapper.h"
#include "spdlog/spdlog.h"
#include "sss.h"
#include "dsp.h"
#include "utils.h"
#include "pbch.h"

using namespace std;

/** 
 * Constructor for ssb_mapper.
 */
ssb_mapper::ssb_mapper(shared_ptr<nr::phy> phy) :
  pbch(phy) {
  this->phy = phy;
}

/** 
 * Destructor for ssb_mapper.
 */
ssb_mapper::~ssb_mapper() {

}

/** 
 * Processes input OFDM symbols, looking for SSS and sending symbols to PBCH.
 * @param symbols contains OFDM symbols
 */
void ssb_mapper::process(shared_ptr<vector<symbol>>& symbols, int64_t metadata) {
  SPDLOG_DEBUG("Got {} symbols", symbols->size());

  auto ssss = phy->ssss;

  if(symbols->size() >= 4) {
    if (phy->in_synch){
      // No need to re-find SSS. fine time synch with SSS is performed in syncer.cc.
      auto pbch_symbols = make_shared<vector<symbol>>(symbols->begin(), symbols->begin()+4);
      pbch.initialize_dmrs_seq();
      pbch.work(pbch_symbols);
    } else {
      assert(symbols->at(2).symbol_index == 4); // Assert SSS is the 5th symbol (index 4)

      // Extract SSS
      span<complex<float>> sss_res = symbols->at(2).get_res(56, 182);
      assert(sss_res.size() == sss_length);
      
      // Find SSS through correlation
      float max_corr = 0.0f;
      uint16_t max_nid = 0;
      float avg_corr = 0.0f;
      for(uint16_t nid1 = 0; nid1 <= 335; nid1++) {
        auto sss_ref = ssss.at(nid1*3 + phy->nid2);
        vector<float> out(1);

        correlate_magnitude(out, sss_ref, sss_res);

        float corr = out.at(0);
        avg_corr += corr;
        if(corr > max_corr) {
          max_corr = corr;
          max_nid = nid1;
        }
      }
      avg_corr /= 355;

      SPDLOG_DEBUG("SSS max corr: {} ({} avg)", max_corr, avg_corr);

      if (max_corr > pss_sss_times_avg_threshold * avg_corr) {
        SPDLOG_DEBUG("NID1: {}, corr {} avg {} ratio {}", max_nid, max_corr, avg_corr, max_corr / avg_corr);
        this->on_sss_found(max_nid);

        // PBCH processing TODO can happen in parallel
        auto pbch_symbols = make_shared<vector<symbol>>(symbols->begin(), symbols->begin()+4);
        pbch.initialize_dmrs_seq();
        pbch.work(pbch_symbols);
      } else {
        this->on_sss_not_found();
      }
    }
  } else {
    SPDLOG_ERROR("Need at least 4 symbols to be able to process SSB.");
  }
}
