//
// wrapping lib/catalogue.cpp
//
#include "grid.h"
#include "error.h"
#include "py_grid.h"
#include "py_assert.h"
#include <iostream>
using namespace std;

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include "numpy/arrayobject.h"

#define NPY_FLOAT_TYPE NPY_DOUBLE
#define NPY_COMPLEX_TYPE NPY_COMPLEX128

//#define NPY_FLOAT_TYPE NPY_FLOAT
//#define NPY_COMPLEX_TYPE NPY_COMPLEX64

PyMODINIT_FUNC
py_grid_module_init()
{
  import_array();

  return NULL;
}


static void py_grid_free(PyObject *obj);

//
// class Grid
//

PyObject* py_grid_alloc(PyObject* self, PyObject* args)
{
  // _grid_alloc(nc)
  // Create a new grid object

  int nc;
  if(!PyArg_ParseTuple(args, "i", &nc)) {
    return NULL;
  }

  //msg_printf(msg_verbose, "Allocating a grid nc = %d\n", nc);

  Grid* grid= 0;
  try {
    grid= new Grid(nc);
  }
  catch(MemoryError) {
    PyErr_SetString(PyExc_MemoryError, "Grid memory error");
    return NULL;
  }

  return PyCapsule_New(grid, "_Grid", py_grid_free);
}

void py_grid_free(PyObject *obj)
{
  // Delete the grid object; called automatically by Python
  Grid* const grid= (Grid*) PyCapsule_GetPointer(obj, "_Grid");
  py_assert_void(grid);

  delete grid;
}

PyObject* py_grid_fft_forward(PyObject* self, PyObject* args)
{
  // Fourier transform the grid
  PyObject *py_grid;

  if(!PyArg_ParseTuple(args, "O", &py_grid)) {
    return NULL;
  }

  Grid* const grid=
    (Grid*) PyCapsule_GetPointer(py_grid, "_Grid");
  py_assert_ptr(grid);

  try {
    grid->fft_forward();
  }
  catch(AssertionError) {
    PyErr_SetString(PyExc_MemoryError, "Grid FFT error");
  }

  Py_RETURN_NONE;
}

PyObject* py_grid_nc(PyObject* self, PyObject* args)
{
  // _grid_nc(_cat)
  // Return the number of grid points per dimension
  PyObject *py_grid;

  if(!PyArg_ParseTuple(args, "O", &py_grid)) {
    return NULL;
  }

  Grid const * const grid=
    (Grid const *) PyCapsule_GetPointer(py_grid, "_Grid");
  py_assert_ptr(grid);

  return Py_BuildValue("n", grid->nc);
}

PyObject* py_grid_get_mode(PyObject* self, PyObject* args)
{
  // _grid_mode(_cat)
  // Return the mode of the grid real-space / fourier-space
  PyObject *py_grid;

  if(!PyArg_ParseTuple(args, "O", &py_grid)) {
    return NULL;
  }

  Grid const * const grid=
    (Grid const *) PyCapsule_GetPointer(py_grid, "_Grid");
  py_assert_ptr(grid);

  if(grid->mode == fft_mode_x)
    return Py_BuildValue("s", "real-space");
  else if(grid->mode == fft_mode_k)
    return Py_BuildValue("s", "fourier-space");

  Py_RETURN_NONE;
}

PyObject* py_grid_set_mode(PyObject* self, PyObject* args)
{
  // _grid_mode(_grid, mode)
  // set mode
  // mode: 0 unknown
  //       1 real-space
  //       2 fourier-space
  PyObject *py_grid;
  int mode;

  if(!PyArg_ParseTuple(args, "Oi", &py_grid, &mode)) {
    return NULL;
  }

  Grid* const grid=
    (Grid*) PyCapsule_GetPointer(py_grid, "_Grid");
  py_assert_ptr(grid);

  grid->mode= static_cast<FFTMode>(mode);

  Py_RETURN_NONE;
}


PyObject* py_grid_get_boxsize(PyObject* self, PyObject* args)
{
  // _grid_get_boxsize(_cat)
  // Return boxsize
  PyObject *py_grid;

  if(!PyArg_ParseTuple(args, "O", &py_grid)) {
    return NULL;
  }

  Grid const * const grid=
    (Grid const *) PyCapsule_GetPointer(py_grid, "_Grid");
  py_assert_ptr(grid);

  return Py_BuildValue("d", grid->boxsize);
}

PyObject* py_grid_set_boxsize(PyObject* self, PyObject* args)
{
  // _grid_set_boxsize(_cat, boxsize)
  // Set boxsize
  PyObject *py_grid;
  double boxsize;

  if(!PyArg_ParseTuple(args, "Od", &py_grid, &boxsize)) {
    return NULL;
  }

  Grid* const grid=
    (Grid*) PyCapsule_GetPointer(py_grid, "_Grid");
  py_assert_ptr(grid);

  grid->boxsize= boxsize;

  Py_RETURN_NONE;
}

PyObject* py_grid_get_x0(PyObject* self, PyObject* args)
{
  // _grid_get_x0(_cat, x0)
  // Return x0_box
  PyObject *py_grid;
  double x0[3];

  if(!PyArg_ParseTuple(args, "O", &py_grid, x0, x0+1, x0+2)) {
    return NULL;
  }

  Grid* const grid=
    (Grid*) PyCapsule_GetPointer(py_grid, "_Grid");
  py_assert_ptr(grid);

  return Py_BuildValue("(d,d,d)",
		       grid->x0_box[0], grid->x0_box[1], grid->x0_box[2]);  
}

PyObject* py_grid_set_x0(PyObject* self, PyObject* args)
{
  // _grid_set_x0(_cat, x0, y0, z0)
  // Set x0_box
  PyObject *py_grid;
  double x0[3];

  if(!PyArg_ParseTuple(args, "Oddd", &py_grid, x0, x0+1, x0+2)) {
    return NULL;
  }

  Grid* const grid=
    (Grid*) PyCapsule_GetPointer(py_grid, "_Grid");
  py_assert_ptr(grid);

  for(int k=0; k<3; ++k)
    grid->x0_box[k]= x0[k];

  Py_RETURN_NONE;
}

PyObject* py_grid_fx_asarray(PyObject* self, PyObject* args)
{
  PyObject *py_grid;

  if(!PyArg_ParseTuple(args, "O", &py_grid)) {
    return NULL;
  }

  Grid const * const grid=
    (Grid const *) PyCapsule_GetPointer(py_grid, "_Grid");
  py_assert_ptr(grid);

  const int nd= 3;
  npy_intp nc= grid->nc;
  npy_intp ncz= 2*(nc/2 + 1);
  
  npy_intp dims[]= {nc, nc, nc};
  npy_intp strides[]= {(npy_intp) (sizeof(double)*nc*ncz),
		       (npy_intp) (sizeof(double)*ncz),
		       (npy_intp) (sizeof(double))};
  return PyArray_New(&PyArray_Type, nd, dims, NPY_FLOAT_TYPE, strides,
  grid->fx, 0, 0, 0);
}


PyObject* py_grid_fk_asarray(PyObject* self, PyObject* args)
{
  PyObject *py_grid;

  if(!PyArg_ParseTuple(args, "O", &py_grid)) {
    return NULL;
  }

  Grid const * const grid=
    (Grid const *) PyCapsule_GetPointer(py_grid, "_Grid");
  py_assert_ptr(grid);

  const int nd= 3;
  const npy_intp nc= grid->nc;
  const int nckz= nc/2 + 1;
  npy_intp dims[]= {nc, nc, nckz};

  return PyArray_SimpleNewFromData(nd, dims, NPY_COMPLEX_TYPE, grid->fk);
}



PyObject* py_grid_clear(PyObject* self, PyObject* args)
{
  PyObject *py_grid;

  if(!PyArg_ParseTuple(args, "O", &py_grid)) {
    return NULL;
  }

  Grid* const grid=
    (Grid*) PyCapsule_GetPointer(py_grid, "_Grid");
  py_assert_ptr(grid);

  grid->clear();

  Py_RETURN_NONE;
}

PyObject* py_grid_load_fx_from_array(PyObject* self, PyObject* args)
{
  PyObject *py_grid, *py_array;

  if(!PyArg_ParseTuple(args, "OO", &py_grid, &py_array)) {
    return NULL;
  }

  Grid const * const grid=
    (Grid const *) PyCapsule_GetPointer(py_grid, "_Grid");
  py_assert_ptr(grid);

  Py_buffer a;
  if(PyObject_GetBuffer(py_array, &a, PyBUF_FORMAT | PyBUF_ANY_CONTIGUOUS | PyBUF_FULL_RO) == -1)
    return NULL;

  if(a.ndim != 3) {
    PyErr_SetString(PyExc_TypeError, "Expected a 3-dimensional array for grid");
    return NULL;
  }

  size_t nc= grid->nc;
  Py_ssize_t nc_check= static_cast<Py_ssize_t>(nc);
  if(a.shape[0] != nc_check || a.shape[1] != nc_check ||
     a.shape[2] != nc_check) {
    PyErr_SetString(PyExc_TypeError, "Expected a cubic grid.");
    return NULL;
  }

  if(strcmp(a.format, "d") != 0) {
    PyErr_SetString(PyExc_TypeError, "Expected an array of double.");
    return NULL;
  }
  
  npy_intp ncz= 2*(nc/2 + 1);

  double* p= (double*) a.buf;

  for(size_t ix=0; ix<nc; ++ix) {
    for(size_t iy=0; iy<nc; ++iy) {
      size_t index= (ix*nc + iy)*ncz;
      for(size_t iz=0; iz<nc; ++iz) {
	grid->fx[index++]= *p++;
      }
    }
  }
  Py_RETURN_NONE;
}

	
