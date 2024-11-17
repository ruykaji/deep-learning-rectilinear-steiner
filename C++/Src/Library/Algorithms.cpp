#include <algorithm>
#include <functional>
#include <iostream>
#include <numeric>
#include <queue>
#include <set>
#include <thread>
#include <unordered_map>

#include "Include/Algorithms.hpp"
#include "Include/Types.hpp"

template <>
struct std::hash<graph::Edge>
{
  std::size_t
  operator()(const graph::Edge& edge) const noexcept
  {
    return std::hash<std::string>{}(std::to_string(edge.m_source) + "_" + std::to_string(edge.m_destination) + "_" + std::to_string(edge.m_weight));
  }
};

namespace algorithms
{

namespace details
{

class UnionFind
{
private:
  std::vector<uint32_t> parent;
  std::vector<uint32_t> rank;

public:
  UnionFind(std::size_t n)
  {
    parent.resize(n);
    rank.resize(n, 0);
    std::iota(parent.begin(), parent.end(), 0);
  }

  uint32_t
  find(uint32_t u)
  {
    while(u != parent[u])
      {
        parent[u] = parent[parent[u]];
        u         = parent[u];
      }

    return u;
  }

  bool
  union_sets(uint32_t u, uint32_t v)
  {
    uint32_t root_u = find(u);
    uint32_t root_v = find(v);

    if(root_u != root_v)
      {
        if(rank[root_u] < rank[root_v])
          {
            parent[root_u] = root_v;
          }
        else if(rank[root_u] > rank[root_v])
          {
            parent[root_v] = root_u;
          }
        else
          {
            parent[root_v] = root_u;
            ++rank[root_u];
          }

        return true;
      }

    return false;
  }

  bool
  connected(uint32_t u, uint32_t v)
  {
    return find(u) == find(v);
  }

  bool
  connected(const std::vector<uint32_t>& n)
  {
    const uint32_t s = find(n[0]);

    for(std::size_t i = 1, end = n.size(); i < end; ++i)
      {
        if(s != find(n[i]))
          {
            return false;
          }
      }

    return true;
  }

  void
  reset()
  {
    std::fill(rank.begin(), rank.end(), 0);
    std::iota(parent.begin(), parent.end(), 0);
  }
};

/**
 * @brief Handles information about single source to single destination path.
 *
 */
struct PathInfo
{
  uint32_t                 m_weight;
  uint32_t                 m_source;
  uint32_t                 m_destination;
  std::vector<graph::Edge> m_path;
};

std::unordered_map<graph::Edge, std::vector<uint32_t>>
all_paths_dijkstra(const std::unordered_set<uint32_t>& terminals, const std::vector<std::vector<graph::Edge>>& adj, const std::size_t paths_count)
{
  const std::vector<uint32_t>                            terminals_v(terminals.begin(), terminals.end());
  const std::size_t                                      num_vertices  = adj.size();
  const std::size_t                                      num_terminals = terminals.size();

  std::unordered_map<graph::Edge, std::vector<uint32_t>> merge_collection;
  uint32_t                                               path_counter = 0;

  for(uint32_t i = 0; i < num_terminals; ++i)
    {
      const uint32_t                     src = terminals_v[i];

      std::vector<uint32_t>              dist(num_vertices, std::numeric_limits<uint32_t>::max());
      std::vector<std::vector<uint32_t>> prev(num_vertices);

      dist[src - 1] = 0;

      std::priority_queue<graph::Edge, std::vector<graph::Edge>, std::greater<graph::Edge>> queue;
      queue.emplace(0, src);

      while(!queue.empty())
        {
          const uint32_t u      = queue.top().m_source;
          const uint32_t dist_u = queue.top().m_destination;
          queue.pop();

          if(dist_u > dist[u - 1])
            {
              continue;
            }

          for(const auto& edge : adj[u - 1])
            {
              const uint32_t v   = edge.m_destination;
              const uint32_t alt = dist[u - 1] + edge.m_weight;

              if(alt < dist[v - 1])
                {
                  dist[v - 1] = alt;
                  prev[v - 1].clear();
                  prev[v - 1].push_back(u);
                  queue.emplace(alt, v);
                }
              else if(alt == dist[v - 1])
                {
                  prev[v - 1].push_back(u);
                }
            }
        }

      for(uint32_t j = i + 1; j < num_terminals; ++j)
        {
          const uint32_t dst = terminals_v[j];

          if(prev[dst - 1].empty())
            {
              continue;
            }

          std::queue<std::vector<uint32_t>> prev_queue;
          prev_queue.push({ dst });

          ++path_counter;

          while(!prev_queue.empty())
            {
              auto current_path = std::move(prev_queue.front());
              prev_queue.pop();

              const uint32_t current_node = current_path.back();

              if(current_node == src)
                {
                  for(std::size_t k = 0, end = current_path.size() - 1; k < end; ++k)
                    {
                      const uint32_t first  = current_path[k + 1];
                      const uint32_t second = current_path[k];

                      graph::Edge    new_edge;

                      if(first < second)
                        {
                          new_edge.m_source      = first;
                          new_edge.m_destination = second;
                        }
                      else
                        {
                          new_edge.m_source      = second;
                          new_edge.m_destination = first;
                        }

                      const auto& connections = adj.at(new_edge.m_source - 1);
                      auto        it          = std::find_if(connections.begin(), connections.end(), [destination = new_edge.m_destination](const graph::Edge& edge) { return edge.m_destination == destination; });

                      if(it != connections.end())
                        {
                          new_edge.m_weight = it.base()->m_weight;

                          if(merge_collection.count(new_edge) == 0)
                            {
                              merge_collection[new_edge].resize(paths_count, 0);
                            }

                          merge_collection[new_edge][path_counter - 1] = 1;
                        }
                      else
                        {
                          throw std::runtime_error("Algorithm: Something went really wrong :)");
                        }
                    }
                }
              else
                {
                  for(const uint32_t prev_node : prev[current_node - 1])
                    {
                      std::vector<uint32_t> new_path = current_path;
                      new_path.push_back(prev_node);
                      prev_queue.push(std::move(new_path));
                    }
                }
            }
        }
    }

  return merge_collection;
}

} // namespace details

std::vector<std::pair<uint32_t, uint32_t>>
dijkstra_kruskal_greedy(const graph::Graph& graph)
{
  const auto& adj         = graph.get_adj();
  const auto& terminals   = graph.get_terminals();

  std::size_t paths_count = 0;

  for(std::size_t i = 0, end = terminals.size(); i < end; ++i)
    {
      paths_count += i;
    }

  const auto                         merge_collection = details::all_paths_dijkstra(terminals, adj, paths_count);

  std::vector<std::set<graph::Edge>> blocks;
  blocks.resize(paths_count, {});

  for(const auto& [key, value] : merge_collection)
    {
      std::size_t merge = 0;

      for(std::size_t i = 0; i < value.size(); ++i)
        {
          if(value[i] == 1)
            {
              ++merge;
            }
        }

      blocks[merge - 1].insert(key);
    }

  const std::vector<uint32_t>            terminals_v(terminals.begin(), terminals.end());

  details::UnionFind                     uf(adj.size());
  std::vector<graph::Edge>               mst;
  std::unordered_map<uint32_t, uint32_t> mst_nodes;

  for(int32_t i = blocks.size() - 1; i >= 0; --i)
    {
      bool done = false;

      for(const auto& edge : blocks[i])
        {
          if(uf.union_sets(edge.m_destination, edge.m_source))
            {
              mst_nodes[edge.m_destination] += 1;
              mst_nodes[edge.m_source] += 1;

              mst.push_back(edge);

              if(done = uf.connected(terminals_v))
                {
                  break;
                }
            }
        }

      if(done)
        {
          break;
        }
    }

  while(true)
    {
      bool is_found = false;

      for(std::size_t i = 0, end = mst.size(); i < end; ++i)
        {
          const auto& edge = mst[i];

          if(mst_nodes[edge.m_source] == 1 && terminals.count(edge.m_source) == 0)
            {
              mst_nodes[edge.m_source] -= 1;
              mst_nodes[edge.m_destination] -= 1;

              if(mst_nodes[edge.m_destination] == 0)
                {
                  mst_nodes.erase(edge.m_destination);
                }

              if(mst_nodes[edge.m_source] == 0)
                {
                  mst_nodes.erase(edge.m_source);
                }

              mst.erase(mst.begin() + i);
              is_found = true;
              --i;
              --end;
            }
          else if(mst_nodes[edge.m_destination] == 1 && terminals.count(edge.m_destination) == 0)
            {
              mst_nodes[edge.m_source] -= 1;
              mst_nodes[edge.m_destination] -= 1;

              if(mst_nodes[edge.m_source] == 0)
                {
                  mst_nodes.erase(edge.m_source);
                }

              if(mst_nodes[edge.m_destination] == 0)
                {
                  mst_nodes.erase(edge.m_destination);
                }

              mst.erase(mst.begin() + i);
              is_found = true;
              --i;
              --end;
            }
        }

      if(!is_found)
        {
          break;
        }
    }

  std::vector<std::pair<uint32_t, uint32_t>> final_mst;

  for(const auto& edge : mst)
    {
      final_mst.emplace_back(edge.m_source, edge.m_destination);
    }

  return final_mst;
}

} // namespace algorithms
