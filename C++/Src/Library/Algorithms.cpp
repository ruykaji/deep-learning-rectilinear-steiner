#include <algorithm>
#include <functional>
#include <iostream>
#include <numeric>
#include <queue>
#include <set>
#include <thread>
#include <unordered_map>

#include "Include/Algorithms.hpp"
#include "Include/Matrix.hpp"
#include "Include/Types.hpp"

namespace algorithms
{

namespace details
{

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

/**
 * @brief Handles information about about single source to multiple destinations path aka branch.
 *
 */
struct PathBranch
{
  uint32_t                 m_weight;
  uint32_t                 m_source;
  std::set<uint32_t>       m_destination;
  std::vector<graph::Edge> m_path;

  PathBranch(const PathInfo& path)
      : m_weight(path.m_weight), m_source(path.m_source), m_destination(), m_path(path.m_path)
  {
    m_destination.insert(path.m_destination);
  }
};

/**
 * @brief Merges branch with simple path if they have common edges.
 *
 * @param lhs The branch to merge in.
 * @param rhs The path to merge with.
 * @return true
 * @return false
 */
bool
merge_edge(PathBranch& lhs, const PathInfo& rhs)
{
  if((lhs.m_destination.count(rhs.m_destination) && lhs.m_source == rhs.m_source) || (lhs.m_destination.count(rhs.m_source) && lhs.m_source == rhs.m_destination))
    {
      return false;
    }

  std::vector<graph::Edge> diff_edges;

  for(const auto& rhs_edge : rhs.m_path)
    {
      bool is_found = false;

      for(const auto& lhs_edge : lhs.m_path)
        {
          if((lhs_edge.m_destination == rhs_edge.m_destination && lhs_edge.m_source == rhs_edge.m_source)
             || (lhs_edge.m_destination == rhs_edge.m_source && lhs_edge.m_source == rhs_edge.m_destination))
            {
              lhs.m_weight -= rhs_edge.m_weight;
              is_found = true;
              break;
            }
        }

      if(!is_found)
        {
          diff_edges.emplace_back(rhs_edge);
        }
    }

  if(diff_edges.size() == rhs.m_path.size())
    {
      return false;
    }

  lhs.m_weight += rhs.m_weight;

  if(lhs.m_source != rhs.m_destination)
    {
      lhs.m_destination.insert(rhs.m_destination);
    }

  if(lhs.m_source != rhs.m_source)
    {
      lhs.m_destination.insert(rhs.m_source);
    }

  lhs.m_path.insert(lhs.m_path.end(), std::make_move_iterator(diff_edges.begin()), std::make_move_iterator(diff_edges.end()));

  return true;
}

/**
 * @brief Merges brach with another branch if they have common edges.
 *
 * @param lhs The branch to merge in.
 * @param rhs The branch to merge with.
 * @return true
 * @return false
 */
bool
merge_branch(PathBranch& lhs, const PathBranch& rhs)
{
  if((lhs.m_destination.count(rhs.m_source) || lhs.m_source == rhs.m_source) && std::includes(lhs.m_path.begin(), lhs.m_path.end(), rhs.m_path.begin(), rhs.m_path.end()))
    {
      return false;
    }

  if((rhs.m_destination.count(lhs.m_source) || lhs.m_source == rhs.m_source) && std::includes(rhs.m_path.begin(), rhs.m_path.end(), lhs.m_path.begin(), lhs.m_path.end()))
    {
      return false;
    }

  std::vector<graph::Edge> diff_edges;
  diff_edges.reserve(rhs.m_path.size());

  /** Trying to find a common edge */
  for(const auto& rhs_edge : rhs.m_path)
    {
      bool is_found = false;

      for(const auto& lhs_edge : lhs.m_path)
        {
          if((lhs_edge.m_destination == rhs_edge.m_destination && lhs_edge.m_source == rhs_edge.m_source)
             || (lhs_edge.m_destination == rhs_edge.m_source && lhs_edge.m_source == rhs_edge.m_destination))
            {
              lhs.m_weight -= rhs_edge.m_weight;
              is_found = true;
              break;
            }
        }

      if(!is_found)
        {
          diff_edges.emplace_back(rhs_edge);
        }
    }

  if(diff_edges.size() == rhs.m_path.size())
    {
      return false;
    }

  lhs.m_weight += rhs.m_weight;
  lhs.m_destination.insert(rhs.m_destination.begin(), rhs.m_destination.end());

  if(lhs.m_source != rhs.m_source)
    {
      lhs.m_destination.insert(rhs.m_source);
    }

  lhs.m_path.insert(lhs.m_path.end(), std::make_move_iterator(diff_edges.begin()), std::make_move_iterator(diff_edges.end()));

  return true;
}

} // namespace details

std::vector<std::pair<uint32_t, uint32_t>>
dijkstra_kruskal(const graph::Graph& graph)
{
  const auto&                                 adj       = graph.get_adj();
  const auto&                                 terminals = graph.get_terminals();
  const std::vector<uint32_t>                 terminals_v(terminals.begin(), terminals.end());
  const std::size_t                           num_vertices  = adj.size();
  const std::size_t                           num_terminals = terminals.size();

  /** Number of sort and full paths have to be equal */
  std::vector<std::vector<details::PathInfo>> paths_info;
  paths_info.reserve(num_vertices - 1);

  for(uint32_t i = 0; i < num_terminals; ++i)
    {
      const uint32_t                                                                        src = terminals_v[i];

      std::vector<uint32_t>                                                                 dist(num_vertices, std::numeric_limits<uint32_t>::max());
      std::vector<uint32_t>                                                                 weights(num_vertices, 0);
      std::vector<std::vector<uint32_t>>                                                    prev(num_vertices);
      std::priority_queue<graph::Edge, std::vector<graph::Edge>, std::greater<graph::Edge>> queue;

      dist[src - 1] = 0;
      queue.emplace(0, src);

      while(!queue.empty())
        {
          const uint32_t u      = queue.top().m_source;
          uint32_t       dist_u = queue.top().m_destination;
          queue.pop();

          if(dist_u > dist[u - 1])
            {
              continue;
            }

          for(const auto& edge : adj[u - 1])
            {
              uint32_t v   = edge.m_destination;
              uint32_t alt = dist[u - 1] + edge.m_weight;

              if(alt < dist[v - 1])
                {
                  dist[v - 1] = alt;
                  prev[v - 1].clear();
                  prev[v - 1].push_back(u);
                  weights[v - 1] = edge.m_weight;
                  queue.emplace(alt, v);
                }
              else if(alt == dist[v - 1])
                {
                  prev[v - 1].push_back(u);
                }
            }
        }

      /** Get short and full paths */
      for(uint32_t j = i + 1; j < num_terminals; ++j)
        {
          const uint32_t                    dst = terminals_v[j];

          std::queue<std::vector<uint32_t>> prev_queue; /** TODO: Replace with queue of graph::Edge */
          prev_queue.push({ dst });

          paths_info.push_back({});
          std::vector<details::PathInfo>& last_pi = paths_info.back();

          while(!prev_queue.empty())
            {
              std::vector<uint32_t> current_path = std::move(prev_queue.front());
              prev_queue.pop();

              const uint32_t current_node = current_path.back();

              if(current_node == src)
                {
                  std::vector<graph::Edge> current_path_as_edges;

                  for(std::size_t k = 0, end = current_path.size() - 1; k < end; ++k)
                    {
                      current_path_as_edges.emplace_back(weights[current_path[k] - 1], current_path[k + 1], current_path[k]);
                    }

                  last_pi.emplace_back(dist[dst - 1], src, dst, std::move(current_path_as_edges));
                }
              else
                {
                  for(uint32_t prev_node : prev[current_node - 1])
                    {
                      std::vector<uint32_t> new_path = current_path;
                      new_path.emplace_back(prev_node);
                      prev_queue.emplace(std::move(new_path));
                    }
                }
            }
        }
    }

  std::vector<std::size_t>                indices(paths_info.size(), 0);
  std::vector<uint32_t>                   parent(num_vertices);

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

  const std::function<bool(uint32_t, uint32_t)> check_union = [&parent, &find](uint32_t u, uint32_t v) {
    uint32_t root_u = find(u);
    uint32_t root_v = find(v);

    return root_u == root_u;
  };

  std::vector<graph::Edge> final_mst;
  std::size_t              min_mst_weight = std::numeric_limits<std::size_t>::max();

  std::vector<uint32_t>    merge_combination(paths_info.size(), 0);
  std::size_t              step_counter = 0;

  /** This is mega greedy algorithm that trying all possible combinations to find most minimal weighted mst */
  while(true)
    {
      std::vector<std::reference_wrapper<details::PathInfo>> pi_combination;

      for(std::size_t i = 0; i < paths_info.size(); ++i)
        {
          pi_combination.emplace_back(paths_info[i][indices[i]]);
        }

      std::sort(pi_combination.begin(), pi_combination.end(), [](const auto& lhs, const auto& rhs) { return lhs.get().m_weight < rhs.get().m_weight; });
      std::iota(merge_combination.begin(), merge_combination.end(), 0);

      do
        {
          std::vector<uint32_t>                                  comb_indices = merge_combination;
          std::vector<std::reference_wrapper<details::PathInfo>> paths        = pi_combination;

          std::vector<details::PathBranch>                       branches;
          branches.reserve(pi_combination.size() / 2);

          details::PathBranch branch(pi_combination[comb_indices[0]].get());
          comb_indices.erase(comb_indices.begin());
          paths.erase(paths.begin());

          /** Working with current selected branch, trying to find any possible merges */
          do
            {
              bool is_any_merged = branches.size() == 0 && paths.size() == 0;

              /** Trying to merge current branch with remaining paths */
              for(std::size_t j = 0, end_j = paths.size(); j < end_j; ++j)
                {
                  if(details::merge_edge(branch, paths[j]))
                    {
                      comb_indices.erase(comb_indices.begin() + j);
                      paths.erase(paths.begin() + j);
                      is_any_merged = true;
                      break;
                    }
                }

              /** Trying to merge current branch with other brunches */
              for(std::size_t j = 0, end_j = branches.size(); j < end_j; ++j)
                {
                  if(details::merge_branch(branch, branches[j]))
                    {
                      branches.erase(branches.begin() + j);
                      is_any_merged = true;
                      break;
                    }
                }

              if(!is_any_merged)
                {
                  break;
                }

              std::vector<graph::Edge> dijkstra_mst;
              std::iota(parent.begin(), parent.end(), 0);

              const auto process_branches = [&](const details::PathBranch& b) {
                bool can_be_united_all = true;

                for(const auto& dst : b.m_destination)
                  {
                    if(!check_union(b.m_source, dst))
                      {
                        can_be_united_all = false;
                        break;
                      }
                  }

                if(can_be_united_all)
                  {
                    for(const auto& dst : b.m_destination)
                      {
                        union_sets(b.m_source, dst);
                      }

                    dijkstra_mst.insert(dijkstra_mst.end(), b.m_path.begin(), b.m_path.end());
                  }
              };

              /** First we need to find MST of Djikstra paths */
              /** It essential to add to mst branches first as they may have way higher weight then any other path */
              process_branches(branch);

              for(const auto& b : branches)
                {
                  process_branches(b);
                }

              for(const auto& path_ref : paths)
                {
                  const auto& path = path_ref.get();

                  if(union_sets(path.m_source, path.m_destination))
                    {
                      dijkstra_mst.insert(dijkstra_mst.end(), path.m_path.begin(), path.m_path.end());
                    }
                }

              /** After we found Djikstra MST we can proceed to finding the real one */
              std::vector<graph::Edge> mst;
              std::size_t              mst_weight = 0;

              std::iota(parent.begin(), parent.end(), 0);
              std::sort(dijkstra_mst.begin(), dijkstra_mst.end(), [](const auto& lhs, const auto& rhs) { return lhs.m_weight < rhs.m_weight; });

              for(const auto& edge : dijkstra_mst)
                {
                  if(union_sets(edge.m_source, edge.m_destination))
                    {
                      mst.push_back(edge);
                      mst_weight += edge.m_weight;
                    }
                }

              if(mst_weight < min_mst_weight)
                {
                  min_mst_weight = mst_weight;
                  final_mst      = std::move(mst);
                }
            }
          while(comb_indices.size() != 0);

          /** If selected branch is actual branch add it to the array */
          if(branch.m_destination.size() > 1)
            {
              branches.emplace_back(std::move(branch));
              std::sort(branches.begin(), branches.end(), [](const details::PathBranch& lhs, const details::PathBranch& rhs) { return lhs.m_destination.size() > rhs.m_destination.size(); });
            }

          if(++step_counter > 1000)
            {
              std::this_thread::sleep_for(std::chrono::milliseconds(1));
              step_counter = 0;
            }
        }
      while(std::next_permutation(merge_combination.begin(), merge_combination.end()));

      std::size_t k = 0;

      while(k < indices.size())
        {
          indices[k]++;

          if(indices[k] < paths_info[k].size())
            {
              break;
            }
          else
            {
              indices[k] = 0;
              ++k;
            }
        }

      if(k == indices.size())
        {
          break;
        }
    }

  std::vector<std::pair<uint32_t, uint32_t>> paired_edges_mst;

  for(const auto& edge : final_mst)
    {
      paired_edges_mst.emplace_back(edge.m_source, edge.m_destination);
    }

  return paired_edges_mst;
}

} // namespace algorithms
