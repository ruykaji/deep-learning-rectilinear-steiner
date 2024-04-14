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
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;

  Matrix(Shape shape) noexcept : m_data(nullptr), m_shape(shape), m_alloc(allocator_type())
  {
    std::size_t length__ = m_shape.x * m_shape.y + 1;

    m_data = alloc_traits::allocate(m_alloc, length__);

    for(std::size_t i = 0; i < length__; ++i)
      alloc_traits::construct(m_alloc, m_data + i, 0);
  };

  ~Matrix()
  {
    alloc_traits::deallocate(m_alloc, m_data, m_shape.x * m_shape.y + 1);
  }

  Shape
  shape() const noexcept
  {
    return m_shape;
  }

  reference
  operator[](Index idx) noexcept
  {
    size_type idx__ = idx.y * m_shape.x + idx.x;
    return m_data[idx__];
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