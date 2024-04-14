#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <string.h>
#include <vector>

#include "mst.hpp"
#include "process.hpp"
#include "utility.hpp"

struct Settings
{
  uint32_t width = 16;
  uint32_t height = 16;
  std::size_t data = 1;
  std::size_t points = 5;
};

Settings
init(int32_t argc, char* const argv[])
{
  static auto is_digit = [](char const* t_char) {
    for(; *t_char != '\0'; ++t_char)
      if(!std::isdigit(*t_char))
        return false;
    return true;
  };

  Settings s__;

  if(argc > 1)
    {
      for(int32_t i = 1; i < argc; i += 2)
        {
          if(!strcmp(argv[i], "--length") || !strcmp(argv[i], "-l"))
            {
              if(i + 1 < argc && is_digit(argv[i + 1]))
                s__.data = std::stoi(argv[i + 1]);
              else
                {
                  std::cout << "--length | -l expects 1 argument: " << argv[i] << '\n';
                  exit(EXIT_FAILURE);
                }
            }
          else if(!strcmp(argv[i], "--width") || !strcmp(argv[i], "-w"))
            {
              if(i + 1 < argc && is_digit(argv[i + 1]))
                s__.width = std::stoi(argv[i + 1]);
              else
                {
                  std::cout << "--width | -w expects 1 argument: " << argv[i] << '\n';
                  exit(EXIT_FAILURE);
                }
            }
          else if(!strcmp(argv[i], "--height") || !strcmp(argv[i], "-h"))
            {
              if(i + 1 < argc && is_digit(argv[i + 1]))
                s__.height = std::stoi(argv[i + 1]);
              else
                {
                  std::cout << "--height | -h expects 1 argument: " << argv[i] << '\n';
                  exit(EXIT_FAILURE);
                }
            }
          else if(!strcmp(argv[i], "--points") || !strcmp(argv[i], "-p"))
            {
              if(i + 1 < argc && is_digit(argv[i + 1]))
                s__.points = std::stoi(argv[i + 1]);
              else
                {
                  std::cout << "--points | -p expects 1 argument: " << argv[i] << '\n';
                  exit(EXIT_FAILURE);
                }
            }
          else
            {
              std::cout << "Unexpected inline argument: " << argv[i] << '\n';
              exit(EXIT_FAILURE);
            }
        }
    }

  return s__;
}

int
main(int argc, char* const argv[])
{
  Settings s__ = init(argc, argv);

  std::filesystem::path root_path__{ std::filesystem::current_path() / "data" };
  std::filesystem::remove_all(root_path__);
  std::filesystem::create_directory(root_path__);

  gen::Process::set_shape({ s__.width, s__.height });

  auto counter_start__ = std::chrono::high_resolution_clock::now();

  for(uint32_t i = 0; i < s__.data; ++i)
    {
      std::vector<gen::Index> terminals__{ s__.points, gen::Index{} };

      for(uint32_t j = 0; j < s__.points; ++j)
        terminals__[j] = gen::Index{ gen::random(1U, s__.width - 2), gen::random(1U, s__.height - 2) };

      gen::Matrix matrix__ = gen::Process::propagate(terminals__);
      gen::Graph graph__ = gen::Process::make_graph(matrix__);

      gen::MST mst__ = gen::make_mst(graph__);

      gen::Matrix mst_matrix__ = gen::Process::make_matrix(mst__);

      std::ofstream out__{ root_path__ / ("data_" + std::to_string(i + 1)) };

      if(out__.is_open())
        {
          out__ << "SHAPE: " << s__.width << ' ' << s__.height << '\n' << std::flush;

          out__ << "INPUT:\n" << std::flush;
          out__ << matrix__;

          out__ << "OUTPUT:\n" << std::flush;
          out__ << mst_matrix__;
        }

      out__.close();
    }

  auto counter_end__ = std::chrono::high_resolution_clock::now();

  auto duration__ = std::chrono::duration_cast<std::chrono::seconds>(counter_end__ - counter_start__);
  std::cout << "Execution time: " << duration__.count() << " seconds" << std::endl;

  return 0;
}