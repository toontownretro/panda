/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pmdlAttachment.h
 * @author lachbr
 * @date 2021-02-15
 */

#ifndef PMDLATTACHMENT_H
#define PMDLATTACHMENT_H

#include "pmdlObject.h"
#include "luse.h"

class EXPCL_PANDA_EGG PMDLAttachment : public PMDLObject {
PUBLISHED:
  INLINE PMDLAttachment(const std::string &name);

  INLINE void set_parent_joint(const std::string &name);
  INLINE std::string get_parent_joint() const;

  INLINE void set_pos(const LPoint3 &pos);
  INLINE LPoint3 get_pos() const;

  INLINE void set_hpr(const LVector3 &hpr);
  INLINE LVector3 get_hpr() const;

private:
  std::string _parent_joint;
  LPoint3 _pos;
  LVector3 _hpr;
};

#include "pmdlAttachment.I"

#endif // PMDLATTACHMENT_H
