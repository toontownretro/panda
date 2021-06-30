/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pythonPhysQueryFilter.cxx
 * @author brian
 * @date 2021-05-26
 */

#include "pythonPhysQueryFilter.h"
#include "physRigidActorNode.h"
#include "physShape.h"
#include "config_pphysics.h"

#ifndef CPPPARSER
extern struct Dtool_PyTypedObject Dtool_PhysRigidActorNode;
#endif

/**
 *
 */
PythonPhysQueryFilter::
PythonPhysQueryFilter(PyObject *method) {
  _method = Py_None;
  Py_INCREF(_method);

  set_method(method);

#ifndef SIMPLE_THREADS
  // Ensure that the Python threading system is initialized and ready to go.
#ifdef WITH_THREAD  // This symbol defined within Python.h

  Py_Initialize();

  PyEval_InitThreads();
#endif
#endif
}

/**
 *
 */
PythonPhysQueryFilter::
~PythonPhysQueryFilter() {
  Py_DECREF(_method);
}

/**
 * Sets the Python method that should be called to perform filtering.
 */
void PythonPhysQueryFilter::
set_method(PyObject *method) {
  Py_DECREF(_method);
  _method = method;
  Py_INCREF(_method);
  if (_method != Py_None && !PyCallable_Check(_method)) {
    nassert_raise("Invalid method passed to PythonPhysQueryFilter");
  }
}

/**
 * Returns the Python method that is called to perform filtering.
 */
PyObject *PythonPhysQueryFilter::
get_method() {
  Py_INCREF(_method);
  return _method;
}


/**
 *
 */
physx::PxQueryHitType::Enum PythonPhysQueryFilter::
preFilter(const physx::PxFilterData &filter_data, const physx::PxShape *shape,
          const physx::PxRigidActor *actor, physx::PxHitFlags &query_flags) {

  // Let the base filter determine the hit type.
  physx::PxQueryHitType::Enum hit_type = PhysBaseQueryFilter::
    preFilter(filter_data, shape, actor, query_flags);

  if (hit_type == physx::PxQueryHitType::eNONE) {
    // Base class filtered it out, no need to call the Python filter.
    return hit_type;
  }

  if (actor->userData == nullptr) {
    // This doesn't correspond to a Panda-created PhysX actor.
    return hit_type;
  }

  PhysRigidActorNode *actor_node = (PhysRigidActorNode *)actor->userData;

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  // Use PyGILState to protect this asynchronous call.
  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();
#endif

  // Build up a Python wrapper around the actor.
  PyObject *pyactor = DTool_CreatePyInstanceTyped(actor_node,
                                                  Dtool_PhysRigidActorNode,
                                                  false, false, actor_node->get_type_index());
  PyObject *args = Py_BuildValue("(O,I,I,I)", pyactor,
                                 (unsigned int)filter_data.word1,
                                 (unsigned int)filter_data.word3,
                                 (unsigned int)hit_type);
  Py_DECREF(pyactor);

  // Call the Python filter.
  PyObject *result = PyObject_CallObject(_method, args);
  Py_DECREF(args);
  if (result == nullptr) {
    if (PyErr_Occurred() != PyExc_SystemExit) {
      pphysics_cat.error()
        << "Exception occurred in PythonPhysQueryFilter:\n";
      PyErr_Print();
    }

  } else {
    // Take the hit type from return value of the Python filter.
    hit_type = (physx::PxQueryHitType::Enum)PyLong_AsUnsignedLong(result);
    Py_DECREF(result);
  }

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  PyGILState_Release(gstate);
#endif

  return hit_type;
}
