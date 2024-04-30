#ifndef __CPU_MST_HPP__
#define __CPU_MST_HPP__

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
    m_parent.emplace(std::make_pair(set, set));
    m_rank.emplace(std::make_pair(set, 0));
    ++m_num_sets;
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
    else if(m_rank[x] > m_rank[y])
      m_parent[y] = x;
    else
      {
        m_parent[y] = x;
        ++m_rank[x];
      }

    --m_num_sets;
  }

  bool
  is_one_set() const noexcept
  {
    return m_num_sets == 1;
  }

  void
  clear() noexcept
  {
    m_parent.clear();
    m_rank.clear();
    m_num_sets = 0;
  }

private:
  std::size_t m_num_sets{};
  std::unordered_map<std::size_t, std::size_t> m_parent{};
  std::unordered_map<std::size_t, std::size_t> m_rank{};
};

} // namespace details

MST
make_mst(const Graph& graph, const std::vector<Index>& nodes)
{
  static std::vector<std::size_t> s_lengths__{};
  static std::vector<std::size_t> s_sources__{};
  static std::vector<std::size_t> s_prevs__{};

  details::Disjoint_set disjoin_set__{};

  std::vector<Edge> mst_edges__{};
  std::priority_queue<Edge, std::vector<Edge>, std::greater<Edge>> queue__{};

  s_lengths__.assign(graph.m_adj.size(), 0);
  s_sources__.assign(graph.m_adj.size(), 0);
  s_prevs__.assign(graph.m_adj.size(), 0);

  for(const auto& t__ : graph.m_terminals)
    {
      disjoin_set__.make_set(t__);
      s_sources__[t__ - 1] = t__;
      s_lengths__[t__ - 1] = 0;

      for(const auto edge__ : graph.m_adj[t__ - 1])
        {
          if(edge__.m_source == t__)
            queue__.push(edge__);
          else
            queue__.emplace(edge__.m_weight, edge__.m_destination, edge__.m_source);
        }
    }

  while(true)
    {
      Edge edge__ = queue__.top();
      queue__.pop();

      uint32_t weight__ = edge__.m_weight;
      std::size_t dest__ = edge__.m_destination;
      std::size_t source__ = edge__.m_source;
      std::size_t prev_source__ = edge__.m_prev_source;

      if(s_sources__[dest__ - 1] == 0)
        {
          s_sources__[dest__ - 1] = source__;
          s_lengths__[dest__ - 1] = weight__;
          s_prevs__[dest__ - 1] = prev_source__ != 0 ? prev_source__ : source__;

          for(const auto& e__ : graph.m_adj[dest__ - 1])
            {
              std::size_t local_dest__ = e__.m_destination != dest__ ? e__.m_destination : e__.m_source;

              if(s_sources__[local_dest__ - 1] == 0)
                queue__.emplace(e__.m_weight + weight__, source__, local_dest__, dest__, 0);
            }
        }
      else if(disjoin_set__.find(s_sources__[dest__ - 1]) != disjoin_set__.find(source__))
        {
          if(graph.m_terminals.find(dest__) != graph.m_terminals.end())
            {
              disjoin_set__.union_sets(source__, dest__);
              mst_edges__.emplace_back(edge__);

              if(disjoin_set__.is_one_set())
                break;
            }
          else
            queue__.emplace(s_lengths__[dest__ - 1] + weight__, source__, s_sources__[dest__ - 1], prev_source__, dest__);
        }
    }

  MST mst__{};

  for(const auto& edge__ : mst_edges__)
    {
      std::size_t source__ = edge__.m_source;
      std::size_t dest__ = edge__.m_destination;
      std::size_t prev_source__ = edge__.m_prev_source;
      std::size_t prev_dest__ = edge__.m_prev_destination;

      if(prev_source__ == 0 && prev_dest__ == 0)
        mst__.push_back(std::make_pair(source__, dest__));
      else
        {
          while(prev_source__ != 0 && s_prevs__[prev_source__ - 1] != 0)
            {
              mst__.push_back(std::make_pair(s_prevs__[prev_source__ - 1], prev_source__));

              std::size_t tmp = s_prevs__[prev_source__ - 1];
              s_prevs__[prev_source__ - 1] = 0;
              prev_source__ = tmp;
            }

          while(prev_dest__ != 0 && s_prevs__[prev_dest__ - 1] != 0)
            {
              mst__.push_back(std::make_pair(prev_dest__, s_prevs__[prev_dest__ - 1]));

              std::size_t tmp = s_prevs__[prev_dest__ - 1];
              s_prevs__[prev_dest__ - 1] = 0;
              prev_dest__ = tmp;
            }

          if(prev_source__ == 0)
            {
              mst__.push_back(std::make_pair(source__, edge__.m_prev_destination));
              s_prevs__[edge__.m_prev_destination - 1] = 0;
            }

          if(prev_dest__ == 0)
            {
              mst__.push_back(std::make_pair(edge__.m_prev_source, dest__));
              s_prevs__[edge__.m_prev_source - 1] = 0;
            }

          if(prev_source__ != 0 && prev_dest__ != 0)
            mst__.push_back(std::make_pair(edge__.m_prev_source, edge__.m_prev_destination));
        }
    }

  return mst__;
}

Matrix
make_rectilinear_mst(const Matrix& matrix, const std::vector<Index>& terminals)
{
  Shape shape__ = matrix.shape();

  Index mean_idx__{};
  uint32_t min_x__ = shape__.x;
  uint32_t max_x__ = 0;
  double min_mean_distance__ = __DBL_MAX__;

  for(auto t1__ : terminals)
    {
      double mean_distance__ = 0;

      for(auto t2__ : terminals)
        if(t1__ != t2__)
          mean_distance__ += t1__.y > t2__.y ? (t1__.y - t2__.y) : (t2__.y - t1__.y);

      mean_distance__ = mean_distance__ / terminals.size() - 1;

      if(mean_distance__ < min_mean_distance__)
        {
          min_mean_distance__ = mean_distance__;
          mean_idx__ = t1__;
        }

      min_x__ = std::min(t1__.x, min_x__);
      max_x__ = std::max(t1__.x, max_x__);
    }

  Matrix mst_matrix__{ shape__ };

  for(auto t__ : terminals)
    {
      if(t__ != mean_idx__)
        if(t__.y > mean_idx__.y)
          while(t__.y >= mean_idx__.y)
            {
              mst_matrix__[t__] = 1;
              t__ -= { 0, 1 };
            }
        else
          while(t__.y <= mean_idx__.y)
            {
              mst_matrix__[t__] = 1;
              t__ += { 0, 1 };
            }
    }

  for(Index itr__{ min_x__, mean_idx__.y }, end__{ max_x__, mean_idx__.y }; itr__ <= end__; itr__ += { 1, 0 })
    mst_matrix__[itr__] = 1;

  return mst_matrix__;
}

} // namespace gen

#endif