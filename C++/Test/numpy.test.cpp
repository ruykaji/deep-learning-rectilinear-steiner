#include <gtest/gtest.h>

#include "Include/Numpy.hpp"

TEST(NumpyTest, SaveData)
{
  const std::vector<uint8_t> data  = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
  const std::vector<uint8_t> shape = { 3, 3 };

  EXPECT_NO_THROW(
      {
        numpy::save_as<uint8_t>("./matrix.npz", reinterpret_cast<const char*>(data.data()), shape);
      });
}

int
main(int argc, char* argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
