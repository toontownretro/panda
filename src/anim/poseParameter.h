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

class BamWriter;
class Datagram;
class BamReader;
class DatagramIterator;

/**
 * A parameter used as input for animation blend nodes.  Can be shared between
 * multiple nodes.
 */
class EXPCL_PANDA_ANIM PoseParameter : public Namable {
PUBLISHED:
  INLINE PoseParameter(const std::string &name, PN_stdfloat min, PN_stdfloat max, PN_stdfloat looping = 0.0f);
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

  INLINE void set_looping(PN_stdfloat loop);
  INLINE PN_stdfloat get_looping() const;

public:
  PoseParameter() = default;

  void write_datagram(BamWriter *manager, Datagram &dg);
  void fillin(DatagramIterator &scan, BamReader *manager);

private:
  PN_stdfloat _min = 0.0f;
  PN_stdfloat _max = 0.0f;
  PN_stdfloat _value = 0.0f;
  PN_stdfloat _looping = 0.0f;

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
