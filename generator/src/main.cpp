#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <vector>

#include "cmd.hpp"
#include "mst.hpp"
#include "process.hpp"
#include "utility.hpp"

std::size_t
factorial(uint32_t n)
{
  std::size_t result = 1;
  for(uint32_t i__ = 2; i__ <= n; ++i__)
    result *= i__;
  return result;
}

std::size_t
combinations(uint32_t n, uint32_t k)
{
  if(k > n)
    return 0;

  if(k == 0 || k == n)
    return 1;

  k = std::min(k, n - k);
  std::size_t c = 1;

  for(uint32_t i__ = 0; i__ < k; ++i__)
    c = c * (n - i__) / (i__ + 1);

  return c;
}

std::size_t
total_combinations(uint32_t rows, uint32_t cols, std::size_t max_points)
{
  int32_t total_points = rows * cols;
  std::size_t total = 0;

  for(uint32_t i__ = 1; i__ <= max_points; ++i__)
    total += combinations(total_points, i__);

  return total;
}

// Simple constrains check.
inline bool
is_agreed(const std::vector<gen::Index>& terminals)
{
  for(std::size_t i = 0, end = terminals.size(); i < end; ++i)
    {
      for(std::size_t j = i + 1; j < end; ++j)
        {
          if(terminals[i] == terminals[j])
            return false;

          if(std::abs(static_cast<double>(terminals[i].x) - terminals[j].x) < 2.0)
            return false;

          if(std::abs(static_cast<double>(terminals[i].y) - terminals[j].y) < 2.0)
            return false;
        }
    }

  return true;
}

std::vector<std::vector<gen::Index>>
split_by_nets(std::vector<gen::Index> vec)
{
  std::vector<std::vector<gen::Index>> nets__{};

  if(vec.size() > 3)
    {
      int64_t seed = std::chrono::system_clock::now().time_since_epoch().count();
      std::shuffle(vec.begin(), vec.end(), std::default_random_engine(seed));

      for(std::size_t i__ = 0, end__ = vec.size(); i__ < end__; i__ += 2)
        {
          if(i__ + 2 < end__)
            {
              nets__.push_back({ vec.begin() + i__, vec.begin() + i__ + 2 });
            }
          else if(i__ < end__ && end__ - i__ > 2)
            {
              nets__.push_back({ vec.begin() + i__, vec.end() });
            }
        }
    }
  else
    {
      nets__.push_back(vec);
    }

  return nets__;
}

int
main(int argc, char* const argv[])
{
  gen::Settings s__ = gen::arg_parse(argc, argv);

  // Setting up save directory.
  std::filesystem::path root_path__{};

  if(s__.path.empty())
    root_path__ = std::filesystem::current_path() / "data";
  else
    root_path__ = s__.path;

  std::filesystem::remove_all(root_path__);
  std::filesystem::create_directory(root_path__);

  // Set shape of matrices that will be generated.
  gen::Process::set_shape({ s__.width, s__.height, 3 });

  // Initialize progress bar.
  gen::Progress_bar progress_bar__{ total_combinations(s__.width - 2, s__.height - 2, s__.points - 1), 75 };

  std::cout << "\nGeneration in progress...\n" << std::flush;

  std::size_t counter__ = 0;
  std::queue<std::vector<gen::Index>> combinations__;
  combinations__.push({ gen::Index{ 1, 1, 0 } });

  while(!combinations__.empty())
    {
      std::vector<gen::Index> current__ = combinations__.front();
      combinations__.pop();

      uint32_t start_x = current__.empty() ? 1 : current__.back().x;
      uint32_t start_y = current__.empty() ? 1 : current__.back().y + 1;

      for(uint32_t x__ = start_x; x__ < s__.width - 1; ++x__)
        {
          for(uint32_t y__ = (x__ == start_x ? start_y : 1); y__ < s__.height - 1; ++y__)
            {
              std::vector<gen::Index> new_combination__ = current__;
              new_combination__.emplace_back(x__, y__);

              if(is_agreed(new_combination__))
                {
                  if(new_combination__.size() != s__.points && counter__ % s__.skip == 0)
                    {
                      bool skip__ = false;
                      counter__ = 0;
                      combinations__.push(new_combination__);

                      std::vector<std::vector<gen::Index>> nets__ = split_by_nets(new_combination__);
                      std::vector<gen::Matrix> nets_matrices__{};

                      gen::Matrix output_matrix__{ gen::Process::s_shape };

                      for(std::size_t i__ = 0, end__ = nets__.size(); i__ < end__; ++i__)
                        {
                          auto [matrix__, is_made__] = gen::Process::propagate(output_matrix__, nets__, i__);

                          if(!is_made__)
                            {
                              skip__ = true;
                              break;
                            }

                          gen::Graph graph__ = gen::Process::make_graph(matrix__, nets__[i__][0]);
                          gen::MST mst__ = gen::make_mst(graph__);

                          output_matrix__ += gen::Process::make_matrix(mst__);
                          nets_matrices__.emplace_back(matrix__);
                        }

                      if(skip__)
                        {
                          continue;
                        }

                      gen::Matrix input_matrix = gen::Process::map_nets(nets_matrices__);

                      // Save both matrices.
                      std::ofstream out__{ root_path__ / ("data_" + std::to_string(progress_bar__.get_step() + 1)) };

                      if(out__.is_open())
                        {
                          out__ << "SHAPE: " << uint32_t(3) << ' ' << s__.width << ' ' << s__.height << '\n'  << std::flush;
                          out__ << "NETS: ";

                          for(const auto& n__ : nets__)
                            {
                              out__ << "[ ";

                              for(const auto& t__ : n__)
                                {
                                  out__ << t__.y << ' ' << t__.x << ' ';
                                }

                              out__ << "] ";
                            }

                          out__ << '\n' << std::flush;

                          out__ << "INPUT:\n" << std::flush;
                          out__ << input_matrix;

                          out__ << "OUTPUT:\n" << std::flush;
                          out__ << output_matrix__;
                        }

                      out__.close();
                      progress_bar__.step();
                    }

                  ++counter__;
                }
            }

          start_y = 0;
        }
    }
  return 0;
}