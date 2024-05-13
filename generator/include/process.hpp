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
  static constexpr uint8_t PATH_CELL = 1;

  static constexpr uint8_t TRACE_CELL = 2;
  static constexpr uint8_t TERMINAL_CELL = 3;
  static constexpr uint8_t INTERSECTION_CELL = 4;
  static constexpr uint8_t INTERSECTION_VIA_CELL = 5;

public:
  inline static std::vector<Index> s_nodes{};
  inline static Shape s_shape{};

  static void
  set_shape(Shape shape)
  {
    s_shape = shape;
  }

  static Matrix
  propagate(Matrix matrix, const std::vector<std::vector<Index>>& nets, const std::size_t i)
  {

    for(std::size_t j__ = 0, end__ = nets.size(); j__ < end__; ++j__)
      {
        if(j__ != i)
          {
            for(const auto& t__ : nets[j__])
              {
                matrix[t__] = gen::Process::PATH_CELL;
              }
          }
        else
          {
            for(const auto& t__ : nets[i])
              {
                matrix[t__] = gen::Process::TERMINAL_CELL;
              }
          }
      }

    for(uint32_t z__ = 0; z__ < s_shape.z; ++z__)
      {
        // Makes traces on matrix's borders.
        m_make_trace(matrix, { 0, 0, z__ }, { 0, s_shape.y - 1, z__ }, { 0, 1, 0 });
        m_make_trace(matrix, { s_shape.x - 1, 0, z__ }, { s_shape.x - 1, s_shape.y - 1, z__ }, { 0, 1, 0 });

        m_make_trace(matrix, { 0, 0, z__ }, { s_shape.x - 1, 0, z__ }, { 1, 0, 0 });
        m_make_trace(matrix, { 0, s_shape.y - 1, z__ }, { s_shape.x - 1, s_shape.y - 1, z__ }, { 1, 0, 0 });
      }

    for(auto t__ : nets[i])
      {
        // Makes traces in x and y direction through terminal point.
        for(uint32_t z__ = 0; z__ < s_shape.z; ++z__)
          {
            if(matrix[t__ + Index{ 0, 0, z__ }] != TERMINAL_CELL)
              {
                matrix[t__ + Index{ 0, 0, z__ }] = INTERSECTION_VIA_CELL;
              }

            m_make_trace(matrix, { t__.x, 0, z__ }, { t__.x, s_shape.y - 1, z__ }, { 0, 1, 0 });
            m_make_trace(matrix, { 0, t__.y, z__ }, { s_shape.x - 1, t__.y, z__ }, { 1, 0, 0 });
          }
      }

    matrix.zero_all(PATH_CELL);

    return matrix;
  }

  static Graph
  make_graph(const Matrix& matrix)
  {
    s_nodes.clear();

    Graph graph__{};

    // Initialize with top left corner as it always is intersection node
    Index itr__{ 0, 0, 0 };
    s_nodes.push_back(itr__);
    graph__.m_adj.push_back({});

    std::queue<Index> queue__{};
    queue__.push(itr__);

    while(!queue__.empty())
      {
        Index front__ = queue__.front();
        queue__.pop();

        std::size_t source_idx__ = find_index(s_nodes.begin(), s_nodes.end(), front__);

        // Searching for intersection or terminal node in x direction.
        Index x_itr__ = front__;

        while(true)
          {
            x_itr__ += { 1, 0, 0 };

            // Break if reached border of a matrix;
            if(x_itr__.x == s_shape.x)
              break;

            if(matrix[x_itr__] == INTERSECTION_VIA_CELL || matrix[x_itr__] == INTERSECTION_CELL || matrix[x_itr__] == TERMINAL_CELL)
              {
                std::size_t dest_idx__ = find_index(s_nodes.begin(), s_nodes.end(), x_itr__);

                // If node haven't been visited then add it to the queue
                if(dest_idx__ == s_nodes.size())
                  {
                    s_nodes.push_back(x_itr__);
                    graph__.m_adj.push_back({});

                    queue__.push(x_itr__);
                  }

                uint32_t distance__ = x_itr__.x - front__.x;

                // Connect source and destination nodes
                graph__.m_adj[dest_idx__].emplace_back(distance__, dest_idx__ + 1, source_idx__ + 1);
                graph__.m_adj[source_idx__].emplace_back(distance__, source_idx__ + 1, dest_idx__ + 1);

                if(matrix[x_itr__] == TERMINAL_CELL)
                  {
                    graph__.m_terminals.insert(dest_idx__ + 1);
                  }

                break;
              }
          }

        // Searching for intersection or terminal node in y direction.
        Index y_itr__ = front__;

        while(true)
          {
            y_itr__ += { 0, 1, 0 };

            // Break if reached border of a matrix;
            if(y_itr__.y == s_shape.y)
              break;

            if(matrix[y_itr__] == INTERSECTION_VIA_CELL || matrix[y_itr__] == INTERSECTION_CELL || matrix[y_itr__] == TERMINAL_CELL)
              {
                std::size_t dest_idx__ = find_index(s_nodes.begin(), s_nodes.end(), y_itr__);

                // If node haven't been visited then add it to the queue
                if(dest_idx__ == s_nodes.size())
                  {
                    s_nodes.push_back(y_itr__);
                    graph__.m_adj.push_back({});

                    queue__.push(y_itr__);
                  }

                uint32_t distance__ = y_itr__.y - front__.y;

                // Connect source and destination nodes
                graph__.m_adj[dest_idx__].emplace_back(distance__, dest_idx__ + 1, source_idx__ + 1);
                graph__.m_adj[source_idx__].emplace_back(distance__, source_idx__ + 1, dest_idx__ + 1);

                if(matrix[y_itr__] == TERMINAL_CELL)
                  {
                    graph__.m_terminals.insert(dest_idx__ + 1);
                  }

                break;
              }
          }

        // Searching for intersection or terminal node in z direction.
        Index z_itr__ = front__;

        while(true)
          {
            z_itr__ += { 0, 0, 1 };

            // Break if reached border of a matrix;
            if(z_itr__.z == s_shape.z)
              break;

            if(matrix[z_itr__] == INTERSECTION_VIA_CELL || matrix[z_itr__] == INTERSECTION_CELL || matrix[z_itr__] == TERMINAL_CELL)
              {
                std::size_t dest_idx__ = find_index(s_nodes.begin(), s_nodes.end(), z_itr__);

                // If node haven't been visited then add it to the queue
                if(dest_idx__ == s_nodes.size())
                  {
                    s_nodes.push_back(z_itr__);
                    graph__.m_adj.push_back({});

                    queue__.push(z_itr__);
                  }

                uint32_t distance__ = z_itr__.z - front__.z + (s_shape.x + s_shape.y) / 4;

                // Connect source and destination nodes
                graph__.m_adj[dest_idx__].emplace_back(distance__, dest_idx__ + 1, source_idx__ + 1);
                graph__.m_adj[source_idx__].emplace_back(distance__, source_idx__ + 1, dest_idx__ + 1);

                if(matrix[z_itr__] == TERMINAL_CELL)
                  {
                    graph__.m_terminals.insert(dest_idx__ + 1);
                  }

                break;
              }
          }
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

        if(first_idx__.x == second_idx__.x && first_idx__.y == second_idx__.y)
          step__ = { 0, 0, 1 };
        else if(first_idx__.x == second_idx__.x)
          step__ = { 0, 1, 0 };
        else if(first_idx__.y == second_idx__.y)
          step__ = { 1, 0, 0 };

        for(; first_idx__ <= second_idx__; first_idx__ += step__)
          mst_matrix__[first_idx__] = PATH_CELL;
      }

    return mst_matrix__;
  }

  static Matrix
  map_nets(const std::vector<gen::Matrix>& matrices)
  {
    Matrix mapped_matrix__{ Shape{ s_shape.x, s_shape.y, 1 } };

    Index itr__{ 0, 0 };

    while(itr__.y != s_shape.y)
      {
        std::vector<uint32_t> values__(s_shape.z * 2, 0);

        for(uint32_t z__ = 0; z__ < s_shape.z; ++z__)
          {
            for(std::size_t m__ = 0; m__ < matrices.size(); ++m__)
              {
                if(matrices[m__][itr__ + Index{ 0, 0, z__ }] != 0)
                  {
                    values__[z__ * 2] = m__ + 1;
                    values__[z__ * 2 + 1] = matrices[m__][itr__ + Index{ 0, 0, z__ }] - 1;
                    break;
                  }
              }
          }

        mapped_matrix__[itr__] = map_integers_to_float(values__);
        itr__.x += 1;

        if(itr__.x == s_shape.x)
          {
            itr__.x = 0;
            itr__.y += 1;
          }
      }

    return mapped_matrix__;
  }

private:
  static void
  m_make_trace(Matrix& matrix, Index first, Index last, Index step)
  {
    if(matrix[first] == INTERSECTION_CELL || matrix[first] == INTERSECTION_VIA_CELL)
      return;

    for(Index i__ = first; i__ <= last; i__ += step)
      {
        if(matrix[i__] == PATH_CELL)
          {
            return;
          }
      }

    for(; first <= last; first += step)
      {
        auto& value__ = matrix[first];

        if(value__ == 0)
          {
            value__ = TRACE_CELL;
          }
        else if(value__ != TERMINAL_CELL && value__ != INTERSECTION_VIA_CELL)
          {
            value__ = INTERSECTION_CELL;
          }
      }
  }

  static uint32_t
  map_integers_to_float(const std::vector<uint32_t>& values)
  {
    if(values.size() == 4)
      {
        uint32_t adjusted_a = values[0];
        uint32_t adjusted_b = values[1];
        uint32_t adjusted_c = values[2];
        uint32_t adjusted_d = values[3];

        uint32_t unique_int = adjusted_a * 125 + adjusted_b * 25 + adjusted_c * 5 + adjusted_d * 1; // 5^3 = 125, 5^2 = 25, 5^1 = 5, 5^0 = 1
        return unique_int;
      }
    else
      {
        uint32_t adjusted_a = values[0];
        uint32_t adjusted_b = values[1];

        uint32_t unique_int = adjusted_a * 125 + adjusted_b * 25;
        return unique_int;
      }
  }
};

}; // namespace gen

#endif