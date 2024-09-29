#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <mutex>
#include <string>

namespace utils
{

class SyncProgressBar
{
public:
  SyncProgressBar(const std::size_t total_step, const std::string_view message);

  ~SyncProgressBar();

  void
  step();

private:
  std::size_t             m_total_steps;
  std::string             m_message;
  std::size_t             m_progress;
  bool                    m_done;
  std::mutex              m_mutex;
};

} // namespace utils

#endif