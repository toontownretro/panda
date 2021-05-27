/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pythonPhysQueryFilter.h
 * @author brian
 * @date 2021-05-26
 */

#ifndef PYTHONPHYSQUERYFILTER_H
#define PYTHONPHYSQUERYFILTER_H

#include "pandabase.h"

#ifdef HAVE_PYTHON

#include "py_panda.h"
#include "physQueryFilter.h"

/**
 * A query filter that allows filtering to be performed by an arbitrary Python
 * method.
 */
class PythonPhysQueryFilter : public PhysBaseQueryFilter {
PUBLISHED:
  PythonPhysQueryFilter(PyObject *method = Py_None);
  ~PythonPhysQueryFilter();

  void set_method(PyObject *method);
  PyObject *get_method();

public:
  virtual physx::PxQueryHitType::Enum preFilter(
    const physx::PxFilterData &filter_data, const physx::PxShape *shape,
    const physx::PxRigidActor *actor, physx::PxHitFlags &query_flags) override;

private:
  PyObject *_method;
};

#include "pythonPhysQueryFilter.I"

#endif // HAVE_PYTHON

#endif // PYTHONPHYSQUERYFILTER_H
