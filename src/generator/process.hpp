#ifndef __PROCESS_HPP__
#define __PROCESS_HPP__

#include <algorithm>
#include <random>
#include <utility>
#include <vector>

#include "matrix.hpp"

namespace gen
{

inline Matrix_index
random_point(uint32_t width, uint32_t height)
{
  static std::random_device rd__;
  static std::mt19937 rng__(rd__());
  static std::uniform_int_distribution<uint32_t> uni_width__(1, width - 2);
  static std::uniform_int_distribution<uint32_t> uni_height__(1, height - 2);

  return { uni_width__(rng__), uni_height__(rng__) };
}

bool
is_agreed(const std::vector<Matrix_index>& idx)
{
  Matrix_index min__{ UINT32_MAX, UINT32_MAX };
  Matrix_index max__{ 0, 0 };

  for(auto idx1__ : idx)
    {
      for(auto idx2__ : idx)
        {
          float x_diff__ = std::abs(static_cast<float>(idx1__.x) - idx2__.x);
          float y_diff__ = std::abs(static_cast<float>(idx1__.y) - idx2__.y);

          if(x_diff__ <= 2 && x_diff__ <= 2)
            return false;

          if(idx1__.x - idx2__.x == 0 && x_diff__ <= 4)
            return false;

          if(idx1__.y - idx2__.y == 0 && x_diff__ <= 4)
            return false;
        }

      min__.x = std::min(min__.x, idx1__.x);
      min__.y = std::min(min__.y, idx1__.y);
      max__.x = std::max(max__.x, idx1__.x);
      max__.y = std::max(max__.y, idx1__.y);
    }

  uint32_t area__ = (max__.x - min__.x) * (max__.y - min__.y);

  if(area__ < idx.size() * 3)
    return false;

  return true;
}

struct Edge
{
  uint32_t from;
  uint32_t to;
};

template <typename Itr, typename Tp>
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
}

template <typename Tp>
void
preprocess(Matrix<Tp>& matrix, const std::vector<Matrix_index>& pos)
{
  constexpr static uint8_t edge__ = 2;
  constexpr static uint8_t terminal__ = 1;
  constexpr static uint8_t intersection__ = 4;

  Matrix_shape shape__ = matrix.shape();

  std::vector<Matrix_index> nodes__;
  std::vector<std::vector<std::size_t>> graph__;

  auto propagate = [&matrix, &nodes__, &graph__](Matrix_index first, Matrix_index last, Matrix_index step, bool force_intersect = false) {
    if(matrix[first] == intersection__ && !force_intersect)
      return;

    Matrix_index last__ = first;
    std::size_t last_idx__ = find_index(nodes__.begin(), nodes__.end(), first);

    if(matrix[first] != intersection__)
      {
        matrix[first] = intersection__;
        matrix[last] = intersection__;
      }
    else
      first += step;

    for(; first <= last; first += step)
      {
        auto& value__ = matrix[first];

        if(value__ == 0)
          value__ = edge__;
        else
          {
            if(value__ != terminal__)
              value__ = intersection__;

            std::size_t idx__ = find_index(nodes__.begin(), nodes__.end(), first);

            if(idx__ != last_idx__)
              {
                if(idx__ == nodes__.size())
                  {
                    nodes__.push_back(first);
                    graph__.push_back({ last_idx__ });
                  }
                else
                  graph__[idx__].push_back(last_idx__);

                graph__[last_idx__].push_back(idx__);

                last__ = first;
                last_idx__ = idx__;
              }
            else
              {
                nodes__.push_back(first);
                graph__.push_back({});
              }
          }
      }
  };

  for(auto pos__ : pos)
    {
      propagate({ pos__.x, 0 }, { pos__.x, shape__.y - 1 }, { 0, 1 });
      propagate({ 0, pos__.y }, { shape__.x - 1, pos__.y }, { 1, 0 });
    }

  propagate({ 0, 0 }, { 0, shape__.y - 1 }, { 0, 1 });
  propagate({ shape__.x - 1, 0 }, { shape__.x - 1, shape__.y - 1 }, { 0, 1 });
  propagate({ 0, 0 }, { shape__.x - 1, 0 }, { 1, 0 }, true);
  propagate({ 0, shape__.y - 1 }, { shape__.x - 1, shape__.y - 1 }, { 1, 0 }, true);
}

}

#endif