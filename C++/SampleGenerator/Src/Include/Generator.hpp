#ifndef __GENERATOR_HPP__
#define __GENERATOR_HPP__

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
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
   * @param progress Reference to the progress of the combination generation.
   * @param mutex Reference to the mutex used for synchronization.
   * @param cv Reference to the condition variable used for synchronization.
   * @param end Indicates whether this iterator represents the end of the sequence.
   */
  GeneratorItr(uint32_t length, uint8_t number_of_points, uint32_t step, uint32_t& progress, std::mutex& mutex, std::condition_variable& cv, bool end = false);

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
   * @brief Equality comparison operator.
   *
   * @param other The other iterator to compare with.
   * @return true
   * @return false
   */
  bool
  operator==(const GeneratorItr& other) const;

  /**
   * @brief Inequality comparison operator.
   *
   * @param other The other iterator to compare with.
   * @return true
   * @return false
   */
  bool
  operator!=(const GeneratorItr& other) const;

private:
  uint8_t                  m_number_of_points; ///< Number of points in each combination.
  uint32_t                 m_length;           ///< Length of the combination sequence.
  uint32_t                 m_step;             ///< Step size between combinations.
  std::vector<uint32_t>    m_combination;      ///< The current combination.
  bool                     m_end;              ///< Flag indicating if this is the end iterator.
  uint32_t&                m_progress;         ///< Reference to the progress of generation.
  std::mutex&              m_mutex;            ///< Mutex for synchronization.
  std::condition_variable& m_cv;               ///< Condition variable for synchronization.
};

/**
 * @class Generator
 * @brief Generator class to create combinations iteratively.
 *
 * This class generates combinations based on the specified parameters.
 */
class Generator
{
public:
  /** =============================== CONSTRUCTORS ================================= */

  /**
   * @brief Constructor for Generator.
   *
   * @param length The length of the combination to generate.
   * @param number_of_points The number of points in each combination.
   * @param step The step size between combinations.
   */
  Generator(uint32_t length, uint8_t number_of_points, uint32_t step);

  /**
   * @brief Destructor for Generator.
   */
  ~Generator();

  /** =============================== PUBLIC METHODS =============================== */

  /**
   * @brief Get an iterator to the beginning of the combination sequence.
   *
   * @return GeneratorItr
   */
  GeneratorItr
  begin();

  /**
   * @brief Get an iterator to the end of the combination sequence.
   *
   * @return GeneratorItr
   */
  GeneratorItr
  end();

private:
  uint8_t                 m_number_of_points; ///< Number of points in each combination.
  uint32_t                m_length;           ///< Length of the combination sequence.
  uint32_t                m_step;             ///< Step size between combinations.
  uint32_t                m_progress;         ///< Progress of the combination generation.
  bool                    m_done;             ///< Flag indicating if generation is complete.
  uint32_t                m_total_steps;      ///< Total number of steps required.
  std::mutex              m_mutex;            ///< Mutex for synchronization.
  std::condition_variable m_cv;               ///< Condition variable for synchronization.
  std::thread             m_progress_thread;  ///< Thread for tracking progress.
};

} // namespace gen

#endif