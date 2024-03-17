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

using Matrix_shape = Matrix_index;

template <typename Tp> class Matrix_iterator
{

public:
  typedef Tp value_type;
  typedef Tp* pointer;
  typedef Tp& reference;
  typedef std::forward_iterator_tag iterator_category;
  typedef std::ptrdiff_t difference_type;
  typedef std::size_t size_type;

  explicit Matrix_iterator(value_type* value, Matrix_index position, Matrix_shape shape)
      : m_value(value), m_position(position), m_shape(shape){};

  reference
  operator*() noexcept
  {
    return *m_value;
  }

  pointer
  operator->() noexcept
  {
    return m_value;
  }

  Matrix_iterator
  operator++()
  {
    Matrix_iterator tmp(*this);
    m_next();
    return tmp;
  }

  Matrix_iterator&
  operator++(int)
  {
    m_next();
    return *this;
  }

private:
  void
  m_next()
  {
    if(++m_position.x == m_shape.x)
      {
        if(++m_position.y == m_shape.y)
          {
            m_value -= m_shape.x * m_shape.y - 1;
            m_position.y = 0;
          }
        else
          ++m_value;

        m_position.x = 0;
      }
    else
      ++m_value;
  }

private:
  value_type* m_value;
  Matrix_index m_position;
  Matrix_shape m_shape;
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
  typedef Matrix_iterator<Tp> iterator;
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

  iterator
  at(Matrix_index idx) noexcept
  {
    size_type idx__ = idx.y * m_shape.x + idx.x;
    return iterator(m_data + idx__, idx, m_shape);
  }

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
};

#endif