#include <gtest/gtest.h>

#include <Include/Matrix.hpp>

TEST(MatrixTest, CreateEmptyMatrix)
{
  matrix::Matrix       matrix({ 0, 0, 0 });

  const matrix::Shape& shape = matrix.shape();

  EXPECT_EQ(shape.m_x, 0);
  EXPECT_EQ(shape.m_y, 0);
  EXPECT_EQ(shape.m_z, 0);

  EXPECT_THROW(
      {
        try
          {
            matrix.get_at(0, 0, 0);
          }
        catch(const std::out_of_range& e)
          {
            EXPECT_STREQ(e.what(), "Out of range");
            throw;
          }
      },
      std::out_of_range);

  EXPECT_THROW(
      {
        try
          {
            matrix.set_at(0, 0, 0, 0);
          }
        catch(const std::out_of_range& e)
          {
            EXPECT_STREQ(e.what(), "Out of range");
            throw;
          }
      },
      std::out_of_range);
}

TEST(MatrixTest, SetGetValue)
{
  matrix::Matrix       matrix({ 10, 10, 10 });

  const matrix::Shape& shape = matrix.shape();

  EXPECT_EQ(shape.m_x, 10);
  EXPECT_EQ(shape.m_y, 10);
  EXPECT_EQ(shape.m_z, 10);

  EXPECT_NO_THROW(matrix.set_at(10, 5, 4, 3));

  EXPECT_NO_THROW({
    const uint8_t value = matrix.get_at(5, 4, 3);
    EXPECT_EQ(value, 10);
  });
}

TEST(MatrixTest, CopyAssign)
{
  matrix::Matrix matrix({ 10, 10, 10 });
  matrix.set_at(10, 5, 4, 3);

  matrix::Matrix copy_matrix = matrix;

  EXPECT_NO_THROW({
    const uint8_t value = copy_matrix.get_at(5, 4, 3);
    EXPECT_EQ(value, 10);
  });
}

TEST(MatrixTest, MoveAssign)
{
  matrix::Matrix matrix({ 10, 10, 10 });
  matrix.set_at(10, 5, 4, 3);

  matrix::Matrix       copy_matrix = std::move(matrix);

  const matrix::Shape& shape       = matrix.shape();

  EXPECT_EQ(shape.m_x, 0);
  EXPECT_EQ(shape.m_y, 0);
  EXPECT_EQ(shape.m_z, 0);

  EXPECT_THROW(
      {
        try
          {
            matrix.get_at(0, 0, 0);
          }
        catch(const std::out_of_range& e)
          {
            EXPECT_STREQ(e.what(), "Out of range");
            throw;
          }
      },
      std::out_of_range);

  EXPECT_NO_THROW({
    const uint8_t value = copy_matrix.get_at(5, 4, 3);
    EXPECT_EQ(value, 10);
  });
}

int
main(int argc, char* argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
