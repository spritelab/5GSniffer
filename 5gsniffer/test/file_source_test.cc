#include "gtest/gtest.h"
#include "file_source.h"
#include "file_sink.h"

class file_source_test : public ::testing::Test {
 protected:
  file_source_test() {
     this->samples_dir = "../../test/samples/";
  }

  std::string samples_dir;
};

TEST_F(file_source_test, read_samples) {
  file_source source(5000000, samples_dir + "att_s5e6_f8715e5_ssb.fc32");

  // Buffer for 10 samples
  vector<complex<float>> all_samples;
  all_samples.reserve(10);

  // Read 10 samples and add to all_samples
  for(int i = 0; i < 2; i++) {
    shared_ptr<vector<complex<float>>> p = source.produce_samples(5);
    EXPECT_EQ(p.get()->size(), 5);
    all_samples.insert(all_samples.end(), p.get()->begin(), p.get()->end());
    EXPECT_EQ(all_samples.size(), (i+1)*5);
  }

  // Compare first 10 samples read
  complex<float> comp[10]= { 
    {-0.012726216  , 0.0021668137},
    {-0.0044557014 ,-0.0021362952},
    {-0.0052797007 , 0.0068056262},
    {-0.018524731  , 0.0055238488},
    {-0.015655991  ,-0.0067445892},
    {-0.0066225152 ,-0.0021973322},
    {-0.0032044428 ,-0.0053712563},
    { 3.0518502e-05,-0.0010681476},
    {-0.013000882  , 0.0059511079},
    {-0.011658068  , 0}
  };
  for (int i = 0; i < all_samples.size(); ++i) {
    EXPECT_FLOAT_EQ(all_samples[i].real(), comp[i].real()) << "Vectors x and y differ at real index " << i;
    EXPECT_FLOAT_EQ(all_samples[i].imag(), comp[i].imag()) << "Vectors x and y differ at imaginary index " << i;
  }

  // Get size of source in bytes
  EXPECT_EQ(source.size_bytes, 171648);
}

TEST_F(file_source_test, to_sink) {
  string source_path = samples_dir + "att_s5e6_f8715e5_ssb.fc32";
  string dest_path = "/tmp/unittest.fc32";

  // Setup flowgraph
  file_source source(5000000, source_path);
  auto sink = make_shared<file_sink>(dest_path);
  source.connect(sink);

  // Push 1024 samples through the flowgraph
  size_t num_samples = 1024;
  size_t num_bytes = num_samples * sizeof(complex<float>);
  source.work(num_samples);

  // Read source data
  ifstream i(source_path, ifstream::binary);
  vector<complex<float>> i_data(num_samples);
  if (i) {
    i.read(reinterpret_cast<char*>(i_data.data()), num_bytes);
  }

  // Read dest data
  ifstream j(dest_path, ifstream::binary);
  vector<complex<float>> j_data(num_samples);
  if (j) {
    j.read(reinterpret_cast<char*>(j_data.data()), num_bytes);
  }

  // Compare if equal
  EXPECT_EQ(i_data, j_data);
}