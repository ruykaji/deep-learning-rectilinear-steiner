#include <iostream>
#include <queue>

#include "Include/Generator.hpp"
#include "Include/Graph.hpp"
#include "Include/Ini.hpp"
#include "Include/Matrix.hpp"
#include "Include/Numpy.hpp"

constexpr uint8_t PATH_CELL              = 1;

constexpr uint8_t INTERSECTION_NETS_CELL = 9;

constexpr uint8_t TRACE_CELL             = 5;
constexpr uint8_t TERMINAL_CELL          = 4;
constexpr uint8_t INTERSECTION_CELL      = 3;
constexpr uint8_t INTERSECTION_VIA_CELL  = 2;

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

template <typename Itr>
using MRequires_input_itr = std::enable_if<std::is_convertible_v<typename Itr::iterator_category, std::input_iterator_tag>>;

template <typename Itr, typename = MRequires_input_itr<Itr>, typename Tp>
std::size_t
find_index(Itr first, Itr last, const Tp& value)
{
  std::size_t idx = 0;

  for(; first != last; ++first, ++idx)
    {
      if(*first == value)
        return idx;
    }

  return idx;
};

graph::Graph
make_graph(const matrix::Matrix& matrix, std::vector<std::tuple<uint8_t, uint8_t, uint8_t>>& nodes)
{
  matrix::Shape                         shape = matrix.shape();
  graph::Graph                          graph;

  // Initialize with top left corner as it always is intersection node
  std::tuple<uint8_t, uint8_t, uint8_t> itr = { 0, 0, 0 };
  nodes.push_back(itr);

  graph.place_node();

  std::queue<std::tuple<uint8_t, uint8_t, uint8_t>> queue{};
  queue.push(itr);

  while(!queue.empty())
    {
      std::tuple<uint8_t, uint8_t, uint8_t> front = queue.front();
      queue.pop();

      const std::size_t source_idx = find_index(nodes.begin(), nodes.end(), front);

      // Searching for intersection or terminal node in x direction.
      {
        auto [x, y, z] = front;

        while(x != shape.m_x - 1)
          {
            x += 1;

            const std::size_t dest_idx = find_index(nodes.begin(), nodes.end(), std::make_tuple(x, y, z));

            if(matrix.get_at(x, y, z) == INTERSECTION_VIA_CELL || matrix.get_at(x, y, z) == INTERSECTION_CELL || matrix.get_at(x, y, z) == TERMINAL_CELL)
              {
                // If node haven't been visited then add it to the queue
                if(dest_idx == nodes.size())
                  {
                    nodes.push_back(std::make_tuple(x, y, z));
                    graph.place_node();

                    queue.push(std::make_tuple(x, y, z));
                  }

                const uint32_t distance = x - std::get<0>(front);

                // Connect source and destination nodes

                graph.add_edge(distance, dest_idx, source_idx);

                if(matrix.get_at(x, y, z) == TERMINAL_CELL)
                  {
                    graph.add_terminal(dest_idx);
                  }
              }
          }
      }

      {
        auto [x, y, z] = front;

        while(x != 0)
          {
            x -= 1;

            const std::size_t dest_idx = find_index(nodes.begin(), nodes.end(), std::make_tuple(x, y, z));

            if(matrix.get_at(x, y, z) == INTERSECTION_VIA_CELL || matrix.get_at(x, y, z) == INTERSECTION_CELL || matrix.get_at(x, y, z) == TERMINAL_CELL)
              {
                // If node haven't been visited then add it to the queue
                if(dest_idx == nodes.size())
                  {
                    nodes.push_back(std::make_tuple(x, y, z));
                    graph.place_node();

                    queue.push(std::make_tuple(x, y, z));
                  }

                const uint32_t distance = std::get<0>(front) - x;

                // Connect source and destination nodes

                graph.add_edge(distance, dest_idx, source_idx);

                if(matrix.get_at(x, y, z) == TERMINAL_CELL)
                  {
                    graph.add_terminal(dest_idx);
                  }
              }
          }
      }

      // Searching for intersection or terminal node in y direction.
      {
        auto [x, y, z] = front;

        while(y != shape.m_y - 1)
          {
            y += 1;

            const std::size_t dest_idx = find_index(nodes.begin(), nodes.end(), std::make_tuple(x, y, z));

            if(matrix.get_at(x, y, z) == INTERSECTION_VIA_CELL || matrix.get_at(x, y, z) == INTERSECTION_CELL || matrix.get_at(x, y, z) == TERMINAL_CELL)
              {
                // If node haven't been visited then add it to the queue
                if(dest_idx == nodes.size())
                  {
                    nodes.push_back(std::make_tuple(x, y, z));
                    graph.place_node();

                    queue.push(std::make_tuple(x, y, z));
                  }

                const uint32_t distance = y - std::get<1>(front);

                // Connect source and destination nodes

                graph.add_edge(distance, dest_idx, source_idx);

                if(matrix.get_at(x, y, z) == TERMINAL_CELL)
                  {
                    graph.add_terminal(dest_idx);
                  }
              }
          }
      }

      {
        auto [x, y, z] = front;

        while(y != 0)
          {
            y -= 1;

            const std::size_t dest_idx = find_index(nodes.begin(), nodes.end(), std::make_tuple(x, y, z));

            if(matrix.get_at(x, y, z) == INTERSECTION_VIA_CELL || matrix.get_at(x, y, z) == INTERSECTION_CELL || matrix.get_at(x, y, z) == TERMINAL_CELL)
              {
                // If node haven't been visited then add it to the queue
                if(dest_idx == nodes.size())
                  {
                    nodes.push_back(std::make_tuple(x, y, z));
                    graph.place_node();

                    queue.push(std::make_tuple(x, y, z));
                  }

                const uint32_t distance = std::get<1>(front) - y;

                // Connect source and destination nodes

                graph.add_edge(distance, dest_idx, source_idx);

                if(matrix.get_at(x, y, z) == TERMINAL_CELL)
                  {
                    graph.add_terminal(dest_idx);
                  }
              }
          }
      }
      // Searching for intersection or terminal node in z direction.
      {
        auto [x, y, z] = front;

        while(z != shape.m_z - 1)
          {
            z += 1;

            const std::size_t dest_idx = find_index(nodes.begin(), nodes.end(), std::make_tuple(x, y, z));

            if(matrix.get_at(x, y, z) == INTERSECTION_VIA_CELL || matrix.get_at(x, y, z) == INTERSECTION_CELL || matrix.get_at(x, y, z) == TERMINAL_CELL)
              {
                // If node haven't been visited then add it to the queue
                if(dest_idx == nodes.size())
                  {
                    nodes.push_back(std::make_tuple(x, y, z));
                    graph.place_node();

                    queue.push(std::make_tuple(x, y, z));
                  }

                const uint32_t distance = z - std::get<2>(front);

                // Connect source and destination nodes

                graph.add_edge(distance, dest_idx, source_idx);

                if(matrix.get_at(x, y, z) == TERMINAL_CELL)
                  {
                    graph.add_terminal(dest_idx);
                  }
              }
          }
      }

      {
        auto [x, y, z] = front;

        while(z != 0)
          {
            z -= 1;

            const std::size_t dest_idx = find_index(nodes.begin(), nodes.end(), std::make_tuple(x, y, z));

            if(matrix.get_at(x, y, z) == INTERSECTION_VIA_CELL || matrix.get_at(x, y, z) == INTERSECTION_CELL || matrix.get_at(x, y, z) == TERMINAL_CELL)
              {
                // If node haven't been visited then add it to the queue
                if(dest_idx == nodes.size())
                  {
                    nodes.push_back(std::make_tuple(x, y, z));
                    graph.place_node();

                    queue.push(std::make_tuple(x, y, z));
                  }

                const uint32_t distance = std::get<2>(front) - z;

                // Connect source and destination nodes

                graph.add_edge(distance, dest_idx, source_idx);

                if(matrix.get_at(x, y, z) == TERMINAL_CELL)
                  {
                    graph.add_terminal(dest_idx);
                  }
              }
          }
      }
    }

  return graph;
}

matrix::Matrix
make_matrix(const matrix::Shape shape, const std::vector<std::pair<uint32_t, uint32_t>>& mst, const std::vector<std::tuple<uint8_t, uint8_t, uint8_t>>& nodes)
{
  matrix::Matrix matrix(shape);

  for(auto [first, second] : mst)
    {
      auto [f_x, f_y, f_z] = nodes[first - 1];
      auto [s_x, s_y, s_z] = nodes[second - 1];

      if(s_x <= f_x && s_y <= f_y && s_z <= f_z)
        {
          std::swap(f_x, s_x);
          std::swap(f_y, s_y);
          std::swap(f_z, s_z);
        }

      if(f_x == s_x && f_y == s_y)
        {
          for(; f_z <= s_z; f_z += 1)
            {
              matrix.set_at(PATH_CELL, f_x, f_y, f_z);
            }
        }
      else if(f_x == s_x)
        {
          for(; f_y <= s_y; f_y += 1)
            {
              matrix.set_at(PATH_CELL, f_x, f_y, f_z);
            }
        }
      else if(f_y == s_y)
        {
          for(; f_x <= s_x; f_x += 1)
            {
              matrix.set_at(PATH_CELL, f_x, f_y, f_z);
            }
        }
    }

  return matrix;
}

} // namespace

int
main(int argc, char* argv[])
{
  ini::Config config = ini::parse("./config.ini");

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

  for(const auto& dir : { source_dir, target_dir })
    {
      if(std::filesystem::exists(dir))
        {
          std::filesystem::remove_all(dir);
        }

      std::filesystem::create_directory(dir);
    }

  std::cout << "  - Source directory: " << source_dir << std::endl;
  std::cout << "  - Target directory: " << target_dir << std::endl;
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
  for(uint8_t i = 2; i <= max_number_of_points; ++i)
    {
      const uint32_t total_cells           = size * size * depth;
      const uint64_t possible_combinations = gen::nCr(total_cells, i);
      const uint32_t step                  = possible_combinations / desired_combinations;

      gen::Generator generator(total_cells, i, step);
      int64_t        counter = 0;

      for(const auto combinations : generator)
        {
          /** Go trough possible combinations */
          matrix::Matrix source_matrix({ size, size, depth });

          /** Make bounders */
          for(uint8_t z = 0; z < depth; ++z)
            {
              for(uint8_t x = 0; x < size; ++x)
                {
                  source_matrix.set_at(TRACE_CELL, x, 0, z);
                }

              for(uint8_t x = 0; x < size; ++x)
                {
                  source_matrix.set_at(TRACE_CELL, x, size - 1, z);
                }

              for(uint8_t y = 0; y < size; ++y)
                {
                  source_matrix.set_at(TRACE_CELL, 0, y, z);
                }

              for(uint8_t y = 0; y < size; ++y)
                {
                  source_matrix.set_at(TRACE_CELL, size - 1, y, z);
                }

              /** Left Top */
              source_matrix.set_at(INTERSECTION_CELL, 0, 0, z);

              /** Right Top */
              source_matrix.set_at(INTERSECTION_CELL, size - 1, 0, z);

              /** Left bottom  */
              source_matrix.set_at(INTERSECTION_CELL, 0, size - 1, z);

              /** Right bottom */
              source_matrix.set_at(INTERSECTION_CELL, size - 1, size - 1, z);
            }

          /** Fill the matrix */
          for(auto index : combinations)
            {
              const auto [c_x, c_y, c_z] = index_to_coordinates(index, size);

              source_matrix.set_at(TERMINAL_CELL, c_x, c_y, c_z);

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
                          source_matrix.set_at(TRACE_CELL, x, c_y, c_z);
                        }
                      else if(value != TERMINAL_CELL && c_y != 0 && c_y != size - 1)
                        {
                          source_matrix.set_at(INTERSECTION_CELL, x, c_y, c_z);
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
                          source_matrix.set_at(TRACE_CELL, c_x, y, c_z);
                        }
                      else if(value != TERMINAL_CELL && c_x != 0 && c_x != size - 1)
                        {
                          source_matrix.set_at(INTERSECTION_CELL, c_x, y, c_z);
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
                          source_matrix.set_at(TRACE_CELL, c_x, c_y, z);
                        }
                      else if(value != TERMINAL_CELL)
                        {
                          source_matrix.set_at(INTERSECTION_VIA_CELL, c_x, c_y, z);
                        }
                    }
                }
            }

          std::vector<std::tuple<uint8_t, uint8_t, uint8_t>> nodes; /** All nodes in graph */

          const graph::Graph                                 source_graph  = make_graph(source_matrix, nodes);
          const std::vector<std::pair<uint32_t, uint32_t>>   mst           = source_graph.mst();
          const matrix::Matrix                               target_matrix = make_matrix({ size, size, depth }, mst, nodes);

          const std::string                                  matrix_name   = "s" + std::to_string(size) + "_d" + std::to_string(depth) + "_p" + std::to_string(i) + "_n" + std::to_string(++counter) + ".npy";

          numpy::save_as<uint8_t>(source_dir / matrix_name, reinterpret_cast<char*>(source_matrix.data()), { depth, size, size });
          numpy::save_as<uint8_t>(target_dir / matrix_name, reinterpret_cast<const char*>(target_matrix.data()), { depth, size, size });
        }
    }

  return 0;
}