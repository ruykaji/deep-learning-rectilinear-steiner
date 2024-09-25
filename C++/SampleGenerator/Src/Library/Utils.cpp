#include <iostream>
#include <thread>

#include "Include/Utilis.hpp"

namespace utils
{

void
show_progress(std::string message, const bool& done, const uint32_t& progress, uint32_t total_steps, std::mutex& mutex, std::condition_variable& cv)
{
  std::unique_lock<std::mutex> lock(mutex);

  while(!done)
    {
      uint32_t percent = (total_steps == 0) ? 100 : (progress * 100) / total_steps;

      std::cout << "\r" << message << ": [" << std::string(percent / 2, '=') << std::string(50 - percent / 2, ' ') << "] " << percent << "%";
      std::cout.flush();

      cv.wait(lock);
    }

  std::cout << "\r" << message << ": [" << std::string(50, '=') << "] 100%\n";
}

} // namespace utils
