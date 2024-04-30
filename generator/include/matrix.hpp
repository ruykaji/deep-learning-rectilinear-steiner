#ifndef __MATRIX_HPP__
#define __MATRIX_HPP__

#include <iostream>
#include <iterator>
#include <memory>
#include <type_traits>

namespace gen
{

struct Index
{
  uint32_t x;
  uint32_t y;

  Index&
  operator+=(const Index& rhs)
  {
    x += rhs.x;
    y += rhs.y;
    return *this;
  }

  Index&
  operator-=(const Index& rhs)
  {
    x -= rhs.x;
    y -= rhs.y;
    return *this;
  }

  friend Index
  operator+(const Index& lhs, const Index& rhs)
  {
    Index tmp__{};

    tmp__.x = lhs.x + rhs.x;
    tmp__.y = rhs.y + lhs.y;

    return tmp__;
  }

  friend Index
  operator-(const Index& lhs, const Index& rhs)
  {
    Index tmp__{};

    tmp__.x = lhs.x - rhs.x;
    tmp__.y = lhs.y - rhs.y;

    return tmp__;
  }

  friend bool
  operator<=(const Index& lhs, const Index& rhs)
  {
    return ((lhs.x == rhs.x) && (lhs.y <= rhs.y)) || ((lhs.y == rhs.y) && (lhs.x <= rhs.x));
  }

  friend bool
  operator==(const Index& lhs, const Index& rhs)
  {
    return lhs.x == rhs.x && lhs.y == rhs.y;
  }
};

struct Shape
{
  uint32_t x;
  uint32_t y;
};

class Matrix
{
  typedef std::allocator<uint32_t> alloc;
  typedef std::allocator_traits<alloc> alloc_traits;

public:
  typedef uint32_t value_type;
  typedef alloc allocator_type;
  typedef uint32_t& reference;
  typedef const uint32_t& const_reference;
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;

  Matrix(Shape shape) noexcept : m_data(nullptr), m_shape(shape), m_alloc(allocator_type())
  {
    std::size_t length__ = m_shape.x * m_shape.y + 1;

    m_data = alloc_traits::allocate(m_alloc, length__);

    for(std::size_t i = 0; i < length__; ++i)
      alloc_traits::construct(m_alloc, m_data + i, 0);
  };

  Matrix(const Matrix&) = delete;

  Matrix(Matrix&& matrix)
  {
    m_data = matrix.m_data;
    m_shape = matrix.m_shape;

    matrix.m_data = nullptr;
    matrix.m_shape = {};
  }

  ~Matrix()
  {
    alloc_traits::deallocate(m_alloc, m_data, m_shape.x * m_shape.y + 1);
  }

  Matrix&
  operator=(const Matrix&)
      = delete;

  Matrix&
  operator=(Matrix&&)
      = delete;

  reference
  operator[](const Index& idx) noexcept
  {
    size_type idx__ = idx.y * m_shape.x + idx.x;
    return m_data[idx__];
  }

  const_reference
  operator[](const Index& idx) const noexcept
  {
    size_type idx__ = idx.y * m_shape.x + idx.x;
    return m_data[idx__];
  }

  Shape
  shape() const noexcept
  {
    return m_shape;
  }

  friend double
  operator/(const Matrix& lhs, const Matrix& rhs)
  {
    std::size_t length__ = lhs.m_shape.x * lhs.m_shape.y + 1;
    double sim_count__ = 0;

    for(std::size_t i = 0; i < length__; ++i)
      {
        if(lhs.m_data[i] == rhs.m_data[i])
          ++sim_count__;
      }

    return sim_count__ / length__;
  }

  friend std::ostream&
  operator<<(std::ostream& out, const Matrix& matrix) noexcept
  {
    Shape shape__ = matrix.m_shape;

    for(Matrix::size_type i = 0; i < shape__.y; ++i)
      {
        for(Matrix::size_type j = 0; j < shape__.x; ++j)
          out << matrix.m_data[i * shape__.x + j] << ' ';

        out << '\n';
      }

    return out;
  }

private:
  value_type* m_data;
  Shape m_shape;
  alloc m_alloc;
};

}; // namespace gen

#endif