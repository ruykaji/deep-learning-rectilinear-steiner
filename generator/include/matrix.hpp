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
  uint32_t z;

  Index&
  operator+=(const Index& rhs)
  {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
  }

  Index&
  operator-=(const Index& rhs)
  {
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    return *this;
  }

  friend void
  swap(Index& lhs, Index& rhs)
  {
    Index tmp{ lhs.x, lhs.y, lhs.z };

    lhs = rhs;
    rhs = tmp;
  }

  friend Index
  operator+(const Index& lhs, const Index& rhs)
  {
    Index tmp__{};

    tmp__.x = lhs.x + rhs.x;
    tmp__.y = rhs.y + lhs.y;
    tmp__.z = rhs.z + lhs.z;

    return tmp__;
  }

  friend Index
  operator-(const Index& lhs, const Index& rhs)
  {
    Index tmp__{};

    tmp__.x = lhs.x - rhs.x;
    tmp__.y = lhs.y - rhs.y;
    tmp__.z = lhs.z - rhs.z;

    return tmp__;
  }

  friend bool
  operator<=(const Index& lhs, const Index& rhs)
  {
    if((lhs.x == rhs.x) && (lhs.y == rhs.y) && (lhs.z <= rhs.z))
      return true;

    if((lhs.x == rhs.x) && (lhs.y <= rhs.y) && (lhs.z == rhs.z))
      return true;

    if((lhs.x <= rhs.x) && (lhs.y == rhs.y) && (lhs.z == rhs.z))
      return true;

    return false;
  }

  friend bool
  operator==(const Index& lhs, const Index& rhs)
  {
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
  }
};

struct Shape
{
  uint32_t x;
  uint32_t y;
  uint32_t z;
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
    std::size_t length__ = m_shape.x * m_shape.y * m_shape.z + 1;

    m_data = alloc_traits::allocate(m_alloc, length__);

    for(std::size_t i = 0; i < length__; ++i)
      alloc_traits::construct(m_alloc, m_data + i, 0);
  };

  Matrix(const Matrix& matrix) : m_data(nullptr), m_shape(matrix.m_shape), m_alloc(allocator_type())
  {
    std::size_t length__ = m_shape.x * m_shape.y * m_shape.z + 1;

    m_data = alloc_traits::allocate(m_alloc, length__);

    for(std::size_t i = 0; i < length__; ++i)
      m_data[i] = matrix.m_data[i];
  };

  Matrix(Matrix&& matrix)
  {
    m_data = matrix.m_data;
    m_shape = matrix.m_shape;

    matrix.m_data = nullptr;
    matrix.m_shape = {};
  }

  ~Matrix()
  {
    alloc_traits::deallocate(m_alloc, m_data, m_shape.x * m_shape.y * m_shape.z + 1);
  }

  Matrix&
  operator=(const Matrix& matrix)
  {
    std::size_t length__ = m_shape.x * m_shape.y * m_shape.z + 1;

    m_data = alloc_traits::allocate(m_alloc, length__);

    for(std::size_t i = 0; i < length__; ++i)
      m_data[i] = matrix.m_data[i];

    return *this;
  }

  Matrix&
  operator=(Matrix&& matrix)
  {
    m_data = matrix.m_data;
    m_shape = matrix.m_shape;

    matrix.m_data = nullptr;
    matrix.m_shape = {};

    return *this;
  }

  reference
  operator[](const Index& idx) noexcept
  {
    size_type idx__ = idx.z * m_shape.x * m_shape.y + idx.y * m_shape.x + idx.x;
    return m_data[idx__];
  }

  const_reference
  operator[](const Index& idx) const noexcept
  {
    size_type idx__ = idx.z * m_shape.x * m_shape.y + idx.y * m_shape.x + idx.x;
    return m_data[idx__];
  }

  Shape
  shape() const noexcept
  {
    return m_shape;
  }

  Matrix&
  operator+=(const Matrix& rhs)
  {
    std::size_t length__ = m_shape.x * m_shape.y * m_shape.z + 1;

    for(std::size_t i__ = 0; i__ < length__; ++i__)
      {
        m_data[i__] += rhs.m_data[i__];
      }

    return *this;
  }

  friend std::ostream&
  operator<<(std::ostream& out, const Matrix& matrix) noexcept
  {
    Shape shape__ = matrix.m_shape;

    for(Matrix::size_type i__ = 0; i__ < shape__.z; ++i__)
      {
        for(Matrix::size_type j__ = 0; j__ < shape__.y; ++j__)
          {
            for(Matrix::size_type k = 0; k < shape__.x; ++k)
              out << matrix.m_data[i__ * shape__.x * shape__.y + j__ * shape__.x + k] << ' ';

            out << '\n';
          }

        if(i__ != shape__.z - 1)
          {
            out << "\n";
          }
      }

    return out;
  }

public:
  void
  zero_all(uint32_t value) noexcept
  {
    std::size_t length__ = m_shape.x * m_shape.y * m_shape.z + 1;

    for(std::size_t i__ = 0; i__ < length__; ++i__)
      if(m_data[i__] == value)
        m_data[i__] = 0;
  }

private:
  value_type* m_data;
  Shape m_shape;
  alloc m_alloc;
};

}; // namespace gen

#endif