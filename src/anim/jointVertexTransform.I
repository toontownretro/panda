/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file jointVertexTransform.I
 * @author drose
 * @date 2005-03-24
 */

/**
 *
 */
INLINE const Character *JointVertexTransform::
get_character() const {
  if (_char.was_deleted()) {
    return nullptr;
  } else {
    return _char.p();
  }
}

/**
 * Returns the joint for which this object returns the transform.
 */
INLINE int JointVertexTransform::
get_joint() const {
  return _joint;
}
