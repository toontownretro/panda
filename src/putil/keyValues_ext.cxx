#include "keyValues_ext.h"

PyObject *Extension<KeyValues>::
as_int_list(const std::string &str) {
  vector_int vec = KeyValues::parse_int_list(str);
  PyObject *list = PyList_New(vec.size());
  for (size_t i = 0; i < vec.size(); i++) {
    PyList_SetItem(list, i, PyLong_FromLong(vec[i]));
  }
  return list;
}

PyObject *Extension<KeyValues>::
as_float_list(const std::string &str) {
  vector_float vec = KeyValues::parse_float_list(str);
  PyObject *list = PyList_New(vec.size());
  for (size_t i = 0; i < vec.size(); i++) {
    PyList_SetItem(list, i, PyFloat_FromDouble(vec[i]));
  }
  return list;
}

PyObject *Extension<KeyValues>::
as_float_tuple_list(const std::string &str) {
  pvector<vector_float> vec = KeyValues::parse_float_tuple_list(str);
  PyObject *list = PyList_New(vec.size());
  for (size_t i = 0; i < vec.size(); i++) {
    const vector_float &floats = vec[i];
    PyObject *pyfloats = PyList_New(floats.size());
    for (size_t j = 0; j < floats.size(); j++) {
      PyList_SetItem(pyfloats, j, PyFloat_FromDouble(floats[j]));
    }
    PyList_SetItem(list, i, pyfloats);
  }
  return list;
}
