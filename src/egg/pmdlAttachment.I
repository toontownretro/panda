/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pmdlAttachment.I
 * @author lachbr
 * @date 2021-02-15
 */

/**
 *
 */
INLINE PMDLAttachment::
PMDLAttachment(const std::string &name) :
  PMDLObject(name) {
}

/**
 * Sets the name of the parent joint for the attachment.
 */
INLINE void PMDLAttachment::
set_parent_joint(const std::string &name) {
  _parent_joint = name;
}

/**
 * Returns the name of the parent joint for the attachment.
 */
INLINE std::string PMDLAttachment::
get_parent_joint() const {
  return _parent_joint;
}

/**
 * Sets the positional offset relative to the parent joint.
 */
INLINE void PMDLAttachment::
set_pos(const LPoint3 &pos) {
  _pos = pos;
}

/**
 * Returns the positional offset relative to the parent joint.
 */
INLINE LPoint3 PMDLAttachment::
get_pos() const {
  return _pos;
}

/**
 * Sets the angles offset relative to the parent joint.
 */
INLINE void PMDLAttachment::
set_hpr(const LVector3 &hpr) {
  _hpr = hpr;
}

/**
 * Returns the angles offset relative to the parent joint.
 */
INLINE LVector3 PMDLAttachment::
get_hpr() const {
  return _hpr;
}
