#include "dmrs.h"

dmrs::dmrs(){
   
}

  
/*This function generates the PBCH DMRS Gold sequence based on the C_init parameter, which is computed from i_ssb, n_hf and CellID.
It outputs a pbch_dmrs_length (144*2) vector containing the sequence of 1s and 0s.More info TS 38.211 7.4.1.4 */

std::vector<uint8_t> dmrs::generate_pbch_dmrs_seq(uint8_t i_ssb, uint8_t n_hf, uint16_t nid_cell){
  int c_init = 0;
  uint8_t i_ssb_ = i_ssb + (n_hf<<2);

  std::vector<uint8_t> pbch_dmrs_seq = {};
  pbch_dmrs_seq.assign(pbch_dmrs_length, 0);

  c_init = (((i_ssb_ + 1) * (static_cast<int>(floor(nid_cell/4)) + 1 ))<<11) + ((i_ssb_ + 1)<<6) + (nid_cell%4);

  pbch_dmrs_seq = pn_sequences::pseudo_random_sequence(pbch_dmrs_length, c_init);

  return pbch_dmrs_seq;
}


/*This function returns the QPSK modulated PBCH DMRS Gold sequence based on the C_init parameter, which is computed from i_ssb, n_hf and CellID.
It outputs a pbch dmrs symbol length (144) vector containing the complex QPSK values normalized, e.g. +-0.7071 +- 0.7071i.
The signal is modulated using LiquidDSP modulator*/

std::vector<std::complex<float>> dmrs::generate_pbch_dmrs_symb(uint8_t i_ssb, uint8_t n_hf, uint16_t nid_cell){

  std::vector<uint8_t> pbch_dmrs_seq = generate_pbch_dmrs_seq(i_ssb,n_hf,nid_cell);

  std::vector<std::complex<float>> pbch_dmrs_symb = {};

  pbch_dmrs_symb.assign(pbch_dmrs_length/2, 0);
  
  // create mod/demod objects
  modulation_scheme ms = LIQUID_MODEM_QPSK;

  // create the modem objects
  modem mod   = modem_create(ms); // modulator
  
  for (int n = 0 ; n < (pbch_dmrs_length>>1) ; n++){
    modem_modulate(mod, pbch_dmrs_seq.at(2*n) + 2*pbch_dmrs_seq.at(2*n+1), pbch_dmrs_symb.data()+n);
  }

  modem_destroy(mod);
  
  return pbch_dmrs_symb;
}


/*This function generates the PDCCH DMRS Gold sequence based on the C_init parameter.
C_init is computed from CellID/PDCCHScramblingID,Slot Number in the frame and OFDM symbol number within the slot/.
It outputs a pdcch_dmrs_length (value is not fixed) vector containing the sequence of 1s and 0s.
More info TS 38.211 7.4.1.3.1 
OAI reference under int nr_pdcch_dmrs_rx(PHY_VARS_NR_UE *ue, under openair1/PHY/NR_REFSIG/nr_dmrs_rx.c */

std::vector<uint8_t> dmrs::generate_pdcch_dmrs_seq(uint16_t n_id, uint8_t n_slot, uint8_t n_ofdm, uint8_t num_symbols_per_slot, uint16_t pdcch_dmrs_length){
  
  int c_init = 0;

  std::vector<uint8_t> pdcch_dmrs_seq = {};
  
  pdcch_dmrs_seq.assign(pdcch_dmrs_length, 0);

  c_init = (((num_symbols_per_slot*n_slot + n_ofdm + 1)<<17)*(2*n_id + 1) + 2*n_id)%(1LL<<32);
  
  pdcch_dmrs_seq = pn_sequences::pseudo_random_sequence(pdcch_dmrs_length, c_init);

  return pdcch_dmrs_seq;
}

/*Generates the QPSK modulated PDCCH DMRS, PDCCH_DMRS_LENGTH is in bits, not in symbols*/
std::vector<std::complex<float>> dmrs::generate_pdcch_dmrs_symb(uint16_t n_id, uint8_t n_slot, uint8_t n_ofdm, uint8_t num_symbols_per_slot, uint16_t pdcch_dmrs_length){

  std::vector<uint8_t> pdcch_dmrs_seq = generate_pdcch_dmrs_seq(n_id, n_slot, n_ofdm, num_symbols_per_slot, pdcch_dmrs_length);

  std::vector<std::complex<float>> pdcch_dmrs_symb = {};

  pdcch_dmrs_symb.assign(pdcch_dmrs_length/2, 0);
  
  // create mod/demod objects
  modulation_scheme ms = LIQUID_MODEM_QPSK;

  // create the modem objects
  modem mod   = modem_create(ms); // modulator
  
  for (int n = 0 ; n < (pdcch_dmrs_length>>1) ; n++){
    modem_modulate(mod, pdcch_dmrs_seq.at(2*n) + 2*pdcch_dmrs_seq.at(2*n+1), pdcch_dmrs_symb.data()+n);
  }

  modem_destroy(mod);

  return pdcch_dmrs_symb;
}
