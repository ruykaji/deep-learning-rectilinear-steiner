#ifndef __PROCESS_HPP__
#define __PROCESS_HPP__

#include <queue>
#include <tuple>
#include <vector>

#include "graph.hpp"
#include "matrix.hpp"
#include "utility.hpp"

namespace gen
{

typedef std::tuple<uint8_t, uint8_t, uint8_t> RGB;

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

  static std::pair<Matrix, bool>
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
        std::size_t count = 0;

        // Makes traces in x and y direction through terminal point.
        for(uint32_t z__ = 0; z__ < s_shape.z; ++z__)
          {
            if(matrix[t__ + Index{ 0, 0, z__ }] != TERMINAL_CELL)
              {
                matrix[t__ + Index{ 0, 0, z__ }] = INTERSECTION_VIA_CELL;
              }

            count += m_make_trace(matrix, { t__.x, 0, z__ }, { t__.x, s_shape.y - 1, z__ }, { 0, 1, 0 });
            count += m_make_trace(matrix, { 0, t__.y, z__ }, { s_shape.x - 1, t__.y, z__ }, { 1, 0, 0 });
          }

        if(count == 0)
          {
            return std::make_pair(matrix, false);
          }
      }

    matrix.zero_all(PATH_CELL);

    return std::make_pair(matrix, true);
  }

  static Graph
  make_graph(const Matrix& matrix, const Index& start)
  {
    s_nodes.clear();

    Graph graph__{};

    // Initialize with top left corner as it always is intersection node
    Index itr__ = start;
    s_nodes.push_back(itr__);
    graph__.m_adj.push_back({});
    graph__.m_terminals.insert(1);

    std::queue<Index> queue__{};
    queue__.push(itr__);

    while(!queue__.empty())
      {
        Index front__ = queue__.front();
        queue__.pop();

        std::size_t source_idx__ = find_index(s_nodes.begin(), s_nodes.end(), front__);

        // Searching for intersection or terminal node in x direction.
        itr__ = front__;

        while(itr__.x != s_shape.x - 1)
          {
            itr__ += { 1, 0, 0 };

            std::size_t dest_idx__ = find_index(s_nodes.begin(), s_nodes.end(), itr__);

            if(matrix[itr__] == INTERSECTION_VIA_CELL || matrix[itr__] == INTERSECTION_CELL || matrix[itr__] == TERMINAL_CELL)
              {
                // If node haven't been visited then add it to the queue
                if(dest_idx__ == s_nodes.size())
                  {
                    s_nodes.push_back(itr__);
                    graph__.m_adj.push_back({});

                    queue__.push(itr__);
                  }

                uint32_t distance__ = itr__.x - front__.x;

                // Connect source and destination nodes

                graph__.m_adj[dest_idx__].emplace_back(distance__, dest_idx__ + 1, source_idx__ + 1);
                graph__.m_adj[source_idx__].emplace_back(distance__, source_idx__ + 1, dest_idx__ + 1);

                if(matrix[itr__] == TERMINAL_CELL)
                  {
                    graph__.m_terminals.insert(dest_idx__ + 1);
                  }
              }
          }

        itr__ = front__;

        while(itr__.x != 0)
          {
            itr__ -= { 1, 0, 0 };

            std::size_t dest_idx__ = find_index(s_nodes.begin(), s_nodes.end(), itr__);
            if(matrix[itr__] == INTERSECTION_VIA_CELL || matrix[itr__] == INTERSECTION_CELL || matrix[itr__] == TERMINAL_CELL)
              {
                // If node haven't been visited then add it to the queue
                if(dest_idx__ == s_nodes.size())
                  {
                    s_nodes.push_back(itr__);
                    graph__.m_adj.push_back({});

                    queue__.push(itr__);
                  }

                uint32_t distance__ = front__.x - itr__.x;

                // Connect source and destination nodes

                graph__.m_adj[dest_idx__].emplace_back(distance__, dest_idx__ + 1, source_idx__ + 1);
                graph__.m_adj[source_idx__].emplace_back(distance__, source_idx__ + 1, dest_idx__ + 1);

                if(matrix[itr__] == TERMINAL_CELL)
                  {
                    graph__.m_terminals.insert(dest_idx__ + 1);
                  }
              }
          }

        // Searching for intersection or terminal node in y direction.
        itr__ = front__;

        while(itr__.y != s_shape.y - 1)
          {
            itr__ += { 0, 1, 0 };

            std::size_t dest_idx__ = find_index(s_nodes.begin(), s_nodes.end(), itr__);
            if(matrix[itr__] == INTERSECTION_VIA_CELL || matrix[itr__] == INTERSECTION_CELL || matrix[itr__] == TERMINAL_CELL)
              {
                // If node haven't been visited then add it to the queue
                if(dest_idx__ == s_nodes.size())
                  {
                    s_nodes.push_back(itr__);
                    graph__.m_adj.push_back({});

                    queue__.push(itr__);
                  }

                uint32_t distance__ = itr__.y - front__.y;

                // Connect source and destination nodes

                graph__.m_adj[dest_idx__].emplace_back(distance__, dest_idx__ + 1, source_idx__ + 1);
                graph__.m_adj[source_idx__].emplace_back(distance__, source_idx__ + 1, dest_idx__ + 1);

                if(matrix[itr__] == TERMINAL_CELL)
                  {
                    graph__.m_terminals.insert(dest_idx__ + 1);
                  }
              }
          }

        itr__ = front__;

        while(itr__.y != 0)
          {
            itr__ -= { 0, 1, 0 };

            std::size_t dest_idx__ = find_index(s_nodes.begin(), s_nodes.end(), itr__);

            if(matrix[itr__] == INTERSECTION_VIA_CELL || matrix[itr__] == INTERSECTION_CELL || matrix[itr__] == TERMINAL_CELL)
              {
                // If node haven't been visited then add it to the queue
                if(dest_idx__ == s_nodes.size())
                  {
                    s_nodes.push_back(itr__);
                    graph__.m_adj.push_back({});

                    queue__.push(itr__);
                  }

                uint32_t distance__ = front__.y - itr__.y;

                // Connect source and destination nodes

                graph__.m_adj[dest_idx__].emplace_back(distance__, dest_idx__ + 1, source_idx__ + 1);
                graph__.m_adj[source_idx__].emplace_back(distance__, source_idx__ + 1, dest_idx__ + 1);

                if(matrix[itr__] == TERMINAL_CELL)
                  {
                    graph__.m_terminals.insert(dest_idx__ + 1);
                  }
              }
          }

        // Searching for intersection or terminal node in z direction.
        itr__ = front__;

        while(itr__.z != s_shape.z - 1)
          {
            itr__ += { 0, 0, 1 };

            std::size_t dest_idx__ = find_index(s_nodes.begin(), s_nodes.end(), itr__);

            if(matrix[itr__] == INTERSECTION_VIA_CELL || matrix[itr__] == INTERSECTION_CELL || matrix[itr__] == TERMINAL_CELL)
              {
                // If node haven't been visited then add it to the queue
                if(dest_idx__ == s_nodes.size())
                  {
                    s_nodes.push_back(itr__);
                    graph__.m_adj.push_back({});

                    queue__.push(itr__);
                  }

                uint32_t distance__ = itr__.z - front__.z;

                // Connect source and destination nodes

                graph__.m_adj[dest_idx__].emplace_back(distance__, dest_idx__ + 1, source_idx__ + 1);
                graph__.m_adj[source_idx__].emplace_back(distance__, source_idx__ + 1, dest_idx__ + 1);

                if(matrix[itr__] == TERMINAL_CELL)
                  {
                    graph__.m_terminals.insert(dest_idx__ + 1);
                  }
              }
          }

        itr__ = front__;

        while(itr__.z != 0)
          {
            itr__ -= { 0, 0, 1 };

            std::size_t dest_idx__ = find_index(s_nodes.begin(), s_nodes.end(), itr__);

            if(matrix[itr__] == INTERSECTION_VIA_CELL || matrix[itr__] == INTERSECTION_CELL || matrix[itr__] == TERMINAL_CELL)
              {
                // If node haven't been visited then add it to the queue
                if(dest_idx__ == s_nodes.size())
                  {
                    s_nodes.push_back(itr__);
                    graph__.m_adj.push_back({});

                    queue__.push(itr__);
                  }

                uint32_t distance__ = front__.z - itr__.z;

                // Connect source and destination nodes

                graph__.m_adj[dest_idx__].emplace_back(distance__, dest_idx__ + 1, source_idx__ + 1);
                graph__.m_adj[source_idx__].emplace_back(distance__, source_idx__ + 1, dest_idx__ + 1);

                if(matrix[itr__] == TERMINAL_CELL)
                  {
                    graph__.m_terminals.insert(dest_idx__ + 1);
                  }
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
    Matrix mapped_matrix__{ Shape{ s_shape.x, s_shape.y, 3 } };

    Index itr__{ 0, 0 };

    while(itr__.y != s_shape.y)
      {
        std::vector<uint32_t> values__(6, 0);

        for(uint32_t z__ = 0; z__ < s_shape.z; ++z__)
          {
            for(std::size_t m__ = 0; m__ < matrices.size(); ++m__)
              {
                if(matrices[m__][itr__ + Index{ 0, 0, z__ }] != 0)
                  {
                    values__[z__ * 2] = m__;
                    values__[z__ * 2 + 1] = matrices[m__][itr__ + Index{ 0, 0, z__ }];
                    break;
                  }
              }
          }

        RGB rgb__ = map_integers_to_float(values__);

        mapped_matrix__[itr__] = std::get<0>(rgb__);
        mapped_matrix__[itr__ + Index{ 0, 0, 1 }] = std::get<1>(rgb__);
        mapped_matrix__[itr__ + Index{ 0, 0, 2 }] = std::get<2>(rgb__);

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
  static int8_t
  m_make_trace(Matrix& matrix, Index first, Index last, Index step)
  {
    if(matrix[first] == INTERSECTION_CELL || matrix[first] == INTERSECTION_VIA_CELL)
      return 0;

    for(Index i__ = first; i__ <= last; i__ += step)
      {
        if(matrix[i__] == PATH_CELL)
          {
            return 0;
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

    return 1;
  }

  static RGB
  map_integers_to_float(const std::vector<uint32_t>& values)
  {
    uint8_t r = values[0] * 10 + values[1];
    uint8_t g = values[2] * 10 + values[3];
    uint8_t b = values[4] * 10 + values[5];

    return std::make_tuple(r, g, b);
  }
};
}; // namespace gen

#endif