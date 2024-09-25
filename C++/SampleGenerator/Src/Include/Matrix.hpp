#ifndef __MATRIX_HPP__
#define __MATRIX_HPP__

#include <cstdint>
#include <cstdlib>
#include <stdexcept>
#include <vector>

namespace matrix
{

struct Shape
{
  uint8_t m_x = 0;
  uint8_t m_y = 0;
  uint8_t m_z = 0;
};

class Matrix
{
public:
  /** =============================== CONSTRUCTORS ================================= */

  /**
   * @brief Constructs a Matrix with the specified shape
   *
   * @param shape Shape structure containing the dimensions of the matrix
   */
  Matrix(const Shape& shape = {});

  /**
   * @brief  Destroys the Matrix object.
   *
   */
  ~Matrix();

  /**
   * @brief Copy constructor.
   *
   * @param matrix The matrix to be copied.
   */
  Matrix(const Matrix& matrix);

  /**
   * @brief Move constructor.
   *
   * @param matrix The matrix to be moved.
   */
  Matrix(Matrix&& matrix);

public:
  /** =============================== OPERATORS ==================================== */

  /**
   * @brief Copy assignment operator.
   *
   * @param matrix The matrix to copy from.
   * @return Matrix&
   */
  Matrix&
  operator=(const Matrix& matrix);

  /**
   * @brief Move assignment operator.
   *
   * @param matrix The matrix to be moved.
   * @return Matrix&
   */
  Matrix&
  operator=(Matrix&& matrix);

public:
  /** =============================== PUBLIC METHODS =============================== */

  /**
   * @brief Returns pointer to the underlying data.
   *
   * @return uint8_t*
   */
  uint8_t*
  data();

  /**
   * @brief Returns pointer to the underlying data.
   *
   * @return const uint8_t*
   */
  const uint8_t*
  data() const;

  /**
   * @brief Returns the shape of the matrix.
   *
   * @return const Shape&
   */
  const Shape&
  shape() const;

  /**
   * @brief Retrieves the value stored at the given (x, y, z) coordinates.
   *
   * @param x X-coordinate (width).
   * @param y Y-coordinate (height).
   * @param z Z-coordinate (depth).
   * @return const uint8_t&
   */
  const uint8_t&
  get_at(const uint8_t x, const uint8_t y, const uint8_t z) const;

  /**
   * @brief Sets the value at the given (x, y, z) coordinates.
   *
   * @param value The value to set at the given coordinates.
   * @param x X-coordinate (width).
   * @param y Y-coordinate (height).
   * @param z Z-coordinate (depth).
   */
  void
  set_at(const uint8_t value, const uint8_t x, const uint8_t y, const uint8_t z);

  /**
   * @brief Clears the matrix data.
   *
   */
  void
  clear() noexcept(true);

private:
  /** =============================== PRIVATE METHODS ============================== */

  /**
   * @brief Allocates memory for matrix elements.
   *
   * @return std::size_t
   */
  std::size_t
  allocate();

private:
  Shape    m_shape; ///< Holds the dimensions of the matrix.
  uint8_t* m_data;  ///< Pointer to the dynamically allocated matrix data.
};

} // namespace matrix

#endif