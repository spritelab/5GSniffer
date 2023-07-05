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

#include "pss.h"

/* Default Constructor */
pss::pss(){
  nid_2 = 0;
  pss_seq_f = {};
  pss_seq_t = {};
  pss_seq_f_matrix = {};
  pss_seq_t_matrix = {};
}

/* Constructor that will already set the array sequences. question, will the pss_seq_f be already set with the new values? */
pss::pss(uint8_t nid_2_){
  nid_2 = nid_2_;
  pss_seq_f = generate_pss_seq(nid_2);
  pss_seq_t = convert_pss_t(pss_seq_f);
  pss_seq_f_matrix = {};
  pss_seq_t_matrix = {};
}

/* Getters */
uint8_t pss::get_nid_2(){
  return nid_2;
}

std::array<std::complex<float>,pss::pss_length> pss::get_pss_seq_f(){
  return pss_seq_f;
}	

std::array<std::complex<float>,ssb_nfft> pss::get_pss_seq_t(){
  return pss_seq_t;
}

std::array <std::array<std::complex<float>,pss::pss_length>,nid_2_max + 1> pss::get_pss_seq_f_matrix(){
  return pss_seq_f_matrix;
}

std::array <std::array<std::complex<float>,ssb_nfft>,nid_2_max + 1> pss::get_pss_seq_t_matrix(){
  return pss_seq_t_matrix;
}

/* Setters */

/* Function to set NID_2, question, should this update the pss_freq sequences if there is any already set? TODO throw exception or something*/
int pss::set_nid_2(uint8_t nid_2_){
  if(valid_nid_2(nid_2_)){
    nid_2 = nid_2_;
    return 0;
  } else {
    SPDLOG_ERROR("Invalid NID_2");
    return -1;
  }
}

// question, should I copy this arrays? or is it better to point to the other ones? but what if the other ones are modified? would they?
void pss::set_pss_seq_f(std::array<std::complex<float>,pss::pss_length> pss_seq_f_){
  std::copy(std::begin(pss_seq_f_), std::end(pss_seq_f_), std::begin(pss_seq_f)); 
}

void pss::set_pss_seq_t(std::array<std::complex<float>,ssb_nfft> pss_seq_t_){
  std::copy(std::begin(pss_seq_t_), std::end(pss_seq_t_), std::begin(pss_seq_t)); 
}

/* Function that generates a PSS sequence. PSS is an m-sequence of length 127.
  The function has been generated as described in 38.211. 
  Output is a 256 complex float with the PSS sequence of array +1 -1 and padded
  It should be padded with 64 0's at beginning and 65 at the end.*/
std::array <std::complex<float>,pss::pss_length> pss::generate_pss_seq(uint8_t nid_2){
  const uint8_t x_initial_size = 7;
  std::array<uint8_t,x_initial_size> x_initial = {0, 1, 1 , 0, 1, 1, 1};
  
  std::array <std::complex<float>,pss::pss_length> pss_seq_f = {};
  std::array <uint8_t,pss::pss_length> aux_array = {};

  uint8_t m_idx = 0;

  // todo Throw some error	
  if(!valid_nid_2(nid_2)){
    SPDLOG_ERROR("Invalid NID_2");
    return pss_seq_f;
  }

  for (int i = 0; i < x_initial_size ; i ++){
    aux_array.at(i) = x_initial.at(i);
  }

  for (int i = 0; i < pss::pss_length - x_initial_size; i++){
    aux_array.at(i + x_initial_size) = (aux_array.at(i + 4) + aux_array.at(i))%2;
  } 

  /*Compute the PSS sequence values and place them in the 256 subcarriers.
  We put the PSS values directly in the position in the grid, after 64 padding 0's*/
  for (int i = 0; i< pss::pss_length ; i++){
    m_idx = (i + 43 * nid_2)%(pss::pss_length);
    pss_seq_f.at(i) = 1 - 2 * aux_array.at(m_idx);
  }
  return pss_seq_f;
}

void pss::init_all_seq() {
  for (int i=0; i< nid_2_max + 1; i++){
    pss_seq_f_matrix.at(i) = generate_pss_seq(i);
    pss_seq_t_matrix.at(i) = convert_pss_t(pss_seq_f_matrix.at(i));
  }  	
}

/* Function that converts an m-sequence PSS in frequency (length 127) to 
  time domain. The input PSS in freq. is already padded to 256 fft size.
  It performs IFFT using liquidDSP library. 
  FFT_shift is performed
  Output is a N_IFFT float array of values. 
  */
std::array <std::complex<float>,ssb_nfft>  pss::convert_pss_t(std::array <std::complex<float>,pss::pss_length>  pss_seq_f){
  std::array <std::complex<float>,ssb_nfft> pss_seq_t = {};
 
  //Padding with 0's to put the PSS in the correct subcarriers
  std::array <std::complex<float>,ssb_nfft> pss_seq_f_padded = {};
  for (int i= 0 ; i< pss::pss_length ; i++){
    pss_seq_f_padded.at(i+64) = pss_seq_f.at(i);
  }  

  /*Perform ifft shift before the transform*/
  std::rotate(pss_seq_f_padded.begin(), pss_seq_f_padded.begin() + 128, pss_seq_f_padded.end());
  
  auto p_time = reinterpret_cast<liquid_float_complex*>(pss_seq_t.data());
  auto p_freq = reinterpret_cast<liquid_float_complex*>(pss_seq_f_padded.data());
  
  /*Create liquid FFT plan*/
  int flags = 0;
  fftplan q = fft_create_plan(pss_seq_f_padded.size(), p_freq, p_time, LIQUID_FFT_BACKWARD, flags);

  /*execute IFFT*/ 
  fft_execute(q);

  /*destroy FFT plan*/
  fft_destroy_plan(q);

  return pss_seq_t;
}



