#ifndef __ALGORITHMS_HPP__
#define __ALGORITHMS_HPP__

#include <cstdint>
#include <vector>

#include "Include/Graph.hpp"
#include "Include/Matrix.hpp"

namespace algorithms
{

/**
 * @brief Finds MST using Dijkstra and Kruskal methods. Greedy version.
 *
 * @param graph The graph to use to find MST.
 * @return std::vector<std::pair<uint32_t, uint32_t>>
 */
std::vector<std::pair<uint32_t, uint32_t>>
dijkstra_kruskal_greedy(const graph::Graph& graph);

} // namespace algorithms

#endif
