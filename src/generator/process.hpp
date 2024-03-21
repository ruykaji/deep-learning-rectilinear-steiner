#ifndef __PROCESS_HPP__
#define __PROCESS_HPP__

#include <algorithm>
#include <random>
#include <unordered_set>
#include <utility>
#include <vector>

#include "graph.hpp"
#include "matrix.hpp"
#include "utility.hpp"

namespace gen
{

class Process
{
private:
  typedef Matrix_index index;
  typedef Matrix_shape shape;

private:
  static constexpr uint8_t mcs_edge = 2;
  static constexpr uint8_t mcs_terminal = 1;
  static constexpr uint8_t mcs_intersection = 4;

private:
  inline static Graph ms_graph{};
  inline static std::vector<Matrix_index> ms_nodes{};

public:
  template <typename Tp>
  static std::pair<Graph, std::vector<Matrix_index>>
  propagate(Matrix<Tp>& matrix, const std::vector<Matrix_index>& terminals)
  {
    shape shape__ = matrix.shape();

    for(auto terminal__ : terminals)
      {
        m_propagate(matrix, { terminal__.x, 0 }, { terminal__.x, shape__.y - 1 }, { 0, 1 });
        m_propagate(matrix, { 0, terminal__.y }, { shape__.x - 1, terminal__.y }, { 1, 0 });
      }

    m_propagate(matrix, { 0, 0 }, { 0, shape__.y - 1 }, { 0, 1 });
    m_propagate(matrix, { shape__.x - 1, 0 }, { shape__.x - 1, shape__.y - 1 }, { 0, 1 });
    m_propagate(matrix, { 0, 0 }, { shape__.x - 1, 0 }, { 1, 0 }, true);
    m_propagate(matrix, { 0, shape__.y - 1 }, { shape__.x - 1, shape__.y - 1 }, { 1, 0 }, true);

    return { std::move(ms_graph), std::move(ms_nodes) };
  }

private:
  template <typename Tp>
  static void
  m_propagate(Matrix<Tp>& matrix, index first, index last, index step, bool force_intersect = false)
  {
    if(matrix[first] == mcs_intersection && !force_intersect)
      return;

    std::size_t last_idx__ = find_index(ms_nodes.begin(), ms_nodes.end(), first);

    if(matrix[first] != mcs_intersection)
      {
        matrix[first] = mcs_intersection;
        matrix[last] = mcs_intersection;
      }
    else
      first += step;

    for(; first <= last; first += step)
      {
        auto& value__ = matrix[first];

        if(value__ == 0)
          value__ = mcs_edge;
        else
          {
            std::size_t idx__ = find_index(ms_nodes.begin(), ms_nodes.end(), first);

            if(value__ != mcs_terminal)
              value__ = mcs_intersection;
            else
              ms_graph.terminals.insert(idx__);

            if(idx__ != last_idx__)
              {
                if(idx__ == ms_nodes.size())
                  {
                    ms_nodes.push_back(first);
                    ms_graph.adj.push_back({ { 0, ms_nodes.size() - 1, last_idx__ } });
                  }
                else
                  ms_graph.adj[idx__].emplace_back(0, idx__, last_idx__);

                ms_graph.adj[last_idx__].emplace_back(0, last_idx__, idx__);

                last_idx__ = idx__;
              }
            else
              {
                ms_nodes.push_back(first);
                ms_graph.adj.push_back({});
              }
          }
      }
  }
};

}; // namespace gen

#endif