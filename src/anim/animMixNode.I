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
 * Sets the first input.
 */
INLINE void AnimMixNode::
set_a(AnimGraphNode *a) {
  if (_a != nullptr) {
    remove_child(_a);
  }

  _a = a;
  add_child(_a);
}

/**
 * Returns the first input.
 */
INLINE AnimGraphNode *AnimMixNode::
get_a() const {
  return _a;
}

/**
 * Sets the second input.
 */
INLINE void AnimMixNode::
set_b(AnimGraphNode *b) {
  if (_b != nullptr) {
    remove_child(_b);
  }

  _b = b;
  add_child(_b);
}

/**
 * Returns the second input.
 */
INLINE AnimGraphNode *AnimMixNode::
get_b() const {
  return _b;
}

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
