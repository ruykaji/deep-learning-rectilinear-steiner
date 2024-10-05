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
  operator>(const Edge& lhs, const Edge& rhs);

  friend bool
  operator<(const Edge& lhs, const Edge& rhs);

  friend bool
  operator==(const Edge& lhs, const Edge& rhs);
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
  const std::vector<std::vector<Edge>>&
  get_adj() const;

  const std::unordered_set<uint32_t>&
  get_terminals() const;

private:
  std::vector<std::vector<Edge>> m_adj;
  std::unordered_set<uint32_t>   m_terminals;
};

} // namespace graph

#endif