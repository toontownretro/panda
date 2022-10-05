/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file refCallbackData.h
 * @author brian
 * @date 2021-04-26
 */

#ifndef REFCALLBACKDATA_H
#define REFCALLBACKDATA_H

#include "pandabase.h"
#include "callbackData.h"
#include "referenceCount.h"
#include "memoryUsage.h"

/**
 * Reference-counted callback data.
 */
class EXPCL_PANDA_PPHYSICS RefCallbackData : public CallbackData, public ReferenceCount {
PUBLISHED:
  INLINE RefCallbackData();
  virtual ~RefCallbackData() = default;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CallbackData::init_type();
    ReferenceCount::init_type();
    register_type(_type_handle, "RefCallbackData",
                  CallbackData::get_class_type(),
                  ReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "refCallbackData.I"

#endif // REFCALLBACKDATA_H
