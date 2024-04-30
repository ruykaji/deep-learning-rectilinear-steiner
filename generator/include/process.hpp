#ifndef __PROCESS_HPP__
#define __PROCESS_HPP__

#include <queue>
#include <vector>

#include "graph.hpp"
#include "matrix.hpp"
#include "utility.hpp"

namespace gen
{

class Process
{
  static constexpr uint8_t TRACE_CELL = 2;
  static constexpr uint8_t TERMINAL_CELL = 1;
  static constexpr uint8_t INTERSECTION_CELL = 4;

public:
  inline static std::vector<Index> s_nodes{};
  inline static Shape s_shape{};

  static void
  set_shape(Shape shape)
  {
    s_shape = shape;
  }

  static Matrix
  propagate(const std::vector<Index>& terminals)
  {
    Matrix matrix__(s_shape);

    // Sets terminals points on matrix
    for(auto t__ : terminals)
      matrix__[t__] = TERMINAL_CELL;

    // Makes traces on matrix's borders.
    m_make_trace(matrix__, { 0, 0 }, { 0, s_shape.y - 1 }, { 0, 1 });
    m_make_trace(matrix__, { s_shape.x - 1, 0 }, { s_shape.x - 1, s_shape.y - 1 }, { 0, 1 });
    m_make_trace(matrix__, { 0, 0 }, { s_shape.x - 1, 0 }, { 1, 0 });
    m_make_trace(matrix__, { 0, s_shape.y - 1 }, { s_shape.x - 1, s_shape.y - 1 }, { 1, 0 });

    for(auto t__ : terminals)
      {
        // Makes traces in x and y direction through terminal point.
        m_make_trace(matrix__, { t__.x, 0 }, { t__.x, s_shape.y - 1 }, { 0, 1 });
        m_make_trace(matrix__, { 0, t__.y }, { s_shape.x - 1, t__.y }, { 1, 0 });
      }

    return matrix__;
  }

  static Graph
  make_graph(Matrix& matrix)
  {
    s_nodes.clear();

    Graph graph__{};

    // Initialize with top left corner as it always is intersection node
    Index itr__{ 0, 0 };
    s_nodes.push_back(itr__);
    graph__.m_adj.push_back({});

    std::queue<Index> queue__{};
    queue__.push(itr__);

    while(!queue__.empty())
      {
        Index front__ = queue__.front();
        std::size_t source_idx__ = find_index(s_nodes.begin(), s_nodes.end(), front__);

        // Searching for intersection or terminal node in x direction.
        Index x_itr__ = front__;

        while(true)
          {
            x_itr__ += { 1, 0 };

            // Break if reached border of a matrix;
            if(x_itr__.x == s_shape.x)
              break;

            if(matrix[x_itr__] == INTERSECTION_CELL || matrix[x_itr__] == TERMINAL_CELL)
              {
                std::size_t dest_idx__ = find_index(s_nodes.begin(), s_nodes.end(), x_itr__);

                // If node haven't been visited then add it to the queue
                if(dest_idx__ == s_nodes.size())
                  {
                    s_nodes.push_back(x_itr__);
                    graph__.m_adj.push_back({});

                    queue__.push(x_itr__);
                  }

                uint32_t distance__ = x_itr__.x - front__.x - 1;

                // Connect source and destination nodes
                graph__.m_adj[dest_idx__].emplace_back(distance__, dest_idx__ + 1, source_idx__ + 1);
                graph__.m_adj[source_idx__].emplace_back(distance__, source_idx__ + 1, dest_idx__ + 1);

                if(matrix[x_itr__] == TERMINAL_CELL)
                  graph__.m_terminals.insert(dest_idx__ + 1);

                break;
              }
          }

        // Searching for intersection or terminal node in y direction.
        Index y_itr__ = front__;

        while(true)
          {
            y_itr__ += { 0, 1 };

            // Break if reached border of a matrix;
            if(y_itr__.y == s_shape.y)
              break;

            if(matrix[y_itr__] == INTERSECTION_CELL || matrix[y_itr__] == TERMINAL_CELL)
              {
                std::size_t dest_idx__ = find_index(s_nodes.begin(), s_nodes.end(), y_itr__);

                // If node haven't been visited then add it to the queue
                if(dest_idx__ == s_nodes.size())
                  {
                    s_nodes.push_back(y_itr__);
                    graph__.m_adj.push_back({});

                    queue__.push(y_itr__);
                  }

                uint32_t distance__ = y_itr__.y - front__.y - 1;

                // Connect source and destination nodes
                graph__.m_adj[dest_idx__].emplace_back(distance__, dest_idx__ + 1, source_idx__ + 1);
                graph__.m_adj[source_idx__].emplace_back(distance__, source_idx__ + 1, dest_idx__ + 1);

                if(matrix[y_itr__] == TERMINAL_CELL)
                  graph__.m_terminals.insert(dest_idx__ + 1);

                break;
              }
          }

        queue__.pop();
      }

    return graph__;
  }

  static Matrix
  make_matrix(const MST& mst)
  {
    Matrix mst_matrix__(s_shape);

    for(auto [first__, second__] : mst)
      {
        gen::Index first_idx__ = s_nodes[first__ - 1];
        gen::Index second_idx__ = s_nodes[second__ - 1];

        if(second_idx__ <= first_idx__)
          std::swap(first_idx__, second_idx__);

        gen::Index step__{};

        if(first_idx__.x == second_idx__.x)
          step__ = { 0, 1 };
        else
          step__ = { 1, 0 };

        for(; first_idx__ <= second_idx__; first_idx__ += step__)
          mst_matrix__[first_idx__] = 1;
      }

    return mst_matrix__;
  }

private:
  static void
  m_make_trace(Matrix& matrix, Index first, Index last, Index step)
  {
    if(matrix[first] == INTERSECTION_CELL)
      return;

    for(; first <= last; first += step)
      {
        auto& value__ = matrix[first];

        if(value__ == 0)
          value__ = TRACE_CELL;
        else if(value__ != TERMINAL_CELL)
          value__ = INTERSECTION_CELL;
      }
  }
};

}; // namespace gen

#endif