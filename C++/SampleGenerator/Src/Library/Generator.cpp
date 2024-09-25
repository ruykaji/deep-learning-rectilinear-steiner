#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <tuple>
#include <vector>

#include "Include/Generator.hpp"
#include "Include/Utilis.hpp"

namespace gen
{

namespace details
{

/**
 * @brief Generates next lexicographical combination.
 *
 * @param combinations Combinations.
 * @param length The size of sequence
 * @param number_of_points The number of points.
 * @return true
 * @return false
 */
bool
next_combination(std::vector<uint32_t>& combinations, const uint32_t length, const uint8_t number_of_points)
{
  for(int64_t i = number_of_points - 1; i >= 0; --i)
    {
      if(combinations[i] < length - number_of_points + i)
        {
          ++combinations[i];

          for(int64_t j = i + 1; j < number_of_points; ++j)
            {
              combinations[j] = combinations[j - 1] + 1;
            }

          return true;
        }
    }

  return false;
}

} // namespace details

uint64_t
nCr(uint32_t n, uint32_t r)
{
  if(r > n)
    {
      return 0;
    }

  if(r == 0 || r == n)
    {
      return 1;
    }

  if(r > n - r)
    {
      r = n - r;
    }

  uint64_t result = 1;

  for(uint32_t i = 1; i <= r; ++i)
    {
      result *= n - i + 1;
      result /= i;
    }

  return result;
}

/**********************************************************************************
 *                               GeneratorItr class                               *
 **********************************************************************************/

/** =============================== CONSTRUCTORS ================================= */

GeneratorItr::GeneratorItr(uint32_t length, uint8_t number_of_points, uint32_t step, uint32_t& progress, std::mutex& mutex, std::condition_variable& cv, bool end)
    : m_length(length), m_number_of_points(number_of_points), m_step(step), m_end(end), m_progress(progress), m_mutex(mutex), m_cv(cv)
{
  if(number_of_points > length)
    {
      throw std::runtime_error("Generator Error: Number of points is greater than total matrix cells");
    }

  for(uint8_t i = 0; i < m_number_of_points; ++i)
    {
      m_combination.emplace_back(i);
    }

  if(m_end)
    {
      m_combination.clear();
    }
}

/** =============================== OPERATORS ==================================== */

GeneratorItr&
GeneratorItr::operator++()
{
  for(uint32_t i = 0; i < m_step; ++i)
    {
      if(!details::next_combination(m_combination, m_length, m_number_of_points))
        {
          m_end = true;
          return *this;
        }
    }

  {
    std::lock_guard<std::mutex> lock(m_mutex);
    ++m_progress;
  }

  m_cv.notify_one();

  return *this;
}

std::vector<uint32_t>
GeneratorItr::operator*() const
{
  return m_combination;
}

bool
GeneratorItr::operator==(const GeneratorItr& other) const
{
  return (m_end == other.m_end);
}

bool
GeneratorItr::operator!=(const GeneratorItr& other) const
{
  return !(*this == other);
}

/**********************************************************************************
 *                                 Generator class                                *
 **********************************************************************************/

/** =============================== CONSTRUCTORS ================================= */
Generator::Generator(const uint32_t length, const uint8_t number_of_points, const uint32_t step)
    : m_length(length), m_number_of_points(number_of_points), m_step(step), m_progress(0), m_done(false), m_total_steps(0), m_mutex(), m_progress_thread()
{
  m_total_steps     = nCr(m_length, m_number_of_points) / m_step;
  m_progress_thread = std::thread(utils::show_progress, "[Combination for " + std::to_string(number_of_points) + " points]", std::ref(m_done), std::ref(m_progress), m_total_steps, std::ref(m_mutex), std::ref(m_cv));
};

Generator::~Generator()
{
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_done = true;
  }

  m_cv.notify_one();

  if(m_progress_thread.joinable())
    {
      m_progress_thread.join();
    }
}

/** =============================== PUBLIC METHODS =============================== */

GeneratorItr
Generator::begin()
{
  return GeneratorItr(m_length, m_number_of_points, m_step, m_progress, m_mutex, m_cv, false);
}

GeneratorItr
Generator::end()
{
  return GeneratorItr(m_length, m_number_of_points, m_step, m_progress, m_mutex, m_cv, true);
}

} // namespace gen
