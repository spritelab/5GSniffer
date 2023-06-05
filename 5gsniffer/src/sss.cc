#include "sss.h"

/* Default Constructor */
sss::sss(){
  nid_1 = 0;
  nid_2 = 0;
  sss_seq_f = {};
  sss_seq_t = {};
}

/* Constructor that will already set the array sequences. question, will the sss_seq_f be already set with the new values? */
sss::sss(uint16_t nid_1_, uint8_t nid_2_){
  nid_1 = nid_1_;
  nid_2 = nid_2_;
  sss_seq_f = generate_sss_seq(nid_1,nid_2);
  sss_seq_t = convert_sss_t(sss_seq_f);
}


/* Getters */
uint16_t sss::get_nid_1(){
  return nid_1;
}

uint8_t sss::get_nid_2(){
  return nid_2;
}


std::array<std::complex<float>,sss_length> sss::get_sss_seq_f(){
  return sss_seq_f;
}	

std::array<std::complex<float>,ssb_nfft> sss::get_sss_seq_t(){
  return sss_seq_t;
}

/* Setters */

/* Function to set NID_1, question, should this update the sss_freq sequences if there is any already set? */
int sss::set_nid_1(uint16_t nid_1_){
  if(valid_nid_1(nid_1_)){
    nid_1 = nid_1_;
    return 0;
  } else {
    SPDLOG_ERROR("Invalid NID_1");
    return -1;
  }
}

int sss::set_nid_2(uint8_t nid_2_){
  if(valid_nid_2(nid_2_)){
    nid_2 = nid_2_;
    return 0;
  } else {
    SPDLOG_ERROR("Invalid NID_2");
    return -1;
  }
}


// question, should I copy this arrays? or is it better to point to the other ones? but what if the other ones are modified? would they?
void sss::set_sss_seq_f(std::array<std::complex<float>,sss_length> sss_seq_f_){
  std::copy(std::begin(sss_seq_f_), std::end(sss_seq_f_), std::begin(sss_seq_f)); 
}

void sss::set_sss_seq_t(std::array<std::complex<float>,ssb_nfft> sss_seq_t_){
  std::copy(std::begin(sss_seq_t_), std::end(sss_seq_t_), std::begin(sss_seq_t)); 
}

/* Function that generates a SSS sequence. SSS is an m-sequence generated from 2 m-sequences of length 127.
  The function has been generated as described in 3GPP TS 38.211 7.4.2.3.
  Output is a 256 complex float with the SSS sequence of array +1 -1 and padded
  It should be padded with 64 0's at beginning and 65 at the end.*/

// question should we generate all of them? Also I guess calling this funciton for all nid_1 and nid_2 is way worse?
std::array <std::complex<float>,sss_length> sss::generate_sss_seq(uint16_t nid_1, uint8_t nid_2){
  const uint8_t x_initial_size = 7;
  std::array<int,x_initial_size> x_initial = {1, 0, 0, 0, 0, 0, 0};

  std::array <std::complex<float>,sss_length> sss_seq_f = {};
  std::array <int,sss_length> aux_array_0 = {};
  std::array <int,sss_length> aux_array_1 = {};

  if(!valid_nid_1(nid_1) | !valid_nid_1(nid_1)){
    SPDLOG_ERROR("Invalid NID");
    return sss_seq_f;
  }

  std::copy_n(x_initial.begin(), x_initial_size, aux_array_0.begin());
  std::copy_n(x_initial.begin(), x_initial_size, aux_array_1.begin());

  for (int i=0; i < (sss_length - x_initial_size); i++) {
    aux_array_0.at(i+x_initial_size) = (aux_array_0.at(i + 4) + aux_array_0.at(i))%2;
    aux_array_1.at(i+x_initial_size) = (aux_array_1.at(i+1) + aux_array_1.at(i))%2;
  }

  uint16_t m_0_idx = 15*(nid_1/112) + (5*nid_2);
  uint16_t m_1_idx = nid_1%112;

/*	We put the SSS values directly in the position in the grid, after 64 padding 0's*/
  for (int i = 0; i < sss_length; i++){
    sss_seq_f.at(i) = (1 - 2*aux_array_0.at((i + m_0_idx)%(sss_length))) * (1 - 2*aux_array_1.at((i + m_1_idx)%(sss_length))); 
  }

  return sss_seq_f;
}


/* Function that generates all 1008 SSS sequences. SSS is an m-sequence generated from 2 m-sequences of length 127.
  The function has been generated as described in 3GPP TS 38.211 7.4.2.3.
  Output is a matrix of 1008 x 256 complex float with the SSS sequence of array +1 -1 and padded
  Each SSS sequence in frequency is be padded with 64 0's at beginning and 65 at the end.*/
std::array <std::array<std::complex<float>,sss_length>,nid_max+1> sss::generate_all_sss_seq(){
   std::array <std::array<std::complex<float>,sss_length>,nid_max+1> sss_seq_all;

  for (int nid_2 = 0; nid_2 < nid_2_max+1; nid_2++){
    for (int nid_1 = 0; nid_1 < nid_1_max+1; nid_1++){
      sss_seq_all.at(3*nid_1 + nid_2) = generate_sss_seq(nid_1,nid_2);
    }
  }
  
  return sss_seq_all;
}

/* Function that converts an m-sequence SSS in frequency (length 127) to 
  time domain. The input SSS in freq. is already padded to 256 fft size.
  It performs IFFT using liquidDSP library. 
  FFT_shift is performed
  Output is a N_IFFT float array of values. 
  */
std::array <std::complex<float>,ssb_nfft>  sss::convert_sss_t(std::array <std::complex<float>,sss_length>  sss_seq_f) {
  std::array <std::complex<float>,ssb_nfft> sss_seq_t = {};

  // //Padding with 0's to put the SSS in the correct subcarriers
  std::array <std::complex<float>,ssb_nfft> sss_seq_f_padded = {};

  for (int i= 0 ; i< sss_length ; i++){
    sss_seq_f_padded.at(i+64) = sss_seq_f.at(i);
  }

  /*Perform ifft shift before the transform*/
  std::rotate(sss_seq_f_padded.begin(), sss_seq_f_padded.begin() + 128, sss_seq_f_padded.end());
  
  auto p_time = reinterpret_cast<liquid_float_complex*>(sss_seq_t.data());
  auto p_freq = reinterpret_cast<liquid_float_complex*>(sss_seq_f_padded.data());
  
  /*Create liquid FFT plan*/
  int flags = 0;
  fftplan q = fft_create_plan(sss_seq_f_padded.size(), p_freq, p_time, LIQUID_FFT_BACKWARD, flags);

  /*execute IFFT*/ 
  fft_execute(q);

  /*destroy FFT plan*/
  fft_destroy_plan(q);

  return sss_seq_t;
}



