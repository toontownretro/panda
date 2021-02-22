/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animAddNode.I
 * @author lachbr
 * @date 2021-02-18
 */

/**
 *
 */
INLINE void AnimAddNode::
set_base(AnimGraphNode *base) {
  _base = base;
}

/**
 *
 */
INLINE AnimGraphNode *AnimAddNode::
get_base() const {
  return _base;
}

/**
 *
 */
INLINE void AnimAddNode::
set_add(AnimGraphNode *add) {
  _add = add;
}

/**
 *
 */
INLINE AnimGraphNode *AnimAddNode::
get_add() const {
  return _add;
}

/**
 * Sets the weight of the addition of the additive pose onto the base pose.
 * A smaller value results in a smaller contribution of the additive pose, and
 * vice versa.
 */
INLINE void AnimAddNode::
set_alpha(PN_stdfloat alpha) {
  _alpha = alpha;
}

/**
 * Returns the weight of the addition of the additive pose onto the base pose.
 */
INLINE PN_stdfloat AnimAddNode::
get_alpha() const {
  return _alpha;
}
