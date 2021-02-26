/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animSampleNode.I
 * @author lachbr
 * @date 2021-02-18
 */

/**
 * Sets the AnimControl that will be used to sample the animation channel when
 * the node is evaluated.
 */
INLINE void AnimSampleNode::
set_control(AnimControl *control) {
  _control = control;
}

/**
 * Returns the AnimControl that will be used to sample the animation channel
 * when the node is evaluated.
 */
INLINE AnimControl *AnimSampleNode::
get_control() const {
  return _control;
}
