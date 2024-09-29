#include <algorithm>
#include <numeric>
#include <queue>
#include <unordered_map>

#include "Include/Graph.hpp"

namespace graph
{

bool
operator>(const Edge& lhs, const Edge& rhs)
{
  return lhs.m_weight > rhs.m_weight;
}

bool
operator==(const Edge& lhs, const Edge& rhs)
{
  return lhs.m_source == rhs.m_source && rhs.m_destination == lhs.m_destination;
}

void
Graph::place_node()
{
  m_adj.push_back({});
}

void
Graph::add_terminal(uint32_t terminal)
{
  m_terminals.insert(terminal + 1);
}

void
Graph::add_edge(uint32_t weight, uint32_t source, uint32_t destination)
{
  auto&      source_v = m_adj.at(source);
  const Edge source_edge{ weight, source + 1, destination + 1 };

  if(std::find(source_v.begin(), source_v.end(), source_edge) == source_v.end())
    {
      m_adj[source].emplace_back(std::move(source_edge));
    }

  auto&      destination_v = m_adj.at(destination);
  const Edge destination_edge{ weight, destination + 1, source + 1 };

  if(std::find(destination_v.begin(), destination_v.end(), destination_edge) == destination_v.end())
    {
      m_adj[destination].emplace_back(std::move(destination_edge));
    }
}

const std::vector<std::vector<Edge>>&
Graph::get_adj() const
{
  return m_adj;
}

const std::unordered_set<uint32_t>&
Graph::get_terminals() const
{
  return m_terminals;
}

} // namespace graph
