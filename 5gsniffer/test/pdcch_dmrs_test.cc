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

#include "gtest/gtest.h"
#include "pdcch.h"
#include "dmrs.h"
#include "fftw3.h"

class pdcch_dmrs_test : public ::testing::Test {
 protected:
  pdcch_dmrs_test() {
  }

};

TEST_F(pdcch_dmrs_test, test_pdcch_set) {

pdcch pdcch_test;

coreset coreset_info_(0,48,1,"interleaved",6,2,160,0, 0, 14, 10, {8, 4, 2, 1, 0});

pdcch_test.set_coreset_info(coreset_info_);

EXPECT_EQ(pdcch_test.get_coreset_info().get_duration(), 1);

EXPECT_EQ(pdcch_test.get_coreset_info().get_reg_bundlesize(), 6);

EXPECT_EQ(pdcch_test.get_coreset_info().get_interleaver_size(), 2);

}


TEST_F(pdcch_dmrs_test, test_pdcch_interleaver) {

pdcch pdcch;

/*PDCCH1 Comparing with the example from: https://www.linkedin.com/pulse/5g-nr-coreset-configuration-pdcch-resources-mapping-naveen-chelikani/
  Rest comparing with Matlab generated*/

std::vector<uint16_t> interleaver_output{0,4,1,5,2,6,3,7};

std::vector<uint16_t> interleaver_output_matlab{6,2,7,3,0,4,1,5};

std::vector<uint16_t> interleaver_output_matlab_regbundle2{6,18,7,19,8,20,9,21,10,22,11,23,12,0,13,1,14,2,15,3,16,4,17,5};

coreset coreset_info_1(0,48,1,"interleaved",6,2,160,0, 0, 14, 10, {8, 4, 2, 1, 0});

coreset coreset_info_2(0,48,1,"interleaved",6,2,102,102, 0, 14, 10, {8, 4, 2, 1, 0});

coreset coreset_info_3(0,48,1,"interleaved",2,2,102,102, 0, 14, 10, {8, 4, 2, 1, 0});

coreset coreset_info_4(0,48,2,"interleaved",6,2,102,102, 0, 14, 10, {8, 4, 2, 1, 0});

std::vector<uint16_t> interleaved = {};

pdcch.set_coreset_info(coreset_info_1);

interleaved = pdcch.cce_reg_interleaving();

for (int i_idx = 0 ; i_idx < interleaved.size(); i_idx++)
  EXPECT_EQ(interleaved.at(i_idx), interleaver_output.at(i_idx));


interleaved = {};

pdcch.set_coreset_info(coreset_info_2);

interleaved = pdcch.cce_reg_interleaving();

for (int i_idx = 0 ; i_idx < interleaved.size(); i_idx++)
  EXPECT_EQ(interleaved.at(i_idx), interleaver_output_matlab.at(i_idx));

interleaved = {};

pdcch.set_coreset_info(coreset_info_3);

interleaved = pdcch.cce_reg_interleaving();

for (int i_idx = 0 ; i_idx < interleaved.size(); i_idx++)
  EXPECT_EQ(interleaved.at(i_idx), interleaver_output_matlab_regbundle2.at(i_idx));

}



TEST_F(pdcch_dmrs_test, test_pdcch_dmrs_rb_indices) {
 
  pdcch pdcch;

  coreset coreset_info_(0,48,1,"interleaved",6,2,102,102, 0, 14, 10, {8, 4, 2, 1, 0});

  pdcch.set_coreset_info(coreset_info_);

  std::vector<uint16_t> pdcch_dmrs_rb_indices_4 = pdcch.get_rb_interleaved(4);

  std::vector<uint16_t> pdcch_dmrs_rb_indices_8 = pdcch.get_rb_interleaved(8);

  dmrs dmrs_pdcch;

}

TEST_F(pdcch_dmrs_test, test_pdcch_sib1_indices_matlab) {

uint16_t nid_cell = 102;

constexpr uint16_t aggregation_level = 4;

constexpr uint16_t max_aggregation_level = 8;

int num_candidate = 0;

int max_num_candidate = 4;

int numSlot = 9;

bool user_search_space = false;

std::array <uint16_t,aggregation_level*18> pdcch_dmrs_indices_matlab_sib1_al4_candidate1 = {146,150,154,158,162,166,170,174,178,182,186,190,194,198,202,206,210,214,218,222,226,230,234,238,242,246,250,254,258,262,266,270,274,278,282,286,434,438,442,446,450,454,458,462,466,470,474,478,482,486,490,494,498,502,506,510,514,518,522,526,530,534,538,542,546,550,554,558,562,566,570,574};

std::array <uint16_t,max_aggregation_level*18> pdcch_dmrs_indices_matlab_sib1_al8_candidate1 = {2,6,10,14,18,22,26,30,34,38,42,46,50,54,58,62,66,70,74,78,82,86,90,94,98,102,106,110,114,118,122,126,130,134,138,142,146,150,154,158,162,166,170,174,178,182,186,190,194,198,202,206,210,214,218,222,226,230,234,238,242,246,250,254,258,262,266,270,274,278,282,286,290,294,298,302,306,310,314,318,322,326,330,334,338,342,346,350,354,358,362,366,370,374,378,382,386,390,394,398,402,406,410,414,418,422,426,430,434,438,442,446,450,454,458,462,466,470,474,478,482,486,490,494,498,502,506,510,514,518,522,526,530,534,538,542,546,550,554,558,562,566,570,574};

std::array <uint16_t,aggregation_level*18> pdcch_dmrs_indices_matlab_sib1_al4_candidate1_regBundle2 = {146,150,154,158,162,166,170,174,178,182,186,190,194,198,202,206,210,214,218,222,226,230,234,238,242,246,250,254,258,262,266,270,274,278,282,286,434,438,442,446,450,454,458,462,466,470,474,478,482,486,490,494,498,502,506,510,514,518,522,526,530,534,538,542,546,550,554,558,562,566,570,574};

std::array <uint16_t,aggregation_level*18> pdcch_dmrs_indices_matlab_sib1_al4_candidate1_regBundle6_Duration2 = {2,6,10,14,18,22,26,30,34,38,42,46,50,54,58,62,66,70,290,294,298,302,306,310,314,318,322,326,330,334,338,342,346,350,354,358,578,582,586,590,594,598,602,606,610,614,618,622,626,630,634,638,642,646,866,870,874,878,882,886,890,894,898,902,906,910,914,918,922,926,930,934};

std::array <uint16_t,aggregation_level*18> pdcch_dmrs_indices_matlab_sib1_al4_candidate1_regBundle6_Duration3 = {2,6,10,14,18,22,26,30,34,38,42,46,290,294,298,302,306,310,314,318,322,326,330,334,578,582,586,590,594,598,602,606,610,614,618,622,866,870,874,878,882,886,890,894,898,902,906,910,1154,1158,1162,1166,1170,1174,1178,1182,1186,1190,1194,1198,1442,1446,1450,1454,1458,1462,1466,1470,1474,1478,1482,1486};

std::array <uint16_t,aggregation_level*18> pdcch_dmrs_indices_matlab_sib1_al4_candidate1_regBundle6_Duration1_nonInterleaved = {2,6,10,14,18,22,26,30,34,38,42,46,50,54,58,62,66,70,74,78,82,86,90,94,98,102,106,110,114,118,122,126,130,134,138,142,146,150,154,158,162,166,170,174,178,182,186,190,194,198,202,206,210,214,218,222,226,230,234,238,242,246,250,254,258,262,266,270,274,278,282,286};

/*Generating a PDCCH object with a CORESET object with the same info as in Matlab for SIB1 (no recording but SIB1 generated on Matlab)*/

pdcch pdcch1;

pdcch pdcch2;

pdcch pdcch3;

pdcch pdcch4;

pdcch pdcch5;

pdcch pdcch6;

pdcch6.get_dmrs_sc_indices(aggregation_level, num_candidate, max_num_candidate, numSlot, user_search_space);

coreset coreset_info_(0,48,1,"interleaved",6,2,102,102, 0, 14, 10, {8, 4, 2, 1, 0});

coreset coreset_info_2(0,48,1,"interleaved",2,2,102,102, 0, 14, 10, {8, 4, 2, 1, 0});

coreset coreset_info_3(0,48,2,"interleaved",6,2,0,1, 0, 14, 10, {8, 4, 2, 1, 0});

coreset coreset_info_4(0,48,3,"interleaved",6,2,0,1, 0, 14, 10, {8, 4, 2, 1, 0});

coreset coreset_info_5(0,48,1,"non-interleaved",6,2,102,102, 0, 14, 10, {8, 4, 2, 1, 0});

pdcch1.set_coreset_info(coreset_info_);
pdcch2.set_coreset_info(coreset_info_2);
pdcch3.set_coreset_info(coreset_info_3);
pdcch4.set_coreset_info(coreset_info_4);
pdcch5.set_coreset_info(coreset_info_5);

std::vector<uint64_t> pdcch_dmrs_indices_al4 = pdcch1.get_dmrs_sc_indices(aggregation_level, num_candidate, max_num_candidate, numSlot, user_search_space);

std::vector<uint64_t> pdcch_dmrs_indices_al8 = pdcch1.get_dmrs_sc_indices(max_aggregation_level, num_candidate, max_num_candidate, numSlot, user_search_space);

std::vector<uint64_t> pdcch_dmrs_indices_al4_regBundle2 = pdcch2.get_dmrs_sc_indices(aggregation_level, num_candidate, max_num_candidate, numSlot, user_search_space);

std::vector<uint64_t> pdcch_dmrs_indices_al4_regBundle6_Duration2 = pdcch3.get_dmrs_sc_indices(aggregation_level, num_candidate, max_num_candidate, numSlot, user_search_space);

std::vector<uint64_t> pdcch_dmrs_indices_al4_regBundle6_Duration3 = pdcch4.get_dmrs_sc_indices(aggregation_level, num_candidate, max_num_candidate, numSlot, user_search_space);

std::vector<uint64_t> pdcch_dmrs_indices_al4_regBundle6_Duration1_nonInterleaved = pdcch5.get_dmrs_sc_indices(aggregation_level, num_candidate, max_num_candidate, numSlot, user_search_space);

std::vector<uint16_t> pdcch_data_indices_al4_regBundle6_Duration1_nonInterleaved = pdcch5.get_data_sc_indices(aggregation_level, num_candidate, max_num_candidate, numSlot, user_search_space);

/*PDCCH-DMRS indices test, MATLAB is not 0-based, adding +1 */

for (int i_idx = 0 ; i_idx < pdcch_dmrs_indices_matlab_sib1_al4_candidate1.size(); i_idx++)
  EXPECT_EQ(pdcch_dmrs_indices_al4.at(i_idx)+1, pdcch_dmrs_indices_matlab_sib1_al4_candidate1.at(i_idx));

for (int i_idx = 0 ; i_idx < pdcch_dmrs_indices_matlab_sib1_al8_candidate1.size(); i_idx++)
  EXPECT_EQ(pdcch_dmrs_indices_al8.at(i_idx)+1, pdcch_dmrs_indices_matlab_sib1_al8_candidate1.at(i_idx));

for (int i_idx = 0 ; i_idx < pdcch_dmrs_indices_matlab_sib1_al4_candidate1_regBundle2.size(); i_idx++)
  EXPECT_EQ(pdcch_dmrs_indices_al4_regBundle2.at(i_idx)+1, pdcch_dmrs_indices_matlab_sib1_al4_candidate1_regBundle2.at(i_idx));

for (int i_idx = 0 ; i_idx < pdcch_dmrs_indices_matlab_sib1_al4_candidate1_regBundle6_Duration2.size(); i_idx++)
  EXPECT_EQ(pdcch_dmrs_indices_al4_regBundle6_Duration2.at(i_idx)+1, pdcch_dmrs_indices_matlab_sib1_al4_candidate1_regBundle6_Duration2.at(i_idx));

for (int i_idx = 0 ; i_idx < pdcch_dmrs_indices_matlab_sib1_al4_candidate1_regBundle6_Duration3.size(); i_idx++)
  EXPECT_EQ(pdcch_dmrs_indices_al4_regBundle6_Duration3.at(i_idx)+1, pdcch_dmrs_indices_matlab_sib1_al4_candidate1_regBundle6_Duration3.at(i_idx));

for (int i_idx = 0 ; i_idx < pdcch_dmrs_indices_al4_regBundle6_Duration1_nonInterleaved.size(); i_idx++)
  EXPECT_EQ(pdcch_dmrs_indices_al4_regBundle6_Duration1_nonInterleaved.at(i_idx) + 1, pdcch_dmrs_indices_matlab_sib1_al4_candidate1_regBundle6_Duration1_nonInterleaved.at(i_idx));


}

