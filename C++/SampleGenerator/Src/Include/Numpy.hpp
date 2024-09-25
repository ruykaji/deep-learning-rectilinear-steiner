#ifndef __NUMPY_HPP__
#define __NUMPY_HPP__

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace numpy
{

/**
 * @brief Saves a matrix as numpy array.
 *
 * @tparam Tp The data type.
 * @param file_path The save path.
 * @param data The matrix's data to save.
 * @param shape The matrix's shape.
 */
template <typename Tp>
void
save_as(const std::filesystem::path& file_path, const char* data, const std::vector<uint8_t>& shape)
{
  if(!std::filesystem::exists(file_path.parent_path()))
    {
      throw std::invalid_argument("Numpy Error: Can't locate parent folder by given path: \"" + file_path.string() + "\".");
    }

  if(data == nullptr)
    {
      throw std::invalid_argument("Numpy Error: data nullptr exception.");
    }

  if(shape.size() == 0)
    {
      throw std::invalid_argument("Numpy Error: Can't save zero shaped matrix.");
    }

  std::ofstream out_file(file_path, std::ios::binary);

  if(!out_file.is_open() || !out_file.good())
    {
      throw std::runtime_error("Numpy Error: Failed to open the file.");
    }

  std::size_t data_length = 1;

  /** Writing header */
  std::string header;

  if constexpr(std::is_floating_point_v<Tp>)
    {
      header = "{'descr': '<f" + std::to_string(sizeof(Tp)) + "', 'fortran_order': False, 'shape': (";
    }
  else if constexpr(std::is_signed_v<Tp>)
    {
      header = "{'descr': '<i" + std::to_string(sizeof(Tp)) + "', 'fortran_order': False, 'shape': (";
    }
  else if constexpr(std::is_unsigned_v<Tp>)
    {
      header = "{'descr': '<u" + std::to_string(sizeof(Tp)) + "', 'fortran_order': False, 'shape': (";
    }
  else
    {
      throw std::invalid_argument("Numpy Error: Unsupported data type.");
    }

  for(std::size_t i = 0, end = shape.size(); i < end; ++i)
    {
      data_length *= shape[i];

      if(i != end - 1)
        {
          header += std::to_string(shape[i]) + ", ";
        }
      else
        {
          header += std::to_string(shape[i]);
        }
    }

  header += "), }";

  std::size_t       header_length       = header.size();
  const std::size_t total_header_length = 8 + 2 + header_length;                  /** 8 bytes for magic + 2 byte version + header_len_field */
  const std::size_t padding             = (16 - (total_header_length % 16)) % 16; /** Align to 16 bytes */

  /** Update header length to include padding */
  header_length += padding + 1;

  /** Prepare the magic string, version, and header length field */
  const std::string magic            = "\x93NUMPY"; /** Magic string for NumPy */
  const char        version[2]       = { 1, 0 };    /** Numpy version 1.0 */
  const uint16_t    header_len_field = static_cast<uint16_t>(header_length);

  /** Write magic string, version, and header length */
  out_file.write(magic.c_str(), magic.size());
  out_file.write(version, 2);
  out_file.write(reinterpret_cast<const char*>(&header_len_field), sizeof(uint16_t));

  /** Write the header content */
  out_file.write(header.c_str(), header.size());

  /** Write padding */
  for(std::size_t i = 0; i < padding; ++i)
    {
      out_file.write(" ", 1); /** Padding with spaces */
    }

  out_file.write("\n", 1);

  /** Write the data */
  out_file.write(data, data_length * sizeof(Tp));

  /** Close the file */
  out_file.close();
}

} // namespace numpy

#endif