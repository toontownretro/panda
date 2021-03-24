/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pmdlIKChain.h
 * @author lachbr
 * @date 2021-02-13
 */

#ifndef PMDLIKCHAIN_H
#define PMDLIKCHAIN_H

#include "pmdlObject.h"
#include "luse.h"

/**
 * Defines a two-link hip/knee/foot inverse kinematics chain.
 *
 * Could also be shoulder/elbow/hand.
 */
class EXPCL_PANDA_EGG PMDLIKChain : public PMDLObject {
PUBLISHED:
  INLINE PMDLIKChain(const std::string &name);

  INLINE void set_foot_joint(const std::string &name);
  INLINE std::string get_foot_joint() const;

  INLINE void set_knee_direction(const LVector3 &dir);
  INLINE LVector3 get_knee_direction() const;

  INLINE void set_center(const LPoint3 &center);
  INLINE LPoint3 get_center() const;

  INLINE void set_height(PN_stdfloat height);
  INLINE PN_stdfloat get_height() const;

  INLINE void set_floor(PN_stdfloat floor);
  INLINE PN_stdfloat get_floor() const;

  INLINE void set_pad(PN_stdfloat pad);
  INLINE PN_stdfloat get_pad() const;

private:
  std::string _foot_joint;
  LVector3 _knee_direction;
  LVector3 _center;
  PN_stdfloat _height;
  PN_stdfloat _floor;
  PN_stdfloat _pad;
};

#include "pmdlIKChain.I"

#endif // PMDLIKCHAIN_H
