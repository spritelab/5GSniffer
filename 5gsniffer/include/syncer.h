/*
 * Copyright (c) 2021.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 */

#ifndef SYNCER_H
#define SYNCER_H

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>
#include <complex>
#include <memory>
#include "worker.h"
#include "pss.h"
#include "sss.h"
#include "phy.h"
#include "flow_pool.h"
#include <srsran/srsran.h>

using namespace std;

/**
 * A sample_worker that synchronizes to a 5G NR signal.
 */
class syncer : public worker {
  public:
    syncer(uint64_t sample_rate, shared_ptr<nr::phy> phy);
    virtual ~syncer();
    void process(shared_ptr<vector<complex<float>>>& samples, int64_t metadata) override;
  private:
    void downsample(uint64_t num_samples, int64_t start_sample, int64_t end_sample);
    void fine_sync();
    void find_pss();
    void find_sss();
    void fine_time_sync();

    std::array<uint8_t, 4> pdcch_coreset0_get(uint16_t min_chann_bw, uint32_t ssb_scs, uint32_t pdcch_scs, uint8_t coreset0_idx);

    // Callbacks
    void on_sss_found(uint16_t nid1);
    void on_sync_lost();
    void on_mib_found(srsran_mib_nr_t& mib, bool found);
    

    uint64_t sample_rate;
    shared_ptr<nr::phy> phy;
    enum class state { find_pss, fine_sync, find_sss, wait, reset, relay } state;
    vector<pss> psss;
    sss ssss;
    vector<complex<float>> processing_queue;
    vector<complex<float>> downsampled_samples;
    resamp_crcf resampler;
    float resampling_rate;
    uint32_t max_resampled_samples_per_sample;
    float cfo;
    float new_cfo_fine;
    int64_t sss_hint;
    uint64_t mib_id;
    int waiting_for_pss;
    int64_t counting_samples;
    float ssb_period;
    shared_ptr<flow_pool> flow_pool;
    uint8_t pss_start;
    uint8_t pss_end;
    int pss_window_size;
};

#endif