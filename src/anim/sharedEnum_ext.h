/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sharedEnum_ext.h
 * @author brian
 * @date 2021-06-14
 */

#ifndef SHAREDENUM_EXT_H
#define SHAREDENUM_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "py_panda.h"
#include "sharedEnum.h"

/**
 * Python extension of the SharedEnum class.  Implements a short-hand accessor
 * to retrieve enum value IDs by name.
 */
template<>
class Extension<SharedEnum> : public ExtensionBase<SharedEnum> {
public:
  INLINE PyObject *__getattr__(PyObject *self, const std::string &attr_name) const;
};

#include "sharedEnum_ext.I"

#endif // HAVE_PYTHON

#endif // SHAREDENUM_EXT_H
