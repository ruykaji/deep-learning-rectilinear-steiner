#include <algorithm>
#include <numeric>
#include <queue>
#include <unordered_map>

#include "Include/Graph.hpp"

namespace graph
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
      return;

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
      m_parent[k] = find(m_parent[k]);
    return m_parent[k];
  }

  void
  union_sets(std::size_t a, std::size_t b) noexcept
  {
    std::size_t x = find(a);
    std::size_t y = find(b);

    if(x == y)
      return;

    if(m_rank[x] < m_rank[y])
      m_parent[x] = y;
    else
      {
        m_parent[y] = x;

        if(m_rank[x] == m_rank[y])
          ++m_rank[x];
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

} // namespace details

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
  m_adj[source].emplace_back(weight, source + 1, destination + 1);
  m_adj[destination].emplace_back(weight, destination + 1, source + 1);
}

std::vector<std::pair<uint32_t, uint32_t>>
Graph::mst() const
{
  const std::size_t              num_vertices = m_adj.size();

  std::vector<std::vector<Edge>> full_paths;
  std::vector<Edge>              short_paths;

  /** Apply Dijkstra algorithm for each of terminals */
  for(const auto& t : m_terminals)
    {
      std::vector<bool>                                                                                                   visited(num_vertices, false);
      std::vector<uint32_t>                                                                                               dist(num_vertices, std::numeric_limits<uint32_t>::max());
      std::vector<std::vector<Edge>>                                                                                      paths(num_vertices);
      std::priority_queue<details::WeightedNode, std::vector<details::WeightedNode>, std::greater<details::WeightedNode>> queue;

      queue.emplace(0, t);
      dist[t - 1] = 0;

      while(!queue.empty())
        {
          const uint32_t u = queue.top().m_node;
          queue.pop();

          if(visited[u - 1])
            {
              continue;
            }

          visited[u - 1] = true;

          for(const Edge& edge : m_adj[u - 1])
            {
              const uint32_t v      = edge.m_destination;
              const uint32_t weight = edge.m_weight;

              if(dist[u - 1] + weight < dist[v - 1])
                {
                  dist[v - 1] = dist[u - 1] + weight;
                  queue.emplace(dist[v - 1], v);

                  paths[v - 1] = paths[u - 1];
                  paths[v - 1].push_back(edge);
                }
            }
        }

      for(const auto& tt : m_terminals)
        {
          if(!paths[tt - 1].empty() && t != tt)
            {
              auto itr = std::find_if(short_paths.begin(), short_paths.end(), [&paths, &tt](auto& edge) {
                return paths[tt - 1].begin()->m_source == edge.m_destination && paths[tt - 1].rbegin()->m_destination == edge.m_source;
              });

              if(itr == short_paths.end())
                {
                  uint32_t path_weight = 0;

                  for(const auto& e : paths[tt - 1])
                    {
                      path_weight += e.m_weight;
                    }

                  short_paths.emplace_back(path_weight, paths[tt - 1].begin()->m_source, paths[tt - 1].rbegin()->m_destination);
                  full_paths.push_back(paths[tt - 1]);
                }
            }
        }
    }

  std::unordered_set<uint32_t>   k_terminals{ m_terminals };
  std::vector<std::vector<Edge>> k_full_paths;
  std::vector<Edge>              k_short_paths;

  {
    /** Search for interconnection points in MST using Kruskal algorithm */
    std::priority_queue<Edge, std::vector<Edge>, std::greater<Edge>> queue;
    details::Disjoint_set                                            disjoint_set;

    for(const auto& t : m_terminals)
      {
        disjoint_set.make_set(t);
      }

    for(const auto& e : short_paths)
      {
        queue.push(e);
      }

    while(true)
      {
        Edge edge = queue.top();
        queue.pop();

        if(disjoint_set.find(edge.m_source) != disjoint_set.find(edge.m_destination))
          {
            std::size_t idx = std::distance(short_paths.begin(), std::find(short_paths.begin(), short_paths.end(), edge));

            for(const auto& e : full_paths[idx])
              {
                if(m_terminals.find(e.m_destination) == m_terminals.end())
                  {
                    if(disjoint_set.exist(e.m_destination))
                      {
                        k_terminals.insert(e.m_destination);
                      }
                    else
                      {
                        disjoint_set.make_set(e.m_destination);
                        disjoint_set.union_sets(edge.m_source, e.m_destination);
                      }
                  }
              }

            disjoint_set.union_sets(edge.m_source, edge.m_destination);

            if(disjoint_set.is_one_set())
              {
                break;
              }
          }
      }

    // Recreating short and full paths considering new kruskal terminals.
    for(std::size_t i = 0; i < short_paths.size(); ++i)
      {
        std::vector<Edge> possible_path;
        uint32_t          possible_weight;

        for(const auto& e : full_paths[i])
          {
            possible_path.push_back(e);
            possible_weight += e.m_weight;

            if(k_terminals.find(e.m_destination) != k_terminals.end() && m_terminals.find(e.m_destination) == m_terminals.end())
              {
                k_full_paths.push_back(possible_path);
                k_short_paths.emplace_back(possible_weight, possible_path.begin()->m_source, e.m_destination);

                possible_path.clear();
                possible_weight = 0;
              }
          }

        k_full_paths.push_back(possible_path);
        k_short_paths.emplace_back(possible_weight, possible_path.begin()->m_source, possible_path.rbegin()->m_destination);
      }
  }

  std::vector<std::pair<uint32_t, uint32_t>> mst;

  /** Actually applying Kruskal algorithm to find final MST */
  {
    std::priority_queue<Edge, std::vector<Edge>, std::greater<Edge>> queue;
    details::Disjoint_set                                            disjoint_set;

    for(const auto& t : k_terminals)
      {
        disjoint_set.make_set(t);
      }

    for(const auto& e : k_short_paths)
      {
        queue.push(e);
      }

    while(true)
      {
        Edge edge = queue.top();
        queue.pop();

        if(disjoint_set.find(edge.m_source) != disjoint_set.find(edge.m_destination))
          {
            std::size_t idx = std::distance(k_short_paths.begin(), std::find(k_short_paths.begin(), k_short_paths.end(), edge));

            for(const auto& e : k_full_paths[idx])
              {
                mst.push_back(std::make_pair(e.m_source, e.m_destination));
              }

            disjoint_set.union_sets(edge.m_source, edge.m_destination);

            if(disjoint_set.is_one_set())
              {
                break;
              }
          }
      }
  }

  return mst;
}

} // namespace graph
