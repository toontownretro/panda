/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animMixNode.I
 * @author lachbr
 * @date 2021-02-18
 */

/**
 * Sets the mixing value.  0.0 is fully input 1, 1.0 is fully input 2.
 */
INLINE void AnimMixNode::
set_alpha(PN_stdfloat alpha) {
  _alpha = alpha;
}

/**
 * Returns the mixing value.  0.0 is fully input 1, 1.0 is fully input 2.
 */
INLINE PN_stdfloat AnimMixNode::
get_alpha() const {
  return _alpha;
}
