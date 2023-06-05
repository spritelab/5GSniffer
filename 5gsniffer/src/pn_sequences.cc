#include "pn_sequences.h"

pn_sequences::pn_sequences(){
  c_init = 0;
}

std::vector<uint8_t> pn_sequences::pseudo_random_sequence(uint16_t seq_length, int c_init){

  uint16_t size_x = seq_length + gold_sequence_length + Nc;
  
  std::vector<uint8_t> x1 = {};
  std::vector<uint8_t> x2 = {};
  std::vector<uint8_t> c = {};

  x1.assign(size_x, 0);
  x2.assign(size_x, 0);
  c.assign(seq_length, 0);
  
  x1.at(0) = 1;  /* init first m sequence */  

  for (int n = 0; n < gold_sequence_length; n++) {
    x2.at(n) = (c_init >> n) & 0x1;
  }

  for (int n = 0; n < (Nc + seq_length); n++) {
    x1.at(n+31) = (x1.at(n+3) + x1.at(n))%2;
    x2.at(n+31) = (x2.at(n+3) + x2.at(n+2) + x2.at(n+1) + x2.at(n))%2;
  }     

  for (int n = 0; n < seq_length; n++) {
    c.at(n) = (x1.at(n+Nc) + x2.at(n+Nc))%2;
  }
  
  return c;

}