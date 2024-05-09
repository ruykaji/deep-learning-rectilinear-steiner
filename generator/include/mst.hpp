#ifndef __CPU_MST_HPP__
#define __CPU_MST_HPP__

#include <algorithm>
#include <numeric>
#include <queue>
#include <unordered_map>

#include "graph.hpp"
#include "matrix.hpp"

namespace gen
{

namespace details
{

class Disjoint_set
{
public:
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
  std::size_t m_num_sets{};
  std::unordered_map<std::size_t, std::size_t> m_parent{};
  std::unordered_map<std::size_t, std::size_t> m_rank{};
};

} // namespace details

MST
make_mst(const Graph& graph)
{
  using WN = std::pair<uint32_t, std::size_t>; // Weight - Node pair
  const std::size_t num_vertices__ = graph.m_adj.size();

  std::vector<std::vector<Edge>> full_paths__{};
  std::vector<Edge> short_paths__{};

  // Apply Dijkstra algorithm for each of terminals
  for(const auto& t__ : graph.m_terminals)
    {
      std::vector<bool> visited__(num_vertices__, false);
      std::vector<uint32_t> dist__(num_vertices__, std::numeric_limits<uint32_t>::max());
      std::vector<std::vector<Edge>> paths__(num_vertices__);
      std::priority_queue<WN, std::vector<WN>, std::greater<WN>> queue__;

      queue__.emplace(0, t__);
      dist__[t__ - 1] = 0;

      while(!queue__.empty())
        {
          std::size_t u__ = queue__.top().second;
          queue__.pop();

          if(visited__[u__ - 1])
            {
              continue;
            }

          visited__[u__ - 1] = true;

          for(const Edge& edge : graph.m_adj[u__ - 1])
            {
              std::size_t v__ = edge.m_destination;
              uint32_t weight__ = edge.m_weight;

              if(dist__[u__ - 1] + weight__ < dist__[v__ - 1])
                {
                  dist__[v__ - 1] = dist__[u__ - 1] + weight__;
                  queue__.emplace(dist__[v__ - 1], v__);

                  paths__[v__ - 1] = paths__[u__ - 1];
                  paths__[v__ - 1].push_back(edge);
                }
            }
        }

      for(const auto& tt__ : graph.m_terminals)
        {
          if(!paths__[tt__ - 1].empty() && t__ != tt__)
            {
              auto itr__ = std::find_if(short_paths__.begin(), short_paths__.end(), [&paths__, &tt__](auto& edge) {
                return paths__[tt__ - 1].begin()->m_source == edge.m_destination
                       && paths__[tt__ - 1].rbegin()->m_destination == edge.m_source;
              });

              if(itr__ == short_paths__.end())
                {
                  uint32_t path_weight__ = 0;

                  for(const auto& e__ : paths__[tt__ - 1])
                    path_weight__ += e__.m_weight;

                  short_paths__.emplace_back(path_weight__, paths__[tt__ - 1].begin()->m_source, paths__[tt__ - 1].rbegin()->m_destination);
                  full_paths__.push_back(paths__[tt__ - 1]);
                }
            }
        }
    }

  std::unordered_set<std::size_t> k_terminals__{ graph.m_terminals };
  std::vector<std::vector<Edge>> k_full_paths__{};
  std::vector<Edge> k_short_paths__{};

  {
    // Search for interconnection points in MST using Kruskal algorithm
    std::priority_queue<Edge, std::vector<Edge>, std::greater<Edge>> queue__{};
    details::Disjoint_set disjoint_set__{};

    for(const auto& t__ : graph.m_terminals)
      {
        disjoint_set__.make_set(t__);
      }

    for(const auto& e__ : short_paths__)
      {
        queue__.push(e__);
      }

    while(true)
      {
        Edge edge__ = queue__.top();
        queue__.pop();

        if(disjoint_set__.find(edge__.m_source) != disjoint_set__.find(edge__.m_destination))
          {
            std::size_t idx__ = std::distance(short_paths__.begin(), std::find(short_paths__.begin(), short_paths__.end(), edge__));

            for(const auto& e__ : full_paths__[idx__])
              {
                if(graph.m_terminals.find(e__.m_destination) == graph.m_terminals.end())
                  {
                    if(disjoint_set__.exist(e__.m_destination))
                      {
                        k_terminals__.insert(e__.m_destination);
                      }
                    else
                      {
                        disjoint_set__.make_set(e__.m_destination);
                        disjoint_set__.union_sets(edge__.m_source, e__.m_destination);
                      }
                  }
              }

            disjoint_set__.union_sets(edge__.m_source, edge__.m_destination);

            if(disjoint_set__.is_one_set())
              break;
          }
      }

    // Recreating short and full paths considering new kruskal terminals.
    for(std::size_t i__ = 0; i__ < short_paths__.size(); ++i__)
      {
        std::vector<Edge> possible_path__{};
        uint32_t possible_weight__{};

        for(const auto& e__ : full_paths__[i__])
          {
            possible_path__.push_back(e__);
            possible_weight__ += e__.m_weight;

            if(k_terminals__.find(e__.m_destination) != k_terminals__.end()
               && graph.m_terminals.find(e__.m_destination) == graph.m_terminals.end())
              {
                k_full_paths__.push_back(possible_path__);
                k_short_paths__.emplace_back(possible_weight__, possible_path__.begin()->m_source, e__.m_destination);

                possible_path__.clear();
                possible_weight__ = 0;
              }
          }

        k_full_paths__.push_back(possible_path__);
        k_short_paths__.emplace_back(possible_weight__, possible_path__.begin()->m_source, possible_path__.rbegin()->m_destination);
      }
  }

  MST mst__{};

  // Actually applying Kruskal algorithm to find final MST.
  {
    std::priority_queue<Edge, std::vector<Edge>, std::greater<Edge>> queue__{};
    details::Disjoint_set disjoint_set__{};

    for(const auto& t__ : k_terminals__)
      disjoint_set__.make_set(t__);

    for(const auto& e__ : k_short_paths__)
      queue__.push(e__);

    while(true)
      {
        Edge edge__ = queue__.top();
        queue__.pop();

        if(disjoint_set__.find(edge__.m_source) != disjoint_set__.find(edge__.m_destination))
          {
            std::size_t idx__ = std::distance(k_short_paths__.begin(), std::find(k_short_paths__.begin(), k_short_paths__.end(), edge__));

            for(const auto& e__ : k_full_paths__[idx__])
              {
                mst__.push_back(std::make_pair(e__.m_source, e__.m_destination));
              }

            disjoint_set__.union_sets(edge__.m_source, edge__.m_destination);

            if(disjoint_set__.is_one_set())
              break;
          }
      }
  }

  return mst__;
}

Matrix
make_rectilinear_mst(const Matrix& matrix, const std::vector<Index>& terminals)
{
  Shape shape__ = matrix.shape();

  Index mean_x_idx__{};
  uint32_t min_x__ = shape__.x;
  uint32_t max_x__ = 0;
  double min_mean_distance_x__ = __DBL_MAX__;

  Index mean_y_idx__{};
  uint32_t min_y__ = shape__.y;
  uint32_t max_y__ = 0;
  double min_mean_distance_y__ = __DBL_MAX__;

  for(auto t1__ : terminals)
    {
      double mean_distance_x__ = 0;
      double mean_distance_y__ = 0;

      for(auto t2__ : terminals)
        if(t1__ != t2__)
          {
            mean_distance_x__ += t1__.y > t2__.y ? (t1__.y - t2__.y) : (t2__.y - t1__.y);
            mean_distance_y__ += t1__.x > t2__.x ? (t1__.x - t2__.x) : (t2__.x - t1__.x);
          }

      mean_distance_x__ = mean_distance_x__ / terminals.size() - 1;

      if(mean_distance_x__ < min_mean_distance_x__)
        {
          min_mean_distance_x__ = mean_distance_x__;
          mean_x_idx__ = t1__;
        }

      min_x__ = std::min(t1__.x, min_x__);
      max_x__ = std::max(t1__.x, max_x__);

      mean_distance_y__ = mean_distance_y__ / terminals.size() - 1;

      if(mean_distance_y__ < min_mean_distance_y__)
        {
          min_mean_distance_y__ = mean_distance_y__;
          mean_y_idx__ = t1__;
        }

      min_y__ = std::min(t1__.y, min_y__);
      max_y__ = std::max(t1__.y, max_y__);
    }

  Matrix mst_matrix__{ shape__ };

  if(min_mean_distance_x__ < min_mean_distance_y__)
    {

      for(auto t__ : terminals)
        {
          if(t__ != mean_x_idx__)
            if(t__.y > mean_x_idx__.y)
              while(t__.y >= mean_x_idx__.y)
                {
                  mst_matrix__[t__] = 1;
                  t__ -= { 0, 1 };
                }
            else
              while(t__.y <= mean_x_idx__.y)
                {
                  mst_matrix__[t__] = 1;
                  t__ += { 0, 1 };
                }
        }

      for(Index itr__{ min_x__, mean_x_idx__.y }, end__{ max_x__, mean_x_idx__.y }; itr__ <= end__; itr__ += { 1, 0 })
        mst_matrix__[itr__] = 1;
    }
  else
    {
      for(auto t__ : terminals)
        {
          if(t__ != mean_y_idx__)
            if(t__.x > mean_y_idx__.x)
              while(t__.x >= mean_y_idx__.x)
                {
                  mst_matrix__[t__] = 1;
                  t__ -= { 1, 0 };
                }
            else
              while(t__.x <= mean_y_idx__.x)
                {
                  mst_matrix__[t__] = 1;
                  t__ += { 1, 0 };
                }
        }

      for(Index itr__{ mean_y_idx__.x, min_y__ }, end__{ mean_y_idx__.x, max_y__ }; itr__ <= end__; itr__ += { 0, 1 })
        mst_matrix__[itr__] = 1;
    }

  return mst_matrix__;
}

} // namespace gen

#endif