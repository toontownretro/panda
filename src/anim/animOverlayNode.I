/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animOverlayNode.I
 * @author brian
 * @date 2021-05-08
 */

/**
 *
 */
INLINE AnimOverlayNode::
AnimOverlayNode(const std::string &name, AnimGraphNode *a, AnimGraphNode *b) :
  AnimGraphNode(name)
{
  set_a(a);
  set_b(b);
}

/**
 *
 */
INLINE void AnimOverlayNode::
set_a(AnimGraphNode *a) {
  _a = a;
}

/**
 *
 */
INLINE void AnimOverlayNode::
set_b(AnimGraphNode *b) {
  _b = b;
}

/**
 *
 */
INLINE AnimGraphNode *AnimOverlayNode::
get_a() const {
  return _a;
}

/**
 *
 */
INLINE AnimGraphNode *AnimOverlayNode::
get_b() const {
  return _b;
}
