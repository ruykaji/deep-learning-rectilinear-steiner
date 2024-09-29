#include "Include/Ini.hpp"

namespace ini
{

namespace details
{

/**
 * @brief Trim leading spaces
 *
 * @param line Line to trim
 */
inline void
ltrim(std::string& line)
{
  line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](unsigned char ch) { return !std::isspace(ch); }));
}

/**
 * @brief Trim trailing  spaces
 *
 * @param line Line to trim
 */
inline void
rtrim(std::string& line)
{
  line.erase(std::find_if(line.rbegin(), line.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), line.end());
}

} // namespace details

/** =============================== PUBLIC METHODS =============================== */

bool
Section::check_key(const std::string& key) const
{
  return m_config.count(key) != 0;
}

/** =============================== GLOBAL FUNCTIONS ============================= */

Config
parse(const std::filesystem::path& file_path)
{
  if(!std::filesystem::exists(file_path))
    {
      throw std::runtime_error("Failed to locate config file. Path \"" + file_path.string() + "\" does not exists");
    }

  Config        config;
  std::string   section_name = "";

  std::ifstream in_file(file_path);

  if(!in_file.is_open() || !in_file.good())
    {
      throw std::runtime_error("Unexpected error occurred while trying to open file:\n" + file_path.string());
    }

  std::string line;

  while(std::getline(in_file, line))
    {
      if(line.empty())
        {
          continue;
        }

      details::ltrim(line);
      details::rtrim(line);

      if(line.at(0) == ';')
        {
          continue;
        }

      if(line.at(0) == '[')
        {
          section_name = line.substr(1, line.length() - 2);
          continue;
        }

      const std::size_t separator = line.find_first_of('=', 0);

      if(separator == std::string::npos)
        {
          throw std::invalid_argument("Expected key value pair but found nothing:\n\"" + line + "\"");
        }

      if(separator == 0)
        {
          throw std::invalid_argument("Expected key but found nothing:\n\"" + line + "\"");
        }

      if(separator == line.length() - 1)
        {
          throw std::invalid_argument("Expected value but found nothing:\n\"" + line + "\"");
        }

      if(!std::isspace(line.at(separator - 1)) || !std::isspace(line.at(separator + 1)))
        {
          throw std::invalid_argument("Unsupported format:\n\"" + line + "\"");
        }

      const std::string key              = line.substr(0, separator - 1);
      const std::string value            = line.substr(separator + 2);

      config[section_name].m_config[key] = value;
    }

  return config;
}

} // namespace ini