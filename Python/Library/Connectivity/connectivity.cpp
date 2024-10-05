#include <queue>
#include <unordered_set>
#include <vector>

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

struct Cell
{
  int32_t x;
  int32_t y;
  int32_t z;

  struct Hash
  {
    std::size_t
    operator()(const Cell& t) const
    {
      std::size_t h1 = std::hash<int32_t>()(t.x);
      std::size_t h2 = std::hash<int32_t>()(t.y);
      std::size_t h3 = std::hash<int32_t>()(t.z);

      return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
  };

  friend bool
  operator==(const Cell& lhs, const Cell& rhs)
  {
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
  }

  friend bool
  operator!=(const Cell& lhs, const Cell& rhs)
  {
    return !(lhs == rhs);
  }
};

bool
is_valid(int32_t x, int32_t y, int32_t z, const py::buffer_info& buf)
{
  return (x >= 0 && x < buf.shape[2] && y >= 0 && y < buf.shape[3] && z >= 0 && z < buf.shape[1]);
}

double
single_net_check_connectivity(py::array_t<int32_t> matrices, py::array_t<int32_t> target_cells)
{
  const py::buffer_info matrix_buf = matrices.request();
  const py::buffer_info target_buf = target_cells.request();

  if(matrix_buf.ndim != 4)
    {
      throw std::runtime_error("Input matrices should be a 4-D array (batch, x, y, z)");
    }

  if(target_buf.ndim != 3 || target_buf.shape[2] != 3)
    {
      throw std::runtime_error("Target cells should be a 3-D array (batch, num_targets, 3)");
    }

  if(matrix_buf.shape[0] != target_buf.shape[0])
    {
      throw std::runtime_error("Batch size of matrices and target cells must match");
    }

  double result = 0.0;

  for(ssize_t batch_index = 0, batch_size = matrix_buf.shape[0]; batch_index < batch_size; ++batch_index)
    {
      std::unordered_set<Cell, Cell::Hash> target_set;
      const int32_t* const                 target_ptr = static_cast<int32_t*>(target_buf.ptr) + batch_index * target_buf.shape[1] * 3;

      for(ssize_t i = 0, end = target_buf.shape[1]; i < end; ++i)
        {
          int32_t x = target_ptr[i * 3 + 0];
          int32_t y = target_ptr[i * 3 + 1];
          int32_t z = target_ptr[i * 3 + 2];

          if(i != 0 && (x == 0 && y == 0 && z == 0))
            {
              break;
            }

          target_set.insert({ x, y, z });
        }

      std::queue<std::pair<Cell, Cell>>    queue;
      std::unordered_set<Cell, Cell::Hash> visited;

      const auto                           start = *target_set.begin();

      queue.push({ start, { 0, 0, 0 } });
      visited.insert(start);
      target_set.erase(start);

      const std::vector<Cell> directions        = { { 1, 0, 0 }, { -1, 0, 0 }, { 0, 1, 0 }, { 0, -1, 0 }, { 0, 0, 1 }, { 0, 0, -1 } };
      const int32_t* const    matrix_ptr        = static_cast<int32_t*>(matrix_buf.ptr) + batch_index * matrix_buf.shape[1] * matrix_buf.shape[2] * matrix_buf.shape[3];
      const std::size_t       total_targets     = target_set.size();

      std::size_t             targets_connected = 0;
      double                  score             = 0.0;
      bool                    end               = false;

      while(!queue.empty() && !end)
        {
          const auto [current, parent] = queue.front();
          queue.pop();

          for(const auto& dir : directions)
            {
              const int32_t nx       = current.x + dir.x;
              const int32_t ny       = current.y + dir.y;
              const int32_t nz       = current.z + dir.z;
              const Cell    neighbor = { nx, ny, nz };

              if(!is_valid(nx, ny, nz, matrix_buf))
                {
                  continue;
                }

              ssize_t offset = nz * matrix_buf.shape[2] * matrix_buf.shape[3] + ny * matrix_buf.shape[3] + nx;

              if(matrix_ptr[offset] == 0)
                {
                  continue;
                }

              if(visited.find(neighbor) != visited.end())
                {
                  if(neighbor != parent)
                    {
                      end = true;
                      break;
                    }

                  continue;
                }

              visited.insert(neighbor);
              queue.push({ neighbor, current });

              if(target_set.find(neighbor) != target_set.end())
                {
                  ++targets_connected;

                  if(targets_connected == total_targets)
                    {
                      end   = true;
                      score = 1.0;
                      break;
                    }
                }
            }

          if(end)
            {
              break;
            }
        }

      result += score;
    }

  return result;
}

PYBIND11_MODULE(connectivity, m)
{
  m.doc() = "single_net_check_connectivity";
  m.def("single_net_check_connectivity", &single_net_check_connectivity, "", py::arg("matrix"), py::arg("target_cells"));
}