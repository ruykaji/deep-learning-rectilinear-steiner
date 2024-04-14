#ifndef __UTILITY_HPP__
#define __UTILITY_HPP__

#include <random>
#include <type_traits>

namespace gen
{

template <typename Tp>
inline Tp
random(Tp min, Tp max)
{
  static_assert(std::is_arithmetic<Tp>::value, "random value type has to be an arithmetic type.");

  static std::random_device rd__;
  static std::mt19937 rng__(rd__());

  if constexpr(std::is_integral<Tp>::value)
    {
      static std::uniform_int_distribution<Tp> uni__(min, max);
      return uni__(rng__);
    }
  else
    {
      static std::uniform_real_distribution<Tp> uni__(min, max);
      return uni__(rng__);
    }
};

template <typename Itr>
using M__Requires_input_itr = std::enable_if<std::is_convertible_v<typename Itr::iterator_category, std::input_iterator_tag>>;

template <typename Itr, typename = M__Requires_input_itr<Itr>, typename Tp>
std::size_t
find_index(Itr first, Itr last, const Tp& value)
{
  std::size_t idx__ = 0;

  for(; first != last; ++first, ++idx__)
    {
      if(*first == value)
        return idx__;
    }

  return idx__;
};

} // namespace gen

#endif
