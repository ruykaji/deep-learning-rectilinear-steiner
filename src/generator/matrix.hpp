#ifndef __MATRIX_HPP__
#define __MATRIX_HPP__

#include <iostream>
#include <iterator>
#include <memory>
#include <type_traits>

namespace gen
{

struct Matrix_index
{
  uint32_t x;
  uint32_t y;

  Matrix_index&
  operator+=(const Matrix_index& rhs)
  {
    x += rhs.x;
    y += rhs.y;
    return *this;
  }

  friend bool
  operator<=(const Matrix_index& lhs, const Matrix_index& rhs)
  {
    return ((lhs.x == rhs.x) && (lhs.y <= rhs.y)) || ((lhs.y == rhs.y) && (lhs.x <= rhs.x));
  }

  friend bool
  operator==(const Matrix_index& lhs, const Matrix_index& rhs)
  {
    return lhs.x == rhs.x && lhs.y == rhs.y;
  }
};

struct Matrix_shape
{
  uint32_t x;
  uint32_t y;
};

template <typename Tp> class Matrix
{
  static_assert(std::is_arithmetic<Tp>::value, "Matrix value_type have to be an arithmetic type.");

  typedef std::allocator<Tp> alloc;
  typedef std::allocator_traits<alloc> alloc_traits;

public:
  typedef Tp value_type;
  typedef alloc allocator_type;
  typedef Tp& reference;
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;

  explicit Matrix(Matrix_shape shape) noexcept : m_data(nullptr), m_shape(shape), m_alloc(alloc())
  {
    m_data = alloc_traits::allocate(m_alloc, m_shape.x * m_shape.y + 1);
  };

  Matrix(const Matrix&) = delete;

  Matrix(Matrix&&) = default;

  ~Matrix()
  {
    alloc_traits::deallocate(m_alloc, m_data, m_shape.x * m_shape.y + 1);
  }

  Matrix&
  operator=(const Matrix&)
      = delete;

  Matrix&
  operator=(Matrix&&)
      = default;

  Matrix_shape
  shape() const noexcept
  {
    return m_shape;
  }

  reference
  operator[](Matrix_index idx) noexcept
  {
    size_type idx__ = idx.y * m_shape.x + idx.x;
    return *(m_data + idx__);
  }

  friend std::ostream&
  operator<<(std::ostream& out, const Matrix<Tp>& matrix)
  {
    Matrix_shape shape__ = matrix.m_shape;

    for(size_type i = 0; i < shape__.y; ++i)
      {
        for(size_type j = 0; j < shape__.x; ++j)
          out << matrix.m_data[i * shape__.x + j] << ' ';

        out << '\n';
      }

    return out;
  }

private:
  value_type* m_data;
  Matrix_shape m_shape;
  alloc m_alloc;
};

}; // namespace gen

#endif