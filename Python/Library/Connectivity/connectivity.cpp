#include <queue>
#include <set>
#include <vector>

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

struct Cell
{
  int32_t x;
  int32_t y;
  int32_t z;
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

  double        result     = 0.0;

  for(ssize_t batch_index = 0, batch_size = matrix_buf.shape[0]; batch_index < batch_size; ++batch_index)
    {
      std::set<std::tuple<int32_t, int32_t, int32_t>> target_set;
      const int32_t* const                            target_ptr = static_cast<int32_t*>(target_buf.ptr) + batch_index * target_buf.shape[1] * 3;

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

      std::queue<Cell>                                queue;
      std::set<std::tuple<int32_t, int32_t, int32_t>> visited;

      const auto                                      start = *target_set.begin();

      queue.push({ std::get<0>(start), std::get<1>(start), std::get<2>(start) });
      visited.insert(start);
      target_set.erase(start);

      const std::vector<Cell> directions = { { 1, 0, 0 }, { -1, 0, 0 }, { 0, 1, 0 }, { 0, -1, 0 }, { 0, 0, 1 }, { 0, 0, -1 } };
      const int32_t* const    matrix_ptr = static_cast<int32_t*>(matrix_buf.ptr) + batch_index * matrix_buf.shape[1] * matrix_buf.shape[2] * matrix_buf.shape[3];

      while(!queue.empty())
        {
          const Cell current = std::move(queue.front());
          queue.pop();

          for(const auto& dir : directions)
            {
              const int32_t                               nx       = current.x + dir.x;
              const int32_t                               ny       = current.y + dir.y;
              const int32_t                               nz       = current.z + dir.z;
              const std::tuple<int32_t, int32_t, int32_t> neighbor = { nx, ny, nz };

              if(is_valid(nx, ny, nz, matrix_buf) && visited.find(neighbor) == visited.end())
                {
                  ssize_t offset = nz * matrix_buf.shape[2] * matrix_buf.shape[3] + ny * matrix_buf.shape[3] + nx;

                  if(matrix_ptr[offset] != 0)
                    {
                      visited.insert(neighbor);
                      queue.push({ nx, ny, nz });

                      target_set.erase(neighbor);
                    }
                }
            }
        }

      result += target_set.empty() ? 1.0 : 0.0;
    }

  return result;
}

PYBIND11_MODULE(connectivity, m)
{
  m.doc() = "single_net_check_connectivity";
  m.def("single_net_check_connectivity", &single_net_check_connectivity, "", py::arg("matrix"), py::arg("target_cells"));
}