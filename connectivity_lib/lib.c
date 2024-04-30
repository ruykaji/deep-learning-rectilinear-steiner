#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <numpy/arrayobject.h>
#include <stdbool.h>

// Helper function to perform DFS
void dfs(int y, int x, int prev_y, int prev_x, int ymax, int xmax, bool **visited, int batch, PyArrayObject *arr, int num_terminals, int *terminals, bool *terminals_visited, bool *has_cycle)
{
    if (x < 0 || x >= xmax || y < 0 || y >= ymax || visited[y][x] || *((int *)PyArray_GETPTR3(arr, batch, y, x)) == 0)
        return;

    visited[y][x] = true;
    for (int i = 0; i < num_terminals; i++)
    {
        if (terminals[2 * i] == y && terminals[2 * i + 1] == x)
        {
            terminals_visited[i] = true;
            break;
        }
    }

    int dx[] = {1, -1, 0, 0};
    int dy[] = {0, 0, 1, -1};

    for (int dir = 0; dir < 4; dir++)
    {
        int ny = y + dy[dir];
        int nx = x + dx[dir];

        if (ny >= 0 && ny < ymax && nx >= 0 && nx < xmax && !(*((int *)PyArray_GETPTR3(arr, batch, ny, nx)) == 0))
        {
            if (!visited[ny][nx])
            {
                dfs(ny, nx, y, x, ymax, xmax, visited, batch, arr, num_terminals, terminals, terminals_visited, has_cycle);
            }
            else if (ny != prev_y || nx != prev_x)
            {
                *has_cycle = true;
            }
        }
    }
}

static PyObject *are_batches_terminals_connected(PyObject *self, PyObject *args)
{
    PyObject *input_array = NULL;
    PyObject *terminals_list = NULL;
    if (!PyArg_ParseTuple(args, "O!O!", &PyArray_Type, &input_array, &PyList_Type, &terminals_list))
    {
        return NULL;
    }

    PyArrayObject *arr = (PyArrayObject *)input_array;
    if (!PyArray_Check(arr))
    {
        PyErr_SetString(PyExc_TypeError, "Input should be a numpy ndarray of integers.");
        return NULL;
    }

    const long int num_batches = PyArray_DIM(arr, 0);
    PyObject *result_array = PyArray_ZEROS(1, &num_batches, NPY_INT, 0);

    for (int b = 0; b < num_batches; b++)
    {
        PyObject *batch_terminals_list = PyList_GetItem(terminals_list, b);

        int rows = PyArray_DIM(arr, 1);
        int cols = PyArray_DIM(arr, 2);
        int num_terminals = PyLong_AsLong(PyList_GetItem(batch_terminals_list, 0)) / 2;
        int *terminals = (int *)malloc(num_terminals * 2 * sizeof(int));
        bool *terminals_visited = (bool *)calloc(num_terminals, sizeof(bool));
        bool **visited = (bool **)malloc(rows * sizeof(bool *));
        bool has_cycle = false;

        for (int i = 0; i < rows; i++)
        {
            visited[i] = (bool *)calloc(cols, sizeof(bool));
        }

        // Fill terminals array from Python list
        for (int i = 0; i < num_terminals * 2; i++)
        {
            terminals[i] = PyLong_AsLong(PyList_GetItem(batch_terminals_list, i + 1));
        }

        // Start DFS from the first terminal if any
        if (num_terminals > 0)
        {
            dfs(terminals[0], terminals[1], -1, -1, rows, cols, visited, b, arr, num_terminals, terminals, terminals_visited, &has_cycle);
        }

        // Check if all terminals are visited
        bool all_connected = true;
        for (int i = 0; i < num_terminals; i++)
        {
            if (!terminals_visited[i])
            {
                all_connected = false;
                break;
            }
        }

        if (all_connected && !has_cycle)
            *((bool *)PyArray_GETPTR1(result_array, b)) = 1;
        else
            *((bool *)PyArray_GETPTR1(result_array, b)) = 0;

        // // Clean up
        for (int i = 0; i < rows; i++)
        {
            free(visited[i]);
        }
        free(visited);
        free(terminals);
        free(terminals_visited);
    }

    return result_array;
}

static PyMethodDef ConnectivityMethods[] = {
    {"are_batches_terminals_connected", are_batches_terminals_connected, METH_VARARGS, "Check if specified terminals in a batch of 2D numpy arrays are connected."},
    {NULL, NULL, 0, NULL}};

static struct PyModuleDef connectivitymodule = {
    PyModuleDef_HEAD_INIT,
    "connectivity_module",
    NULL,
    -1,
    ConnectivityMethods};

PyMODINIT_FUNC PyInit_connectivity_module(void)
{
    import_array();
    return PyModule_Create(&connectivitymodule);
}
