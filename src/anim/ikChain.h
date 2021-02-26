/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ikChain.h
 * @author lachbr
 * @date 2021-02-11
 */

#if 0
#ifndef IKCHAIN_H
#define IKCHAIN_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "namable.h"
#include "nodePath.h"

class MovingPartMatrix;

/**
 * This class represents a chain of joints that should receive inverse kinematics.
 *
 * Currently, this only supports a hip-knee-foot set up as it is easy to
 * compute and the most common use of IK in a video game.
 */
class EXPCL_PANDA_ANIM IKChain : public TypedWritableReferenceCount, public Namable {
PUBLISHED:
  IKChain(const std::string &name, MovingPartMatrix *foot);

  bool solve_ik();

  INLINE void set_enabled(bool enable);
  INLINE bool get_enabled() const;

  INLINE void set_knee_direction(const LVector3 &dir);
  INLINE const LVector3 &get_knee_direction() const;

  INLINE void set_center(const LPoint3 &center);
  INLINE const LPoint3 &get_center() const;

  INLINE void set_height(PN_stdfloat height);
  INLINE PN_stdfloat get_height() const;

  INLINE void set_floor(PN_stdfloat floor);
  INLINE PN_stdfloat get_floor() const;

  INLINE void set_pad(PN_stdfloat pad);
  INLINE PN_stdfloat get_pad() const;

  INLINE MovingPartMatrix *get_foot() const;
  INLINE MovingPartMatrix *get_knee() const;
  INLINE MovingPartMatrix *get_hip() const;

private:
  INLINE IKChain();

private:
  MovingPartMatrix *_foot;
  MovingPartMatrix *_knee;
  MovingPartMatrix *_hip;

  // NodePath that feeds the IK solver the desired foot placement location.
  NodePath _foot_locator;

  LVector3 _knee_direction;
  LPoint3 _center;
  PN_stdfloat _height;
  PN_stdfloat _floor;
  PN_stdfloat _pad;

  bool _enabled;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **p_list,
                                BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:

  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "IKChain",
                  TypedWritableReferenceCount::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "ikChain.I"

#endif // IKCHAIN_H

#endif
