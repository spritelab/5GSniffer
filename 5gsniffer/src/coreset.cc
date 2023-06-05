#include "coreset.h"

/*Default constructor*/

coreset::coreset(){
 control_resourceset_id = 0;
 frequency_domain_resources = 48;
 duration = 1;
 cce_reg_mapping_type = "non-interleaved";
 reg_bundlesize= 6;
 interleaver_size = 2;
 shift_index = 0;
 cell_id = 0;
 starting_ofdm_symbol_within_slot = 0;
 num_symbols_per_slot = 14;
 num_slots_per_frame = 10;
 candidates_search_space = {{8,4,2,1,0}};
}


/*Non-default constructor*/
coreset::coreset(uint8_t control_resourceset_id_,uint16_t frequency_domain_resources_,
uint8_t duration_, std::string cce_reg_mapping_type_, uint8_t reg_bundlesize_, uint8_t interleaver_size_,
uint16_t shift_index_, uint16_t cell_id_, uint8_t starting_ofdm_symbol_within_slot_, 
uint8_t num_symbols_per_slot_, uint8_t num_slots_per_frame_, std::vector<uint8_t> candidates_search_space_)
{
  set_control_resourceset_id(control_resourceset_id_);
  set_frequency_domain_resources(frequency_domain_resources_);
  set_duration(duration_);
  set_cce_reg_mapping_type(cce_reg_mapping_type_);
  set_reg_bundlesize(reg_bundlesize_);
  set_interleaver_size(interleaver_size_);
  set_shift_index(shift_index_);
  set_cell_id(cell_id_);
  set_starting_ofdm_symbol_within_slot(starting_ofdm_symbol_within_slot_);
  set_num_symbols_per_slot(num_symbols_per_slot_);
  set_num_slots_per_frame(num_slots_per_frame_);
  set_candidates_search_space(candidates_search_space_);
}

/*Setters*/

void coreset::set_control_resourceset_id(uint8_t control_resourceset_id_){
  control_resourceset_id = control_resourceset_id_;
}


void coreset::set_frequency_domain_resources(uint16_t frequency_domain_resources_){
  frequency_domain_resources = frequency_domain_resources_;
}


void coreset::set_duration(uint8_t duration_){
  if (isvalid_coreset_duration(duration_)){
    duration = duration_;
  }
  else{
    duration = 1; 
    SPDLOG_ERROR("Invalid duration for CORESET, must be 1, 2 or 3");
  }
}


void coreset::set_cce_reg_mapping_type(std::string cce_reg_mapping_type_){
  if (isvalid_coreset_cce_reg_mapping_type(cce_reg_mapping_type_)){
    cce_reg_mapping_type = cce_reg_mapping_type_;
  }
  else{
    cce_reg_mapping_type = "non-interleaved"; 
    SPDLOG_ERROR("Invalid CCE-REG mapping type");
  }
}


void coreset::set_reg_bundlesize(uint8_t reg_bundlesize_){
  if (isvalid_coreset_reg_bundlesize(reg_bundlesize_,cce_reg_mapping_type,duration)){
    reg_bundlesize = reg_bundlesize_;
  }
  else{
    reg_bundlesize = 6; 
    SPDLOG_ERROR("Invalid Bundle Size for the CORESET configuration");
  }
}


void coreset::set_interleaver_size(uint8_t interleaver_size_){
  if (isvalid_interleaver_size(interleaver_size_)){
    interleaver_size = interleaver_size_;
  }
  else{
    interleaver_size = 2; 
    SPDLOG_ERROR("Invalid Interleaver Size, must be 2 3 or 6");
  }
}


void coreset::set_shift_index(uint16_t shift_index_){
  if (isvalid_shift_index(shift_index_)){
    shift_index = shift_index_;
  }
  else{
    shift_index = 0; 
    SPDLOG_ERROR("Invalid nShift index, must be between 0 and 1008");
  }
}


void coreset::set_cell_id(uint16_t cell_id_){
  cell_id = cell_id_;
}

void coreset::set_starting_ofdm_symbol_within_slot(uint8_t starting_ofdm_symbol_within_slot_){
  starting_ofdm_symbol_within_slot = starting_ofdm_symbol_within_slot_;
}
  
void coreset::set_num_symbols_per_slot(uint8_t num_symbols_per_slot_){
  num_symbols_per_slot = num_symbols_per_slot_;
}

void coreset::set_num_slots_per_frame(uint8_t num_slots_per_frame_){
  num_slots_per_frame = num_slots_per_frame_;
}

void coreset::set_candidates_search_space(std::vector<uint8_t> candidates_search_space_){
  candidates_search_space = candidates_search_space_;
}


uint8_t coreset::get_control_resourceset_id(){
  return control_resourceset_id;
}

uint16_t coreset::get_frequency_domain_resources(){
  return frequency_domain_resources;
}


uint8_t  coreset::get_duration(){
  return duration;
}

std::string coreset::get_cce_reg_mapping_type(){
  return cce_reg_mapping_type;
}

uint8_t coreset::get_reg_bundlesize(){
  return reg_bundlesize;
}

uint8_t coreset::get_interleaver_size(){
  return interleaver_size;
}

uint16_t coreset::get_shift_index(){
  return shift_index;
}

uint16_t coreset::get_cell_id(){
  return cell_id;
}

uint8_t coreset::get_starting_ofdm_symbol_within_slot(){
  return starting_ofdm_symbol_within_slot;
}

uint8_t coreset::get_num_symbols_per_slot(){
  return num_symbols_per_slot;
}

uint8_t coreset::get_num_slots_per_frame(){
  return num_slots_per_frame;
}

std::vector<uint8_t> coreset::get_candidates_search_space(){
  return candidates_search_space;
}
