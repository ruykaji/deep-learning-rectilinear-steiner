#include <gtest/gtest.h>

#include "Include/Ini.hpp"

class IniTest : public ::testing::Test
{
protected:
  IniTest() : m_config_path("./config_example.ini") {};

  ~IniTest() override {};

  void
  SetUp() override
  {
    m_config_stream.open(m_config_path);
  }

  void
  TearDown() override
  {
    if(m_config_stream.is_open() && m_config_stream.good())
      {
        m_config_stream.close();
      }
  }

protected:
  std::filesystem::path m_config_path;
  std::ofstream         m_config_stream;
};

TEST_F(IniTest, ParseWithNoExceptions)
{
  this->m_config_stream << "[Section1]\n";
  this->m_config_stream << "StringKey = value\n";
  this->m_config_stream << "IntegerKey = 10\n\n";
  this->m_config_stream << "[Section2]\n";
  this->m_config_stream << "FloatKey = 0.1\n";
  this->m_config_stream << "BooleanKey = true\n";
  this->m_config_stream << std::flush;

  ini::Config config = ini::parse(this->m_config_path);

  EXPECT_EQ(config.count("Section1"), 1);
  EXPECT_EQ(config.at("Section1").get_as<std::string>("StringKey"), "value");
  EXPECT_EQ(config.at("Section1").get_as<int32_t>("IntegerKey"), 10);

  EXPECT_EQ(config.count("Section2"), 1);
  EXPECT_EQ(config.at("Section2").get_as<double>("FloatKey"), 0.1);
  EXPECT_EQ(config.at("Section2").get_as<bool>("BooleanKey"), true);
}

TEST_F(IniTest, NoFileFoundException)
{
  EXPECT_THROW(
      {
        try
          {
            ini::parse("./no_existing_config.ini");
          }
        catch(const std::exception& e)
          {
            EXPECT_STREQ(e.what(), "Failed to locate config file. Path \"./no_existing_config.ini\" does not exists");
            throw;
          }
      },
      std::runtime_error);
}

TEST_F(IniTest, NoKeyValuePairException)
{
  this->m_config_stream << "[Section1]\n";
  this->m_config_stream << "StringKey\n";
  this->m_config_stream << std::flush;

  EXPECT_THROW(
      {
        try
          {
            ini::parse(this->m_config_path);
          }
        catch(const std::exception& e)
          {
            EXPECT_STREQ(e.what(), "Expected key value pair but found nothing:\n\"StringKey\"");
            throw;
          }
      },
      std::invalid_argument);
}

TEST_F(IniTest, NoKeyException)
{
  this->m_config_stream << "[Section1]\n";
  this->m_config_stream << "= value\n";
  this->m_config_stream << std::flush;

  EXPECT_THROW(
      {
        try
          {
            ini::parse(this->m_config_path);
          }
        catch(const std::exception& e)
          {
            EXPECT_STREQ(e.what(), "Expected key but found nothing:\n\"= value\"");
            throw;
          }
      },
      std::invalid_argument);
}

TEST_F(IniTest, NoValueException)
{
  this->m_config_stream << "[Section1]\n";
  this->m_config_stream << "StringKey =\n";
  this->m_config_stream << std::flush;

  EXPECT_THROW(
      {
        try
          {
            ini::parse(this->m_config_path);
          }
        catch(const std::exception& e)
          {
            EXPECT_STREQ(e.what(), "Expected value but found nothing:\n\"StringKey =\"");
            throw;
          }
      },
      std::invalid_argument);
}

TEST_F(IniTest, UnsupportedFormatException)
{
  this->m_config_stream << "[Section1]\n";
  this->m_config_stream << "StringKey=value\n";
  this->m_config_stream << std::flush;

  EXPECT_THROW(
      {
        try
          {
            ini::parse(this->m_config_path);
          }
        catch(const std::exception& e)
          {
            EXPECT_STREQ(e.what(), "Unsupported format:\n\"StringKey=value\"");
            throw;
          }
      },
      std::invalid_argument);
}

TEST_F(IniTest, NoKeyExistsException)
{
  this->m_config_stream << "[Section1]\n";
  this->m_config_stream << "StringKey = value\n";
  this->m_config_stream << std::flush;

  EXPECT_THROW(
      {
        try
          {
            ini::Config config = ini::parse(this->m_config_path);
            config.at("Section1").get_as<std::string>("NumberKey");
          }
        catch(const std::exception& e)
          {
            EXPECT_STREQ(e.what(), "Key \"NumberKey\" doesn't exists");
            throw;
          }
      },
      std::invalid_argument);
}

TEST_F(IniTest, InvalidValueForBooleanTypeException)
{
  this->m_config_stream << "[Section1]\n";
  this->m_config_stream << "StringKey = value\n";
  this->m_config_stream << std::flush;

  EXPECT_THROW(
      {
        try
          {
            ini::Config config = ini::parse(this->m_config_path);
            config.at("Section1").get_as<bool>("StringKey");
          }
        catch(const std::exception& e)
          {
            EXPECT_STREQ(e.what(), "Invalid value for key \"StringKey\" and boolean type");
            throw;
          }
      },
      std::invalid_argument);
}

TEST_F(IniTest, InvalidValueForArithmeticTypeException)
{
  this->m_config_stream << "[Section1]\n";
  this->m_config_stream << "StringKey = value\n";
  this->m_config_stream << std::flush;

  EXPECT_THROW(
      {
        try
          {
            ini::Config config = ini::parse(this->m_config_path);
            config.at("Section1").get_as<int32_t>("StringKey");
          }
        catch(const std::exception& e)
          {
            EXPECT_STREQ(e.what(), "Invalid value for key \"StringKey\" and arithmetic type");
            throw;
          }
      },
      std::invalid_argument);
}

int
main(int argc, char* argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
