#ifndef __GRAPH_HPP__
#define __GRAPH_HPP__

#include <cstdint>
#include <unordered_set>
#include <vector>

namespace graph
{

struct Edge
{
  uint32_t m_weight;
  uint32_t m_source;
  uint32_t m_destination;

  friend bool
  operator>(const Edge& lhs, const Edge& rhs)
  {
    return lhs.m_weight > rhs.m_weight;
  }

  friend bool
  operator==(const Edge& lhs, const Edge& rhs)
  {
    return lhs.m_source == rhs.m_source && rhs.m_destination == lhs.m_destination;
  }
};

class Graph
{
public:
  void
  place_node();

  void
  add_terminal(uint32_t terminal);

  void
  add_edge(uint32_t weight, uint32_t source, uint32_t destination);

  std::vector<std::pair<uint32_t, uint32_t>>
  mst() const;

private:
  std::vector<std::vector<Edge>> m_adj;
  std::unordered_set<uint32_t>   m_terminals;
};

} // namespace graph

#endif