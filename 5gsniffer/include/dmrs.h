#ifndef DMRS_H
#define DMRS_H

#include "pn_sequences.h"
#include <liquid/liquid.h>

class dmrs
{
  public:

    /*Constructor/Destructor*/
    dmrs();
    ~dmrs() = default;
		
		std::vector<uint8_t> generate_pbch_dmrs_seq(uint8_t i_ssb, uint8_t n_hf, uint16_t nid_cell);
    std::vector<std::complex<float>> generate_pbch_dmrs_symb(uint8_t i_ssb, uint8_t n_hf, uint16_t nid_cell);

    std::vector<uint8_t> generate_pdcch_dmrs_seq(uint16_t n_id, uint8_t n_slot, uint8_t n_ofdm, uint8_t num_symbols_per_slot, uint16_t pdcch_dmrs_length);
		std::vector<std::complex<float>> generate_pdcch_dmrs_symb(uint16_t n_id, uint8_t n_slot, uint8_t n_ofdm, uint8_t num_symbols_per_slot, uint16_t pdcch_dmrs_length);
    
	private:
};

#endif