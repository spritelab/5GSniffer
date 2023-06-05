#include "gtest/gtest.h"
#include "pdcch.h"
#include "symbol.h"
#include <vector>
#include "fftw3.h"
#include "file_source.h"
#include "utils.h"
#include <numeric>
#include <span>

class pdcch_process_test : public ::testing::Test {
 protected:
  pdcch_process_test() {
  }

};


TEST_F(pdcch_process_test, test_pdcch_processing) {

  int num_samples = 576;

  std::string samples_dir = "../../test/samples/";
  file_source source(num_samples, samples_dir + "SIB1_Slot1_OFDM1_Matlab_EQUALIZED.fc32");

  // Buffer for the input source file. It is the SIB1 PDCCH, only the OFDM symbol that contains it. 

  shared_ptr<vector<std::complex<float>>> file_symbols = source.produce_samples(num_samples);
  SPDLOG_INFO("size of file_symbols {}", file_symbols->size());
  vector<symbol> produced_symbols;
  produced_symbols.reserve(num_samples);

  symbol s;
  s.samples = std::move(*file_symbols);
  s.symbol_index = 0;
  produced_symbols.push_back(std::move(s));
  
  pdcch pdcch1;
  // For SI
  coreset coreset_info_(0,48,1,"interleaved",6,2,102,102, 0, 14, 10, {8, 4, 2, 1, 0});
  pdcch1.set_coreset_info(coreset_info_);
  pdcch1.scrambling_id_start = 102;
  pdcch1.scrambling_id_end = 102;
  pdcch1.rnti_start = 65535;
  pdcch1.rnti_end = 65535;
  SPDLOG_DEBUG(" pdcch al corr thresh {}, {}, {}, {}, {}", pdcch1.AL_corr_thresholds.at(0), pdcch1.AL_corr_thresholds.at(1), pdcch1.AL_corr_thresholds.at(2),pdcch1.AL_corr_thresholds.at(3),pdcch1.AL_corr_thresholds.at(4));
  auto pdcch_symbols = make_shared<vector<symbol>>(produced_symbols.begin(), produced_symbols.begin()+1);
  pdcch1.work(pdcch_symbols);
}

