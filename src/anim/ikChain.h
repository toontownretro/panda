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

#ifndef IKCHAIN_H
#define IKCHAIN_H

#include "pandabase.h"
#include "namable.h"
#include "luse.h"

class BamWriter;
class BamReader;

/**
 * This class represents a chain of joints that should receive inverse kinematics.
 *
 * Currently, this only supports a hip-knee-foot set up as it is easy to
 * compute and the most common use of IK in a video game.
 */
class EXPCL_PANDA_ANIM IKChain : public Namable {
PUBLISHED:
  IKChain(const std::string &name, int top_joint, int middle_joint, int end_joint);
  IKChain(const IKChain &copy);
  IKChain(IKChain &&other);
  void operator = (const IKChain &copy);
  void operator = (IKChain &&other);

  INLINE void set_middle_joint_direction(const LVector3 &dir);
  INLINE const LVector3 &get_middle_joint_direction() const;

  INLINE void set_center(const LPoint3 &center);
  INLINE const LPoint3 &get_center() const;

  INLINE void set_height(PN_stdfloat height);
  INLINE PN_stdfloat get_height() const;

  INLINE void set_floor(PN_stdfloat floor);
  INLINE PN_stdfloat get_floor() const;

  INLINE void set_pad(PN_stdfloat pad);
  INLINE PN_stdfloat get_pad() const;

  INLINE int get_end_joint() const;
  INLINE int get_middle_joint() const;
  INLINE int get_top_joint() const;

public:
  INLINE IKChain();

private:
  // Joint indices.
  int _end_joint;
  int _middle_joint;
  int _top_joint;

  LVector3 _middle_direction;
  LPoint3 _center;
  PN_stdfloat _height;
  PN_stdfloat _floor;
  PN_stdfloat _pad;

public:
  void write_datagram(BamWriter *manager, Datagram &dg);
  void fillin(DatagramIterator &scan, BamReader *manager);
};

#include "ikChain.I"

#endif // IKCHAIN_H
