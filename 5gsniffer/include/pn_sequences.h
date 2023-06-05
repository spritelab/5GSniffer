#ifndef PN_SEQUENCES_H
#define PN_SEQUENCES_H

#include <iostream>
#include <math.h>
#include <array>
#include <vector>
#include <complex>
#include "common_checks.h"

class pn_sequences
{
  public:

    /*Constructor/Destructor*/
    pn_sequences();
    ~pn_sequences() = default;     

/*    The PN_sequence is used to generate the DMRS signals but also also for scrambling de-scrambling of PDSCH/PDCCH*/
    static std::vector<uint8_t> pseudo_random_sequence(uint16_t seq_length, int c_init);
  // static std::array<int,size> pseudo_random_sequence_optimised(unsigned int size, int c_init)
    
  private:
    int c_init;
};

#endif