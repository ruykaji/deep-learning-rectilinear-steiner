#ifndef __INI_HPP__
#define __INI_HPP__

#include <algorithm>
#include <cinttypes>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>

namespace ini
{

/**
 * @brief Section with parameters in ini file.
 *
 */
class Section
{
  friend std::unordered_map<std::string, Section>
  parse(const std::filesystem::path& file_path);

public:
  /** =============================== PUBLIC METHODS =============================== */

  /**
   * @brief Checks if key presents in section.
   *
   * @param key The key to check
   * @return true
   * @return false
   */
  bool
  check_key(const std::string& key) const;

  /**
   * @brief Gets the section parameter as object of some type.
   *
   * @tparam Tp Type.
   * @param key Key.
   * @return A value of a `Tp` type
   */
  template <typename Tp>
  Tp
  get_as(const std::string key) const
  {
    static_assert(std::is_same_v<Tp, bool> || std::is_arithmetic_v<Tp> || std::is_same_v<Tp, std::string>, "No conversions exists for this type");

    if(!m_config.count(key))
      {
        throw std::invalid_argument("Key \"" + key + "\" doesn't exists");
      }

    const std::string& value = m_config.at(key);

    if constexpr(std::is_same_v<Tp, bool>)
      {
        if(value == "true" || value == "1" || value == "on")
          {
            return true;
          }

        if(value == "false" || value == "0" || value == "off")
          {
            return false;
          }

        throw std::invalid_argument("Invalid value for key \"" + key + "\" and boolean type");
      }

    if constexpr(std::is_integral_v<Tp>)
      {
        try
          {
            Tp number = static_cast<Tp>(std::stoll(value.data()));
            return number;
          }
        catch(...)
          {
            throw std::invalid_argument("Invalid value for key \"" + key + "\" and arithmetic type");
          }
      }

    if constexpr(std::is_floating_point_v<Tp>)
      {
        try
          {
            Tp number = static_cast<Tp>(std::stold(value.data()));
            return number;
          }
        catch(...)
          {
            throw std::invalid_argument("Invalid value for key \"" + key + "\" and arithmetic type");
          }
      }

    if constexpr(std::is_same_v<Tp, std::string>)
      {
        return value;
      }
  }

private:
  std::unordered_map<std::string, std::string> m_config; ///> Holds key&value pairs.
};

/** Config alias */
using Config = std::unordered_map<std::string, Section>;

/**
 * @brief Parses config file.
 *
 * @param file_path Path to the configuration ini file.
 * @return A `Config` object.
 */
Config
parse(const std::filesystem::path& file_path);

} // namespace ini

#endif