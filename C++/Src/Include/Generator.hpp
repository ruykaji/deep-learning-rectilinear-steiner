#ifndef __GENERATOR_HPP__
#define __GENERATOR_HPP__

#include <tuple>
#include <vector>

namespace gen
{

/**
 * @brief Computes nCr (combinations count) for large numbers
 *
 * @param n The number of items.
 * @param r The number of items being chosen at a time.
 * @return unsigned long long
 */
uint64_t
nCr(uint32_t n, uint32_t r);

class GeneratorItr
{
public:
  /** =============================== CONSTRUCTORS ================================= */

  /**
   * @brief Constructor for GeneratorItr.
   *
   * @param length The length of the combination to generate.
   * @param number_of_points The number of points in the combination.
   * @param step The step size between combinations.
   * @param x The inital combination.
   */
  GeneratorItr(const uint32_t length, const uint8_t number_of_points, const uint64_t step, const uint64_t start, const uint64_t end);

  /** =============================== OPERATORS ==================================== */

  /**
   * @brief Increment operator to move to the next combination.
   *
   * @return GeneratorItr&
   */
  GeneratorItr&
  operator++();

  /**
   * @brief Dereference operator to access the current combination.
   *
   * @return std::vector<uint32_t>
   */
  std::vector<uint32_t>
  operator*() const;

  /**
   * @brief Less comparison operator.
   *
   * @param other The other iterator to compare with.
   * @return true
   * @return false
   */
  bool
  operator<(const GeneratorItr& other) const;

private:
  uint8_t               m_number_of_points; ///< Number of points in each combination.
  uint32_t              m_length;           ///< Length of the combination sequence.
  uint64_t              m_step;             ///<  Size between combinations
  uint64_t              m_start;
  uint64_t              m_end;
  std::vector<uint32_t> m_combination; ///< The current combination.
};

} // namespace gen

#endif