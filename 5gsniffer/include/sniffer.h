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

#ifndef SNIFFER_H
#define SNIFFER_H

#include "worker.h"
#include "syncer.h"
#include "phy.h"

using namespace std;

/**
 * 5G sniffer main class
 */
class sniffer {
  public:
    sniffer(uint64_t sample_rate, uint64_t frequency); ///< Create a sniffer for an SDR source.
    sniffer(uint64_t sample_rate, string path); ////< Create a sniffer for a file source.
    virtual ~sniffer();
    void start();
    void stop();

    uint64_t sample_rate;
    unique_ptr<worker> device;
  private:
    void init();
    bool running;
};

#endif // SNIFFER_H