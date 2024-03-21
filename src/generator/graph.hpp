#ifndef __GRAPH_HPP__
#define __GRAPH_HPP__

#include <cstdint>
#include <unordered_set>
#include <vector>

namespace gen
{

struct Edge
{
  uint32_t weight;
  std::size_t source;
  std::size_t destination;
  std::size_t p_source;
  std::size_t p_destination;
};

struct Graph
{
  std::unordered_set<std::size_t> terminals;
  std::vector<std::vector<Edge>> adj;
};

}; // namespace gen

#endif