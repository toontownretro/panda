/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file poseParameter.h
 * @author brian
 * @date 2021-05-07
 */

#ifndef POSEPARAMETER_H
#define POSEPARAMETER_H

#include "pandabase.h"
#include "referenceCount.h"
#include "namable.h"

/**
 * A parameter used as input for animation blend nodes.  Can be shared between
 * multiple nodes.
 */
class EXPCL_PANDA_ANIM PoseParameter : public Namable {
PUBLISHED:
  INLINE PoseParameter(const std::string &name, PN_stdfloat min, PN_stdfloat max, bool looping = false);
  INLINE PoseParameter(PoseParameter &&other);
  INLINE PoseParameter(const PoseParameter &copy);
  INLINE void operator = (const PoseParameter &copy);
  INLINE void operator = (PoseParameter &&other);

  INLINE void set_min(PN_stdfloat min);
  INLINE PN_stdfloat get_min() const;

  INLINE void set_max(PN_stdfloat max);
  INLINE PN_stdfloat get_max() const;

  INLINE void set_value(PN_stdfloat value);
  INLINE PN_stdfloat get_value() const;

  INLINE void set_looping(bool looping);
  INLINE bool get_looping() const;

private:
  PN_stdfloat _min;
  PN_stdfloat _max;
  PN_stdfloat _value;
  bool _looping;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    Namable::init_type();
    register_type(_type_handle, "PoseParameter",
                  Namable::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "poseParameter.I"

#endif // POSEPARAMETER_H
