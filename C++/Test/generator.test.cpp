#include <gtest/gtest.h>

#include "Include/Generator.hpp"

TEST(GeneratorTest, NumberOfCombinations)
{
  const uint8_t  size                 = 32;
  const uint8_t  depth                = 2;
  const uint8_t  number_of_points     = 5;
  const uint32_t desired_combinations = 20;

  const uint64_t total_combinations   = gen::nCr(size * size * depth, number_of_points);
  const uint32_t step                 = total_combinations / desired_combinations;

  const auto     combinations         = gen::make_combinations_3d(size, depth, number_of_points, step, desired_combinations);

  EXPECT_EQ(combinations.size(), desired_combinations * number_of_points);
}

int
main(int argc, char* argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
