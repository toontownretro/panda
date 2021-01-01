/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionHandlerPhysical_ext.cxx
 * @author rdb
 * @date 2020-12-31
 */

#include "collisionHandlerPhysical_ext.h"
#include "collisionHandlerEvent_ext.h"

#ifdef HAVE_PYTHON

/**
 * Implements pickling behavior.
 */
PyObject *Extension<CollisionHandlerPhysical>::
__reduce__(PyObject *self) const {
  extern struct Dtool_PyTypedObject Dtool_Datagram;
  extern struct Dtool_PyTypedObject Dtool_NodePath;

  // Create a tuple with all the NodePath pointers.
  PyObject *nodepaths = PyTuple_New(_this->_colliders.size() * 2 + 1);
  Py_ssize_t i = 0;

  if (_this->has_center()) {
    const NodePath *center = &(_this->get_center());
    PyTuple_SET_ITEM(nodepaths, i++,
      DTool_CreatePyInstance((void *)center, Dtool_NodePath, false, true));
  } else {
    PyTuple_SET_ITEM(nodepaths, i++, Py_None);
    Py_INCREF(Py_None);
  }

  CollisionHandlerPhysical::Colliders::const_iterator it;
  for (it = _this->_colliders.begin(); it != _this->_colliders.end(); ++it) {
    const NodePath *collider = &(it->first);
    const NodePath *target = &(it->second._target);
    PyTuple_SET_ITEM(nodepaths, i++,
      DTool_CreatePyInstance((void *)collider, Dtool_NodePath, false, true));
    PyTuple_SET_ITEM(nodepaths, i++,
      DTool_CreatePyInstance((void *)target, Dtool_NodePath, false, true));
  }

  // Call the write_datagram method via Python, since it's not a virtual method
  // on the C++ end.
#if PY_MAJOR_VERSION >= 3
  PyObject *method_name = PyUnicode_FromString("write_datagram");
#else
  PyObject *method_name = PyString_FromString("write_datagram");
#endif

  Datagram dg;
  PyObject *destination = DTool_CreatePyInstance(&dg, Dtool_Datagram, false, false);

  PyObject *retval = PyObject_CallMethodOneArg(self, method_name, destination);
  Py_DECREF(method_name);
  Py_DECREF(destination);
  if (retval == nullptr) {
    return nullptr;
  }
  Py_DECREF(retval);

  const char *data = (const char *)dg.get_data();
  Py_ssize_t size = dg.get_length();
#if PY_MAJOR_VERSION >= 3
  return Py_BuildValue("O()(y#N)", Py_TYPE(self), data, size, nodepaths);
#else
  return Py_BuildValue("O()(s#N)", Py_TYPE(self), data, size, nodepaths);
#endif
}

/**
 * Takes the value returned by __getstate__ and uses it to freshly initialize
 * this CollisionHandlerPhysical object.
 */
void Extension<CollisionHandlerPhysical>::
__setstate__(PyObject *self, vector_uchar data, PyObject *nodepaths) {
  extern struct Dtool_PyTypedObject Dtool_DatagramIterator;

  // Call the read_datagram method via Python, since it's not a virtual method
  // on the C++ end.
#if PY_MAJOR_VERSION >= 3
  PyObject *method_name = PyUnicode_FromString("read_datagram");
#else
  PyObject *method_name = PyString_FromString("read_datagram");
#endif

  {
    Datagram dg(std::move(data));
    DatagramIterator scan(dg);
    PyObject *source = DTool_CreatePyInstance(&scan, Dtool_DatagramIterator, false, false);

    PyObject *retval = PyObject_CallMethodOneArg(self, method_name, source);
    Py_DECREF(method_name);
    Py_DECREF(source);
    Py_XDECREF(retval);
  }

  PyObject *center = PyTuple_GET_ITEM(nodepaths, 0);
  if (center != Py_None) {
    _this->set_center(*(NodePath *)DtoolInstance_VOID_PTR(center));
  } else {
    _this->clear_center();
  }

  size_t num_nodepaths = Py_SIZE(nodepaths);
  for (size_t i = 1; i < num_nodepaths;) {
    NodePath *collider = (NodePath *)DtoolInstance_VOID_PTR(PyTuple_GET_ITEM(nodepaths, i++));
    NodePath *target = (NodePath *)DtoolInstance_VOID_PTR(PyTuple_GET_ITEM(nodepaths, i++));
    _this->add_collider(*collider, *target);
  }
}

#endif
