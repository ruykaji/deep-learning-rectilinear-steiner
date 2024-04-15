#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <vector>

#include "cmd.hpp"
#include "mst.hpp"
#include "process.hpp"
#include "utility.hpp"

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

  // Vector that saves all generated matrices for further collision check.
  std::vector<gen::Matrix> all_matrix__{};

  // Set shape of matrices that will be generated.
  gen::Process::set_shape({ s__.width, s__.height });

  // Initialize progress bar.
  gen::Progress_bar progress_bar__{ s__.length, 75 };

  std::cout << "\nGeneration in progress...\n" << std::flush;

  for(uint32_t i = 0; i < s__.length; ++i)
    {
      // Generate new terminals.
      std::vector<gen::Index> terminals__{ s__.points, gen::Index{} };

      for(uint32_t j = 0; j < s__.points; ++j)
        terminals__[j] = gen::Index{ gen::random(1U, s__.width - 2), gen::random(1U, s__.height - 2) };

      // Process matrices.
      gen::Matrix matrix__ = gen::Process::propagate(terminals__);
      gen::Graph graph__ = gen::Process::make_graph(matrix__);

      gen::MST mst__ = gen::make_mst(graph__);

      gen::Matrix mst_matrix__ = gen::Process::make_matrix(mst__);

      // Save both matrices.
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
      all_matrix__.push_back(std::move(matrix__));
      progress_bar__.step();
    }

  // Find collisions between all created matrices.

  std::cout << "\nCollecting collision report...\n";

  std::vector<std::size_t> collisions__(10, 0);

  for(std::size_t i__ = 0, end__ = all_matrix__.size(); i__ < end__; ++i__)
    {
      for(std::size_t j__ = i__ + 1; j__ < end__; ++j__)
        {
          double similarity__ = (all_matrix__[i__] / all_matrix__[j__]) * 100.0;

          int32_t index = std::min(static_cast<int32_t>(similarity__ / 10), 9);
          ++collisions__[index];
        }
    }

  std::cout << "\n\033[1;32m";
  std::cout << "┏━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\n";
  std::cout << "┃     Collision Report      ┃\n";
  std::cout << "┣━━━━━━━━━━━━━┳━━━━━━━━━━━━━┫\n";
  std::cout << "┃ \033[1;36mRange       \033[1;32m┃ \033[1;36mCount       \033[1;32m┃\n";
  std::cout << "┣━━━━━━━━━━━━━╋━━━━━━━━━━━━━┫\n";

  for(int i = 0; i < 10; ++i)
    {
      std::cout << "┃ \033[0m" << std::left << std::setw(12) << std::to_string((i * 10) + 1) + "-" + std::to_string((i + 1) * 10) + "%"
                << "\033[1;32m┃ \033[0m" << std::right << std::setw(11) << collisions__[i] << " \033[1;32m┃\n";
    }

  std::cout << "┗━━━━━━━━━━━━━┻━━━━━━━━━━━━━┛\n" << std::flush;

  return 0;
}