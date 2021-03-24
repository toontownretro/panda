/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pmdlObject.h
 * @author lachbr
 * @date 2021-02-13
 */

#ifndef PMDLOBJECT_H
#define PMDLOBJECT_H

#include "pandabase.h"
#include "namable.h"
#include "referenceCount.h"

/**
 * Base class for all .pmdl object types.
 */
class EXPCL_PANDA_EGG PMDLObject : public ReferenceCount, public Namable {
PUBLISHED:
  INLINE PMDLObject(const std::string &name);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ReferenceCount::init_type();
    Namable::init_type();
    register_type(_type_handle, "PMDLObject");
  }

private:
  static TypeHandle _type_handle;
};

#include "pmdlObject.I"

#endif // PMDLOBJECT_H
