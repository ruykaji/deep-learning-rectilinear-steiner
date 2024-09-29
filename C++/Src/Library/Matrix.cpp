#include "Include/Matrix.hpp"

namespace matrix
{
/** =============================== CONSTRUCTORS ================================= */

Matrix::Matrix(const Shape& shape)
    : m_shape(shape), m_data(nullptr)
{
  const std::size_t length = allocate();

  for(std::size_t i = 0; i < length; ++i)
    {
      m_data[i] = 0;
    }
};

Matrix::~Matrix()
{
  delete[] m_data;
}

Matrix::Matrix(const Matrix& matrix)
    : m_shape(matrix.m_shape), m_data(nullptr)
{
  const std::size_t length = allocate();

  for(std::size_t i = 0; i < length; ++i)
    {
      m_data[i] = matrix.m_data[i];
    }
}

Matrix::Matrix(Matrix&& matrix)
    : m_shape(matrix.m_shape), m_data(matrix.m_data)
{
  matrix.m_data  = nullptr;
  matrix.m_shape = Shape{ 0, 0, 0 };
}

/** =============================== OPERATORS ==================================== */

Matrix&
Matrix::operator=(const Matrix& matrix)
{
  clear();

  m_shape                  = matrix.m_shape;

  const std::size_t length = allocate();

  for(std::size_t i = 0; i < length; ++i)
    {
      m_data[i] = matrix.m_data[i];
    }

  return *this;
}

Matrix&
Matrix::operator=(Matrix&& matrix)
{
  m_shape        = matrix.m_shape;
  m_data         = matrix.m_data;
  matrix.m_data  = nullptr;
  matrix.m_shape = Shape{ 0, 0, 0 };

  return *this;
}

/** =============================== PUBLIC METHODS =============================== */

uint8_t*
Matrix::data()
{
  return m_data;
}

const uint8_t*
Matrix::data() const
{
  return m_data;
}

const Shape&
Matrix::shape() const
{
  return m_shape;
}

const uint8_t&
Matrix::get_at(const uint8_t x, const uint8_t y, const uint8_t z) const
{
  if(x >= m_shape.m_x || y >= m_shape.m_y || z >= m_shape.m_z)
    {
      throw std::out_of_range("Out of range");
    }

  const std::size_t index = y * m_shape.m_y * m_shape.m_z + x * m_shape.m_z + (m_shape.m_z - z - 1);
  return m_data[index];
}

void
Matrix::set_at(const uint8_t value, const uint8_t x, const uint8_t y, const uint8_t z)
{
  if(x >= m_shape.m_x || y >= m_shape.m_y || z >= m_shape.m_z)
    {
      throw std::out_of_range("Out of range");
    }

  const std::size_t index = y * m_shape.m_y * m_shape.m_z + x * m_shape.m_z + (m_shape.m_z - z - 1);
  m_data[index]           = value;
}

void
Matrix::clear() noexcept(true)
{
  m_shape = Shape{ 0, 0, 0 };
  delete[] m_data;
}

/** =============================== PRIVATE METHODS ============================== */

std::size_t
Matrix::allocate()
{
  const std::size_t length = m_shape.m_z * m_shape.m_y * m_shape.m_x;

  if(length != 0)
    {
      m_data = new uint8_t[length];

      if(m_data == nullptr)
        {
          throw std::runtime_error("Failed to allocate " + std::to_string(length) + " bytes");
        }
    }

  return length;
}

} // namespace matrix
