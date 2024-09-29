#include <cmath>
#include <functional>
#include <queue>
#include <unordered_map>

#include "Include/Transform.hpp"

namespace transform
{

namespace details
{

struct TupleHash
{
  template <typename T>
  std::size_t
  operator()(const T& t) const
  {
    auto hash = std::hash<uint8_t>()(std::get<0>(t));
    hash ^= std::hash<uint8_t>()(std::get<1>(t)) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    hash ^= std::hash<uint8_t>()(std::get<2>(t)) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    return hash;
  }
};

} // namespace details

std::pair<graph::Graph, std::vector<std::tuple<uint8_t, uint8_t, uint8_t>>>
matrix_to_graph(const matrix::Matrix& matrix)
{
  /** TODO: Add iterator for coordinates */
  std::vector<std::tuple<uint8_t, uint8_t, uint8_t>>                                      nodes;
  std::unordered_map<std::tuple<uint8_t, uint8_t, uint8_t>, uint32_t, details::TupleHash> node_map;

  matrix::Shape                                                                           shape = matrix.shape();
  graph::Graph                                                                            graph;

  std::tuple<uint8_t, uint8_t, uint8_t>                                                   inital_state = { 0, 0, 0 };
  std::queue<std::tuple<uint8_t, uint8_t, uint8_t>>                                       queue;

  node_map[inital_state] = nodes.size();
  nodes.emplace_back(inital_state);

  queue.push(inital_state);
  graph.place_node();

  auto search_direction = [&](int8_t dx, int8_t dy, int8_t dz, const std::tuple<uint8_t, uint8_t, uint8_t>& front) {
    auto [x, y, z] = front;

    uint8_t new_x  = x;
    uint8_t new_y  = y;
    uint8_t new_z  = z;

    while(true)
      {
        if((dx > 0 && new_x < shape.m_x - 1) || (dx < 0 && new_x > 0))
          {
            new_x += dx;
          }
        else if(dx != 0)
          {
            break;
          }

        if((dy > 0 && new_y < shape.m_y - 1) || dy < 0 && new_y > 0)
          {
            new_y += dy;
          }
        else if(dy != 0)
          {
            break;
          }

        if((dz > 0 && new_z < shape.m_z - 1) || dz < 0 && new_z > 0)
          {
            new_z += dz;
          }
        else if(dz != 0)
          {
            break;
          }

        const uint8_t& matrix_value = matrix.get_at(new_x, new_y, new_z);

        if(matrix_value == types::INTERSECTION_VIA_CELL || matrix_value == types::INTERSECTION_CELL || matrix_value == types::TERMINAL_CELL)
          {
            const std::tuple<uint8_t, uint8_t, uint8_t> next_node = std::make_tuple(new_x, new_y, new_z);
            uint32_t                                    dest_idx;

            if(node_map.count(next_node) == 0)
              {
                node_map[next_node] = nodes.size();
                dest_idx            = nodes.size();

                nodes.push_back(next_node);
                queue.push(next_node);
              }
            else
              {
                dest_idx = node_map[next_node];
              }

            const uint32_t weight     = std::abs(dx != 0 ? new_x - x : (dy != 0 ? new_y - y : new_z - z));
            const uint32_t source_idx = node_map[front];

            graph.place_node();
            graph.add_edge(weight, source_idx, dest_idx);

            if(matrix_value == types::TERMINAL_CELL)
              {
                graph.add_terminal(dest_idx);
              }

            break;
          }
      }
  };

  while(!queue.empty())
    {
      const auto front = queue.front();
      queue.pop();

      search_direction(1, 0, 0, front);
      search_direction(-1, 0, 0, front);
      search_direction(0, 1, 0, front);
      search_direction(0, -1, 0, front);
      search_direction(0, 0, 1, front);
      search_direction(0, 0, -1, front);
    }

  return std::make_pair(graph, nodes);
}

matrix::Matrix
mst_to_matrix(const matrix::Shape shape, const std::vector<std::pair<uint32_t, uint32_t>>& mst, const std::vector<std::tuple<uint8_t, uint8_t, uint8_t>>& nodes)
{
  matrix::Matrix matrix(shape);

  for(auto [first, second] : mst)
    {
      auto [f_x, f_y, f_z] = nodes[first - 1];
      auto [s_x, s_y, s_z] = nodes[second - 1];

      if((s_x == f_x && s_y == f_y && s_z <= f_z) || (s_x == f_x && s_y <= f_y && s_z == f_z) || (s_x <= f_x && s_y == f_y && s_z == f_z))
        {
          std::swap(f_x, s_x);
          std::swap(f_y, s_y);
          std::swap(f_z, s_z);
        }

      if(f_x == s_x && f_y == s_y)
        {
          for(; f_z <= s_z; ++f_z)
            {
              matrix.set_at(types::PATH_CELL, f_x, f_y, f_z);
            }
        }
      else if(f_x == s_x)
        {
          for(; f_y <= s_y; ++f_y)
            {
              matrix.set_at(types::PATH_CELL, f_x, f_y, f_z);
            }
        }
      else if(f_y == s_y)
        {
          for(; f_x <= s_x; ++f_x)
            {
              matrix.set_at(types::PATH_CELL, f_x, f_y, f_z);
            }
        }
    }

  return matrix;
}

} // namespace transform
