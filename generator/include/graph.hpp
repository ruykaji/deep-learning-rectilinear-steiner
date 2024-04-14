#ifndef __GRAPH_HPP__
#define __GRAPH_HPP__

#include <cstdint>
#include <unordered_set>
#include <vector>

namespace gen
{

struct Edge
{
  uint32_t m_weight;
  std::size_t m_source;
  std::size_t m_destination;
  std::size_t m_prev_source;
  std::size_t m_prev_destination;

  friend bool
  operator>(const Edge& lhs, const Edge& rhs)
  {
    return lhs.m_weight > rhs.m_weight;
  }
};

struct Graph
{
  std::vector<std::vector<Edge>> m_adj;
  std::unordered_set<std::size_t> m_terminals;
};

using MST = std::vector<std::pair<std::size_t, std::size_t>>;

}; // namespace gen

#endif