#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <tuple>
#include <vector>

#include "Include/Generator.hpp"

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
 */
void
next_combination(std::vector<uint32_t>& combinations, const uint32_t length, const uint8_t number_of_points, uint64_t idx)
{
  uint32_t x = 0;

  for(uint8_t i = 0; i < number_of_points; ++i)
    {
      uint64_t binom;

      while((binom = nCr(length - x - 1, number_of_points - i - 1)) <= idx)
        {
          ++x;
          idx -= binom;
        }

      combinations[i] = x;
      ++x;
    }
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

GeneratorItr::GeneratorItr(uint32_t length, uint8_t number_of_points, uint64_t step, const uint64_t start, const uint64_t end)
    : m_length(length), m_number_of_points(number_of_points), m_step(step), m_start(start), m_end(end), m_combination()
{
  m_combination.resize(number_of_points, 0);
  details::next_combination(m_combination, m_length, m_number_of_points, m_start);
};

/** =============================== OPERATORS ==================================== */

GeneratorItr&
GeneratorItr::operator++()
{
  if(m_start += m_step; m_start < m_end)
    {
      details::next_combination(m_combination, m_length, m_number_of_points, m_start);
    }

  return *this;
}

std::vector<uint32_t>
GeneratorItr::operator*() const
{
  return m_combination;
}

bool
GeneratorItr::operator<(const GeneratorItr& other) const
{
  return m_start < other.m_start && m_end == other.m_end;
}


} // namespace gen
