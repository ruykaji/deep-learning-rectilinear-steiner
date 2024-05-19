#ifndef __CMD_HPP__
#define __CMD_HPP__

#include <chrono>
#include <iostream>

namespace gen
{

namespace details
{

constexpr char PROGRAM_NAME[] = "Matrix Generator";
constexpr char VERSION[] = "1.0.0";
constexpr char AUTHOR[] = "Ruykaji";
constexpr char COPYRIGHT_YEAR[] = "2024";

void
show_version()
{
  std::cout << "\n\033[1;96m";
  std::cout << "╔══════════════════════════════════════════════╗\n";
  std::cout << "║       \033[1;93m" << PROGRAM_NAME << " - Version " << VERSION << "\033[1;96m       ║\n";
  std::cout << "╠══════════════════════════════════════════════╣\n";
  std::cout << "║       \033[0;36mCompiled with C++" << (__cplusplus / 100) % 100 << " (C++" << (__cplusplus / 100 % 100) - 1
            << ")\033[1;96m            ║\n";
  std::cout << "║       \033[0;36mWritten by " << AUTHOR << ", " << COPYRIGHT_YEAR << "\033[1;96m               ║\n";
  std::cout << "╚══════════════════════════════════════════════╝\n";
  std::cout << "\033[0m\n";
}

void
show_usage(std::string_view name)
{
  std::cerr << "Usage: " << name << " <options>\n\n"
            << "Options:\n"
            << "  --rectilinear <flag> Is rectilinear algorithm\n"
            << "  --path <string>      Path to generated data\n"
            << "  --skip <number>      Number of cases to skip\n"
            << "  --width <number>     Width of each matrix\n"
            << "  --height <number>    Height of each matrix\n"
            << "  --points <number>    Number of terminal points per matrix\n"
            << "  --help               Show this help message\n"
            << "  --version            Show version information\n";
}

} // namespace details

struct Settings
{
  std::string path = "";
  uint32_t width = 32;
  uint32_t height = 32;
  std::size_t skip = 55;
  std::size_t points = 10;
};

Settings
arg_parse(int32_t argc, char* const argv[])
{
  static auto is_digit = [](char const* t_char) {
    for(; *t_char != '\0'; ++t_char)
      if(!std::isdigit(*t_char))
        return false;
    return true;
  };

  Settings s__;

  for(int i = 1; i < argc; ++i)
    {
      std::string arg = argv[i];
      if(arg == "--help")
        {
          details::show_usage(argv[0]);
          exit(EXIT_SUCCESS);
        }
      else if(arg == "--version")
        {
          details::show_version();
          exit(EXIT_SUCCESS);
        }
      else if(arg == "--path")
        {
          if(i + 1 < argc)
            s__.path = argv[++i];
          else
            {
              std::cerr << "--path option requires one string argument." << std::endl;
              exit(EXIT_FAILURE);
            }
        }
      else if(arg == "--skip")
        {
          if(i + 1 < argc && is_digit(argv[i + 1]))
            s__.skip = std::stoi(argv[++i]);
          else
            {
              std::cerr << "--skip option requires one numeric argument." << std::endl;
              exit(EXIT_FAILURE);
            }
        }
      else if(arg == "--width")
        {
          if(i + 1 < argc && is_digit(argv[i + 1]))
            s__.width = std::stoi(argv[++i]);
          else
            {
              std::cerr << "--width option requires one numeric argument." << std::endl;
              exit(EXIT_FAILURE);
            }
        }
      else if(arg == "--height")
        {
          if(i + 1 < argc && is_digit(argv[i + 1]))
            s__.height = std::stoi(argv[++i]);
          else
            {
              std::cerr << "--height option requires one numeric argument." << std::endl;
              exit(EXIT_FAILURE);
            }
        }
      else if(arg == "--points")
        {
          if(i + 1 < argc && is_digit(argv[i + 1]))
            s__.points = std::stoi(argv[++i]);
          else
            {
              std::cerr << "--points option requires one numeric argument." << std::endl;
              exit(EXIT_FAILURE);
            }
        }
      else
        {
          std::cerr << "Unknown option: " << arg << std::endl;
          exit(EXIT_FAILURE);
        }
    }

  details::show_version();

  return s__;
}

class Progress_bar
{
public:
  Progress_bar(std::size_t total, int32_t bar_width) : m_total_steps(total), m_current_step(0), m_start_time(), m_width(bar_width)
  {
    m_start_time = std::chrono::steady_clock::now(); // Record start time
  }

  ~Progress_bar()
  {
    std::cout << std::endl;
  }

  void
  step()
  {
    if(m_current_step < m_total_steps)
      ++m_current_step;
    update();
  }

  void
  update()
  {
    auto now__ = std::chrono::steady_clock::now();
    auto elapsed__ = std::chrono::duration_cast<std::chrono::seconds>(now__ - m_start_time).count();
    double progress__ = static_cast<double>(m_current_step) / m_total_steps;
    int32_t pos__ = static_cast<int32_t>(m_width * progress__);

    std::cout << "\r";
    std::cout << int32_t(progress__ * 100.0) << "% " << "[";

    for(int32_t i = 0; i < pos__; ++i)
      std::cout << "=";
    for(int32_t i = pos__; i < m_width; ++i)
      std::cout << " ";

    std::cout << "] " << std::setw(3) << m_current_step << "/" << m_total_steps << " [" << elapsed__ << "s]" << std::flush;

    if(progress__ == 1.0)
      std::cout << std::endl;
  }

  std::size_t
  get_step()
  {
    return m_current_step;
  }

private:
  std::size_t m_total_steps;
  std::size_t m_current_step;
  std::chrono::steady_clock::time_point m_start_time;
  int32_t m_width;
};

} // namespace gen

#endif