#ifndef PY_GRID_H
#define PY_GRID_H 1

#include "Python.h"

PyMODINIT_FUNC
py_grid_module_init();

PyObject* py_grid_alloc(PyObject* self, PyObject* args);
PyObject* py_grid_fft_forward(PyObject* self, PyObject* args);
PyObject* py_grid_fft_inverse(PyObject* self, PyObject* args);
PyObject* py_grid_nc(PyObject* self, PyObject* args);
PyObject* py_grid_get_mode(PyObject* self, PyObject* args);
PyObject* py_grid_set_mode(PyObject* self, PyObject* args);
PyObject* py_grid_get_boxsize(PyObject* self, PyObject* args);
PyObject* py_grid_set_boxsize(PyObject* self, PyObject* args);
PyObject* py_grid_get_x0(PyObject* self, PyObject* args);
PyObject* py_grid_set_x0(PyObject* self, PyObject* args);

PyObject* py_grid_fx_asarray(PyObject* self, PyObject* args);
PyObject* py_grid_fk_asarray(PyObject* self, PyObject* args);

PyObject* py_grid_load_fx_from_array(PyObject* self, PyObject* args);

PyObject* py_grid_clear(PyObject* self, PyObject* args);


#endif
