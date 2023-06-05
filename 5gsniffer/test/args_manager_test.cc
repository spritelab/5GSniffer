#include "gtest/gtest.h"
#include "args_manager.h"

TEST(args_test, input_check) {
  args_t args;
  args_manager::default_args(args);
  EXPECT_EQ(args.force_N_id_2, -1);
  // EXPECT_NO_THROW(ArgsManager::defaultArgs(args));
}