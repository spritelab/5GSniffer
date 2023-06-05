#include "dci.h"

/* Default Constructor */
dci::dci(){
  found_possible_dci = false;         
  found_aggregation_level = 0;
  found_candidate = 0;
  max_num_candidate = 0;
  coreset_id = 0 ;     
  coreset_start_rb = 0; 
  std::string rnti_type = {};
  std::string dci_format = {};
  uint16_t rnti = 0;
  std::vector<uint8_t> payload = {};
  uint16_t nof_bits = 0;
  uint16_t pdcch_scrambling_id = 0;
  uint8_t n_slot = 0;
  uint8_t n_ofdm = 0;
  float correlation = 0;
}

/* Constructor that will already set the array sequences. question, will the pss_seq_f be already set with the new values? */
dci::dci(bool found_possible_dci_, uint8_t found_aggregation_level_, uint8_t found_candidate_, uint8_t max_num_candidate_, uint8_t coreset_id_,
 uint8_t coreset_start_rb_, std::string rnti_type_, std::string dci_format_, uint16_t rnti_, std::vector<uint8_t> payload_, uint16_t nof_bits_, 
 uint16_t pdcch_scrambling_id_, uint8_t n_slot_, uint8_t n_ofdm_, float correlation_){
  set_found_possible_dci(found_possible_dci_);
  set_found_aggregation_level(found_aggregation_level_);
  set_found_candidate(found_candidate_);
  set_max_num_candidate(max_num_candidate_);
  set_coreset_id(coreset_id_);
  set_coreset_start_rb(coreset_start_rb_);
  set_rnti_type(rnti_type_); 
  set_dci_format(dci_format_);
  set_rnti(rnti_);
  set_payload(payload_);
  set_nof_bits(nof_bits_);
  set_pdcch_scrambling_id(pdcch_scrambling_id_);
  set_n_slot(n_slot_);
  set_n_ofdm(n_ofdm_);
  set_correlation(correlation_);
   
}

/*Getters and Setters*/
void dci::set_found_possible_dci(bool found_possible_dci_){
  found_possible_dci = found_possible_dci_;
}

void dci::set_found_aggregation_level(uint8_t found_aggregation_level_){
  found_aggregation_level = found_aggregation_level_;
}

void dci::set_found_candidate(uint8_t found_candidate_){
  found_candidate = found_candidate_;
}

void dci::set_max_num_candidate(uint8_t max_num_candidate_){
  max_num_candidate = max_num_candidate_;
}

void dci::set_coreset_id(uint8_t coreset_id_){
  coreset_id = coreset_id_;
}

void dci::set_coreset_start_rb(uint8_t coreset_start_rb_){
  coreset_start_rb = coreset_start_rb_;
}

void dci::set_rnti_type(std::string rnti_type_){
  rnti_type = rnti_type_;
}

void dci::set_dci_format(std::string dci_format_){
  dci_format = dci_format_;
}

void dci::set_rnti(uint16_t rnti_){
  rnti = rnti_;
}

void dci::set_payload(std::vector<uint8_t> payload_){
  payload = payload_;
}

void dci::set_nof_bits(uint16_t nof_bits_){
  nof_bits = nof_bits_;
}

void dci::set_pdcch_scrambling_id(uint16_t pdcch_scrambling_id_){
  pdcch_scrambling_id = pdcch_scrambling_id_;
}

void dci::set_n_slot(uint8_t n_slot_){
  n_slot = n_slot_;
}

void dci::set_n_ofdm(uint8_t n_ofdm_){
  n_ofdm = n_ofdm_;
}

void dci::set_correlation(float correlation_){
  correlation = correlation_;
}

bool dci::get_found_possible_dci(){
  return found_possible_dci;
}

uint8_t dci::get_found_aggregation_level(){
  return found_aggregation_level;
}

uint8_t dci::get_found_candidate(){ 
  return found_candidate;
}

uint8_t dci::get_max_num_candidate(){ 
  return max_num_candidate;
}

uint8_t dci::get_coreset_id(){
  return coreset_id;
}

uint8_t dci::get_coreset_start_rb(){
  return coreset_start_rb;
}

std::string dci::get_rnti_type(){
  return rnti_type;
}

std::string dci::get_dci_format(){
  return dci_format;
}

uint16_t dci::get_rnti(){
  return rnti;
}

std::vector<uint8_t> dci::get_payload(){
  return payload;
}

uint16_t dci::get_nof_bits(){
  return nof_bits;
}

uint16_t dci::get_pdcch_scrambling_id(){
  return pdcch_scrambling_id;
}

uint8_t dci::get_n_slot(){
  return n_slot;
}

uint8_t dci::get_n_ofdm(){
  return n_ofdm;
}

float dci::get_correlation(){
  return correlation;
}
