#include <iostream>
#include <thread>

#include "Include/Utilis.hpp"

namespace utils
{

SyncProgressBar::SyncProgressBar(const std::size_t total_steps, const std::string_view message)
    : m_total_steps(total_steps), m_message(message), m_progress(0), m_done(false), m_mutex() {};

SyncProgressBar::~SyncProgressBar()
{
  std::lock_guard lock(m_mutex);
  std::cout << "\r" << m_message << ": [" << std::string(50, '=') << "] 100%\n";
  std::cout.flush();
}

void
SyncProgressBar::step()
{
  std::lock_guard lock(m_mutex);

  if(!m_done)
    {
      ++m_progress;

      if(m_progress < m_total_steps)
        {
          uint32_t percent = (m_progress * 100) / m_total_steps;

          std::cout << "\r" << m_message << ": [" << std::string(percent / 2, '=') << std::string(50 - percent / 2, ' ') << "] " << percent << "%";
          std::cout.flush();
        }
      else
        {
          m_done = true;
        }
    }
}

} // namespace utils
