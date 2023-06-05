#include "common_checks.h"

bool valid_nid_2(uint8_t nid_2)
{
  if (nid_2 <= nid_2_max & nid_2 >= 0) {
    return true;
  } else {
    return false;
  }
}

bool valid_nid_1(uint16_t nid_1)
{
  if ((nid_1 <= nid_1_max) & (nid_1 >= 0)) {
    return true;
  } else {
    return false;
  }
}

bool valid_scs_ssb(uint8_t scs_ssb)
{
  std::array<uint8_t,4> valid_list = {15, 30, 120, 240}; // Only ones valid for SSB. 15 or 30 for FR1.
  uint8_t *found_pos = std::find(std::begin(valid_list), std::end(valid_list), scs_ssb);
  if (found_pos != std::end(valid_list)) {
      return true;
  } else {
      return false;
  }
}



/*CORESET checks*/

bool isvalid_coreset_duration(uint8_t duration)
{
  std::array<uint8_t,3> valid_list = {1, 2, 3};
  uint8_t *found_pos = std::find(std::begin(valid_list), std::end(valid_list), duration);
  if (found_pos != std::end(valid_list)) {
      return true;
  } else {
      return false;
  }
}

bool isvalid_coreset_cce_reg_mapping_type(std::string cce_reg_mapping_type)
{
  if (cce_reg_mapping_type.compare("non-interleaved") == 0 || cce_reg_mapping_type.compare("interleaved") == 0)
    return true;
  else
    return false;
}

bool isvalid_coreset_reg_bundlesize(uint8_t reg_bundlesize, std::string cce_reg_mapping_type, uint8_t duration)
{
  std::array<uint8_t,3> valid_list = {2, 3, 6};
  
  uint8_t *found_pos = std::find(std::begin(valid_list), std::end(valid_list), reg_bundlesize);
  
  bool isvalid = false;

  if (found_pos != std::end(valid_list)){
    if (cce_reg_mapping_type.compare("interleaved") == 0){
      if (((duration == 1 || duration == 2) && reg_bundlesize == 2) || (duration == 3 && reg_bundlesize == 3) || reg_bundlesize == 6){
        isvalid = true;
      }
    }else{
      if (reg_bundlesize == 6){
        isvalid = true;
      }
    }
  }
  return isvalid;
}

bool isvalid_interleaver_size(uint8_t interleaver_size)
{
  std::array<uint8_t,3> valid_list = {2, 3, 6};
  uint8_t *found_pos = std::find(std::begin(valid_list), std::end(valid_list), interleaver_size);
  if (found_pos != std::end(valid_list)) {
      return true;
  } else {
      return false;
  }
}


bool isvalid_shift_index(uint16_t shift_index)
{
  if ((shift_index <= nid_max) & (shift_index >= 0)) {
    return true;
  } else {
    return false;
  }

}

