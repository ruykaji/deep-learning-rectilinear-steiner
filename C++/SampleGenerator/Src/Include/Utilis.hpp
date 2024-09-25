#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <condition_variable>
#include <mutex>
#include <string>

namespace utils
{

/**
 * @brief Function to display the progress bar asynchronously
 *
 * @param message The message to display before progress bar.
 * @param done The done flag.
 * @param progress The progress value.
 * @param total_steps The total steps to take.
 * @param mutex The main thread mutex.
 * @param cv The main thread conditional variable.
 */
void
show_progress(std::string message, const bool& done, const uint32_t& progress, uint32_t total_steps, std::mutex& mutex, std::condition_variable& cv);

} // namespace utils

#endif