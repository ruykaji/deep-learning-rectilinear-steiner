#include <iostream>
#include <random>
#include <stdexcept>
#include <string.h>
#include <vector>

#include "process.hpp"

struct Settings
{
  uint32_t data;
  uint32_t width;
  uint32_t height;
  uint32_t points;
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

  Settings default__{ 1, 32, 32, 5 };

  if(argc > 1)
    {
      for(int32_t i = 1; i < argc; i += 2)
        {
          if(!strcmp(argv[i], "--length") || !strcmp(argv[i], "-l"))
            {
              if(i + 1 < argc && is_digit(argv[i + 1]))
                default__.data = std::stoi(argv[i + 1]);
              else
                {
                  std::cout << "--length | -l expects 1 argument: " << argv[i] << '\n';
                  exit(EXIT_FAILURE);
                }
            }
          else if(!strcmp(argv[i], "--width") || !strcmp(argv[i], "-w"))
            {
              if(i + 1 < argc && is_digit(argv[i + 1]))
                default__.width = std::stoi(argv[i + 1]);
              else
                {
                  std::cout << "--width | -w expects 1 argument: " << argv[i] << '\n';
                  exit(EXIT_FAILURE);
                }
            }
          else if(!strcmp(argv[i], "--height") || !strcmp(argv[i], "-h"))
            {
              if(i + 1 < argc && is_digit(argv[i + 1]))
                default__.height = std::stoi(argv[i + 1]);
              else
                {
                  std::cout << "--height | -h expects 1 argument: " << argv[i] << '\n';
                  exit(EXIT_FAILURE);
                }
            }
          else if(!strcmp(argv[i], "--points") || !strcmp(argv[i], "-p"))
            {
              if(i + 1 < argc && is_digit(argv[i + 1]))
                default__.points = std::stoi(argv[i + 1]);
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

  return default__;
}

int
main(int argc, char* const argv[])
{

  Settings s__ = init(argc, argv);

  for(uint32_t i = 0; i < s__.data; ++i)
    {
      gen::Matrix<uint32_t> matrix__({ s__.width, s__.height });
      std::vector<gen::Matrix_index> terminals__(s__.points);

      for(uint32_t j = 0; j < s__.points; ++j)
        {
          terminals__.emplace_back(gen::random_point(s__.width, s__.width));
          matrix__[terminals__.back()] = 1;
        }

      gen::preprocess(matrix__, terminals__);

      std::cout << matrix__;
    }

  return 0;
}