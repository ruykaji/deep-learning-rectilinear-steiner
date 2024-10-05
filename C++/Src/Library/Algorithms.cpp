#include <algorithm>
#include <functional>
#include <numeric>
#include <queue>
#include <unordered_map>

#include "Include/Algorithms.hpp"
#include "Include/Matrix.hpp"
#include "Include/Types.hpp"

namespace algorithms
{

namespace details
{

class Disjoint_set
{
public:
  Disjoint_set()
      : m_num_sets(0), m_parent(), m_rank() {};

  void
  make_set(std::size_t set) noexcept
  {
    if(exist(set))
      {
        return;
      }

    m_parent.emplace(set, set);
    m_rank.emplace(set, 0);
    ++m_num_sets;
  }

  bool
  exist(std::size_t k) const noexcept
  {
    return m_parent.count(k) != 0;
  }

  std::size_t
  find(std::size_t k) noexcept
  {
    if(m_parent[k] != k)
      {
        m_parent[k] = find(m_parent[k]);
      }

    return m_parent[k];
  }

  void
  union_sets(std::size_t a, std::size_t b) noexcept
  {
    const std::size_t x = find(a);
    const std::size_t y = find(b);

    if(x == y)
      {
        return;
      }

    if(m_rank[x] < m_rank[y])
      {
        m_parent[x] = y;
      }
    else
      {
        m_parent[y] = x;

        if(m_rank[x] == m_rank[y])
          {
            ++m_rank[x];
          }
      }

    --m_num_sets;
  }

  bool
  is_one_set() const noexcept
  {
    return m_num_sets == 1;
  }

private:
  std::size_t                                  m_num_sets;
  std::unordered_map<std::size_t, std::size_t> m_parent;
  std::unordered_map<std::size_t, std::size_t> m_rank;
};

struct WeightedNode
{
  uint32_t m_weight;
  uint32_t m_node;

  friend bool
  operator>(const WeightedNode& lhs, const WeightedNode& rhs)
  {
    return lhs.m_weight > rhs.m_weight;
  }
};

struct ViewLine
{
  std::tuple<uint8_t, uint8_t, uint8_t> m_start;
  std::tuple<uint8_t, uint8_t, uint8_t> m_end;
};

} // namespace details

std::vector<std::pair<uint32_t, uint32_t>>
dijkstra_kruskal(const graph::Graph& graph)
{
  struct PathInfo
  {
    uint32_t                 total_weight;
    uint32_t                 source;
    uint32_t                 destination;
    std::vector<graph::Edge> full_path;
  };

  const auto&                           adj       = graph.get_adj();
  const auto&                           terminals = graph.get_terminals();
  const std::vector<uint32_t>           terminals_v(terminals.begin(), terminals.end());
  const std::size_t                     num_vertices  = adj.size();
  const std::size_t                     num_terminals = terminals.size();

  /** Number of sort and full paths have to be equal */
  std::vector<PathInfo>                 paths_info;
  std::vector<graph::Edge>              short_paths;
  std::vector<std::vector<graph::Edge>> full_paths;

  for(uint32_t i = 0; i < num_terminals; ++i)
    {
      const uint32_t                                                                        src     = terminals_v[i];

      uint32_t                                                                              visited = 0;
      std::vector<uint32_t>                                                                 dist(num_vertices, std::numeric_limits<uint32_t>::max());
      std::vector<uint32_t>                                                                 weights(num_vertices, 0);
      std::vector<uint32_t>                                                                 prev(num_vertices, 0);
      std::priority_queue<graph::Edge, std::vector<graph::Edge>, std::greater<graph::Edge>> queue;

      dist[src - 1] = 0;
      queue.emplace(0, src);

      while(!queue.empty())
        {
          const uint32_t u = queue.top().m_source;
          queue.pop();

          for(const auto& edge : adj[u - 1])
            {
              uint32_t v   = edge.m_destination;
              uint32_t alt = dist[u - 1] + edge.m_weight;

              if(alt < dist[v - 1])
                {
                  dist[v - 1]    = alt;
                  prev[v - 1]    = u;
                  weights[v - 1] = edge.m_weight;
                  queue.emplace(alt, v);

                  if(terminals.find(v) != terminals.end())
                    {
                      ++visited;
                    }

                  if(visited == num_terminals)
                    {
                      break;
                    }
                }
            }

          if(visited == num_terminals)
            {
              break;
            }
        }

      /** Get short and full paths */
      for(uint32_t j = i + 1; j < num_terminals; ++j)
        {
          uint32_t                 dst = terminals_v[j];
          std::vector<graph::Edge> full_path;

          for(uint32_t at = dst; at != src; at = prev[at - 1])
            {
              const uint32_t from   = prev[at - 1];
              const uint32_t weight = weights[at - 1];

              full_path.emplace_back(weight, from, at);
            }

          std::reverse(full_path.begin(), full_path.end());
          paths_info.push_back({ dist[dst - 1], src, dst, std::move(full_path) });
        }
    }

  /** Using Kruskal's algorithm find MST in short paths */
  std::vector<graph::Edge> mst_edges;

  std::sort(paths_info.begin(), paths_info.end(), [](const PathInfo& a, const PathInfo& b) { return a.total_weight < b.total_weight; });

  std::vector<uint32_t> parent(num_vertices);
  std::iota(parent.begin(), parent.end(), 0);

  const std::function<uint32_t(uint32_t)> find = [&parent](uint32_t u) {
    while(u != parent[u])
      {
        parent[u] = parent[parent[u]];
        u         = parent[u];
      }

    return u;
  };

  const std::function<bool(uint32_t, uint32_t)> union_sets = [&parent, &find](uint32_t u, uint32_t v) {
    uint32_t root_u = find(u);
    uint32_t root_v = find(v);

    if(root_u != root_v)
      {
        parent[root_u] = root_v;
        return true;
      }

    return false;
  };

  for(const auto& path_info : paths_info)
    {
      if(union_sets(path_info.source, path_info.destination))
        {
          mst_edges.insert(mst_edges.end(), path_info.full_path.begin(), path_info.full_path.end());
        }
    }

  std::sort(mst_edges.begin(), mst_edges.end(), [](const graph::Edge& a, const graph::Edge& b) { return std::tie(a.m_source, a.m_destination, a.m_weight) < std::tie(b.m_source, b.m_destination, b.m_weight); });
  mst_edges.erase(std::unique(mst_edges.begin(), mst_edges.end(), [](const graph::Edge& a, const graph::Edge& b) { return a.m_source == b.m_source && a.m_destination == b.m_destination; }), mst_edges.end());

  /** Using Kruskal's algorithm find MST in full paths from short path mst edges*/
  std::vector<std::pair<uint32_t, uint32_t>> final_mst;

  std::iota(parent.begin(), parent.end(), 0);
  std::sort(mst_edges.begin(), mst_edges.end(), [](const graph::Edge& a, const graph::Edge& b) { return a.m_weight < b.m_weight; });

  for(const auto& edge : mst_edges)
    {
      if(union_sets(edge.m_source, edge.m_destination))
        {
          final_mst.emplace_back(edge.m_source, edge.m_destination);
        }
    }

  return final_mst;
}

} // namespace algorithms
