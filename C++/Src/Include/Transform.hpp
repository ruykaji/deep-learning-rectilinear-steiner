#ifndef __TRANSFORM_HPP__
#define __TRANSFORM_HPP__

#include "Include/Graph.hpp"
#include "Include/Matrix.hpp"
#include "Include/Types.hpp"

namespace transform
{

/**
 * @brief Constructs a graph from a given matrix.
 *
 * @param matrix The matrix to use to construct the graph.
 * @return std::pair<graph::Graph, std::vector<std::tuple<uint8_t, uint8_t, uint8_t>>>
 */
std::pair<graph::Graph, std::vector<std::tuple<uint8_t, uint8_t, uint8_t>>>
matrix_to_graph(const matrix::Matrix& matrix);

/**
 * @brief Constructs a matrix from a given graph.
 *
 * @param shape
 * @param mst
 * @param nodes
 * @return matrix::Matrix
 */
matrix::Matrix
mst_to_matrix(const matrix::Shape shape, const std::vector<std::pair<uint32_t, uint32_t>>& mst, const std::vector<std::tuple<uint8_t, uint8_t, uint8_t>>& nodes);

} // namespace algo

#endif