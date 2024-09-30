#include <atomic>
#include <cstring>
#include <iostream>
#include <queue>
#include <thread>

#include "Include/Algorithms.hpp"
#include "Include/Generator.hpp"
#include "Include/Ini.hpp"
#include "Include/Numpy.hpp"
#include "Include/Transform.hpp"
#include "Include/Utilis.hpp"

namespace
{

/**
 * @brief Converts 1D index to 3D index.
 *
 * @param index The 1D index.
 * @param size The size of a matrix.
 * @return std::tuple<uint8_t, uint8_t, uint8_t>
 */
std::tuple<uint8_t, uint8_t, uint8_t>
index_to_coordinates(uint32_t index, uint8_t size)
{
  const uint16_t layer_size = size * size;
  const uint16_t remainder  = index % layer_size;

  const uint8_t  x          = remainder / size;
  const uint8_t  y          = remainder % size;
  const uint8_t  z          = index / layer_size;

  return { x, y, z };
}

template <typename Tp>
Tp
get_config_number(const ini::Section& section, const std::string& key, Tp default_value, Tp min_value, Tp max_value, const std::string& error_message)
{
  if(!section.check_key(key))
    {
      return default_value;
    }

  Tp value = section.get_as<Tp>(key);

  if(value < min_value || value > max_value)
    {
      std::cerr << error_message << std::endl;
      std::cout << "Using default value instead, which is " << std::to_string(default_value) << "." << std::endl;

      return default_value;
    }

  return value;
}

} // namespace

int
main(int argc, char* argv[])
{
  std::filesystem::path config_path = "./config.ini";

  /** Simple arg-parser */
  if(argc > 1)
    {
      if(std::strcmp(argv[1], "--config") == 0 && argc > 2)
        {
          config_path = argv[2];
        }
    }

  ini::Config config = ini::parse(config_path);

  /** Setup output directory and its subdirectories */
  std::cout << "\n";
  std::cout << "============= Setting up output directories =============" << std::endl;

  std::filesystem::path output_directory = std::filesystem::current_path() / "GeneratedData";

  if(auto it = config.find("Path"); it != config.end())
    {
      const ini::Section& ps = it->second;

      output_directory       = ps.get_as<std::string>("Output");
    }

  if(!std::filesystem::exists(output_directory))
    {
      std::filesystem::create_directories(output_directory);
    }

  const std::filesystem::path source_dir = output_directory / "Source";
  const std::filesystem::path target_dir = output_directory / "Target";
  const std::filesystem::path nodes_dir  = output_directory / "Nodes";

  for(const auto& dir : { source_dir, target_dir, nodes_dir })
    {
      if(std::filesystem::exists(dir))
        {
          std::filesystem::remove_all(dir);
        }

      std::filesystem::create_directory(dir);
    }

  std::cout << "  - Source directory: " << source_dir << std::endl;
  std::cout << "  - Target directory: " << target_dir << std::endl;
  std::cout << "  - Nodes  directory: " << target_dir << std::endl;
  std::cout << "\n";

  /** Setup up generation settings */
  std::cout << "=================== Generating samples ===================" << std::endl;

  uint8_t  size                 = 32;
  uint8_t  depth                = 1;
  uint8_t  max_number_of_points = 4;
  uint32_t desired_combinations = 100;

  if(auto it = config.find("Generation"); it != config.end())
    {
      const ini::Section& gs = it->second;

      size                   = get_config_number<uint8_t>(gs, "Size", size, 1, UINT8_MAX, "Size must be between 1 and 255.");
      depth                  = get_config_number<uint8_t>(gs, "Depth", depth, 1, UINT8_MAX, "Depth must be between 1 and 255.");
      max_number_of_points   = get_config_number<uint8_t>(gs, "MaxNumberOfPoints", max_number_of_points, 1, UINT8_MAX, "MaxNumberOfPoints must be between 1 and 255.");
      desired_combinations   = get_config_number<uint32_t>(gs, "DesiredCombinations", desired_combinations, 1, UINT32_MAX, "DesiredCombinations must be between 1 and 4294967295.");
    }

  std::cout << "  - Size                : " << uint32_t(size) << std::endl;
  std::cout << "  - Depth               : " << uint32_t(depth) << std::endl;
  std::cout << "  - Max number of points: " << uint32_t(max_number_of_points) << std::endl;
  std::cout << "  - Desired combinations: " << desired_combinations << std::endl;
  std::cout << "\n";

  /** Generate source matrices */

  /** Go trough all number of points */
  // const std::size_t number_of_threads = 1;
  const std::size_t number_of_threads = std::thread::hardware_concurrency() / 2;
  const uint32_t    total_cells       = size * size * depth;

  for(uint8_t i = 2; i <= max_number_of_points; ++i)
    {
      const uint64_t           possible_combinations           = gen::nCr(total_cells, i);
      const uint64_t           combinations_per_thread         = possible_combinations / number_of_threads;
      const uint64_t           desired_combinations_per_thread = desired_combinations / number_of_threads;

      const std::string        progress_bar_message            = "Combinations for " + std::to_string(i) + " points";

      utils::SyncProgressBar   progress_bar(desired_combinations, progress_bar_message);

      std::vector<std::thread> threads(number_of_threads);
      std::atomic<int64_t>     counter(0);

      for(std::size_t j = 0; j < number_of_threads; ++j)
        {
          auto worker = [&, j]() {
            const uint64_t    start_idx = j * combinations_per_thread;
            const uint64_t    end_idx   = (j == number_of_threads - 1) ? possible_combinations : start_idx + combinations_per_thread;
            const uint64_t    step      = (end_idx - start_idx) / desired_combinations_per_thread;

            gen::GeneratorItr itr(total_cells, i, step, start_idx, end_idx);
            gen::GeneratorItr itr_end(total_cells, i, step, end_idx, end_idx);

            for(; itr < itr_end; ++itr)
              {
                /** Go trough possible combinations */
                matrix::Matrix       source_matrix({ size, size, depth });
                std::vector<uint8_t> nodes_coordinates(i * 3);

                /** Make bounders */
                for(uint8_t z = 0; z < depth; ++z)
                  {
                    for(uint8_t x = 0; x < size; ++x)
                      {
                        source_matrix.set_at(types::TRACE_CELL, x, 0, z);
                      }

                    for(uint8_t x = 0; x < size; ++x)
                      {
                        source_matrix.set_at(types::TRACE_CELL, x, size - 1, z);
                      }

                    for(uint8_t y = 0; y < size; ++y)
                      {
                        source_matrix.set_at(types::TRACE_CELL, 0, y, z);
                      }

                    for(uint8_t y = 0; y < size; ++y)
                      {
                        source_matrix.set_at(types::TRACE_CELL, size - 1, y, z);
                      }

                    /** Left Top */
                    source_matrix.set_at(types::INTERSECTION_CELL, 0, 0, z);

                    /** Right Top */
                    source_matrix.set_at(types::INTERSECTION_CELL, size - 1, 0, z);

                    /** Left bottom  */
                    source_matrix.set_at(types::INTERSECTION_CELL, 0, size - 1, z);

                    /** Right bottom */
                    source_matrix.set_at(types::INTERSECTION_CELL, size - 1, size - 1, z);
                  }

                /** Fill the matrix */
                for(auto index : *itr)
                  {
                    const auto [c_x, c_y, c_z] = index_to_coordinates(index, size);

                    source_matrix.set_at(types::TERMINAL_CELL, c_x, c_y, c_z);
                    nodes_coordinates.emplace_back(c_x);
                    nodes_coordinates.emplace_back(c_y);
                    nodes_coordinates.emplace_back(c_z);

                    bool is_x_line_free = false;

                    for(uint8_t x = 0; x < size; ++x)
                      {
                        if(source_matrix.get_at(x, c_y, c_z) == 0)
                          {
                            is_x_line_free = true;
                            break;
                          }
                      }

                    if(is_x_line_free)
                      {
                        for(uint8_t x = 0; x < size; ++x)
                          {
                            const uint8_t& value = source_matrix.get_at(x, c_y, c_z);

                            if(value == 0)
                              {
                                source_matrix.set_at(types::TRACE_CELL, x, c_y, c_z);
                              }
                            else if(value != types::TERMINAL_CELL && c_y != 0 && c_y != size - 1)
                              {
                                source_matrix.set_at(types::INTERSECTION_CELL, x, c_y, c_z);
                              }
                          }
                      }

                    bool is_y_line_free = false;

                    for(uint8_t y = 0; y < size; ++y)
                      {
                        if(source_matrix.get_at(c_x, y, c_z) == 0)
                          {
                            is_y_line_free = true;
                            break;
                          }
                      }

                    if(is_y_line_free)
                      {
                        for(uint8_t y = 0; y < size; ++y)
                          {
                            const uint8_t& value = source_matrix.get_at(c_x, y, c_z);

                            if(value == 0)
                              {
                                source_matrix.set_at(types::TRACE_CELL, c_x, y, c_z);
                              }
                            else if(value != types::TERMINAL_CELL && c_x != 0 && c_x != size - 1)
                              {
                                source_matrix.set_at(types::INTERSECTION_CELL, c_x, y, c_z);
                              }
                          }
                      }

                    bool is_z_line_free = false;

                    for(uint8_t z = 0; z < depth; ++z)
                      {
                        if(source_matrix.get_at(c_x, c_y, z) == 0)
                          {
                            is_z_line_free = true;
                            break;
                          }
                      }

                    if(is_z_line_free)
                      {
                        for(uint8_t z = 0; z < depth; ++z)
                          {
                            const uint8_t& value = source_matrix.get_at(c_x, c_y, z);

                            if(value == 0)
                              {
                                source_matrix.set_at(types::TRACE_CELL, c_x, c_y, z);
                              }
                            else if(value != types::TERMINAL_CELL)
                              {
                                source_matrix.set_at(types::INTERSECTION_VIA_CELL, c_x, c_y, z);
                              }
                          }
                      }
                  }

                const auto [source_graph, nodes]                               = transform::matrix_to_graph(source_matrix);
                const std::vector<std::pair<uint32_t, uint32_t>> mst           = algorithms::dijkstra_kruskal(source_graph);
                const matrix::Matrix                             target_matrix = transform::mst_to_matrix({ size, size, depth }, mst, nodes);

                counter.fetch_add(1);

                const std::string matrix_name = "s" + std::to_string(size) + "_d" + std::to_string(depth) + "_p" + std::to_string(i) + "_n" + std::to_string(counter) + ".npy";

                numpy::save_as<uint8_t>(source_dir / matrix_name, reinterpret_cast<char*>(source_matrix.data()), { depth, size, size });
                numpy::save_as<uint8_t>(target_dir / matrix_name, reinterpret_cast<const char*>(target_matrix.data()), { depth, size, size });
                numpy::save_as<uint8_t>(nodes_dir / matrix_name, reinterpret_cast<const char*>(nodes_coordinates.data()), { max_number_of_points, 3 });

                progress_bar.step();
              }
          };

          threads[j] = std::thread(worker);
        }

      for(auto& thread : threads)
        {
          thread.join();
        }
    }

  return 0;
}