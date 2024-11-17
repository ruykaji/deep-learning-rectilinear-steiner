#include <ctime>
#include <iomanip>
#include <iostream>
#include <thread>

#include "Include/Utilis.hpp"

namespace utils
{

SyncProgressBar::SyncProgressBar(const std::size_t total_steps, const std::string_view message)
    : m_total_steps(total_steps), m_message(message), m_progress(0), m_done(false), m_mutex()
{
  m_start_time = std::chrono::steady_clock::now();
};

SyncProgressBar::~SyncProgressBar()
{
  auto            now             = std::chrono::steady_clock::now();
  auto            elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(now - m_start_time).count();

  int32_t         hours           = elapsed_seconds / 3600;
  int32_t         minutes         = (elapsed_seconds % 3600) / 60;
  int32_t         seconds         = elapsed_seconds % 60;

  std::lock_guard lock(m_mutex);
  std::cout << "\r" << m_message << ": [" << std::string(50, '=')
            << "] 100% " << m_progress << "/" << m_total_steps
            << " [" << std::setw(2) << std::setfill('0') << hours << ":"
            << std::setw(2) << std::setfill('0') << minutes << ":"
            << std::setw(2) << std::setfill('0') << seconds << "]\n"
            << std::flush;
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
          uint32_t percent           = (m_progress * 100) / m_total_steps;

          auto     now               = std::chrono::steady_clock::now();
          auto     elapsed_seconds   = std::chrono::duration_cast<std::chrono::seconds>(now - m_start_time).count();

          int32_t  remaining_seconds = m_progress > 0 ? static_cast<int32_t>((elapsed_seconds * (m_total_steps - m_progress)) / m_progress) : 0;
          int32_t  hours             = remaining_seconds / 3600;
          int32_t  minutes           = (remaining_seconds % 3600) / 60;
          int32_t  seconds           = remaining_seconds % 60;

          std::cout << "\r" << m_message << ": [" << std::string(percent / 2, '=') << std::string(50 - percent / 2, ' ') << "] "
                    << percent << "% " << m_progress << "/" << m_total_steps
                    << " [" << std::setw(2) << std::setfill('0') << hours << ":"
                    << std::setw(2) << std::setfill('0') << minutes << ":"
                    << std::setw(2) << std::setfill('0') << seconds << "]" << std::flush;
        }
      else
        {
          m_done = true;
        }
    }
}

} // namespace utils
