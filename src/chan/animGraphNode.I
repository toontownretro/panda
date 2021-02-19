/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animGraphNode.I
 * @author lachbr
 * @date 2021-02-18
 */

/**
 * Adds an input node to this node.
 */
INLINE void AnimGraphNode::
add_input(AnimGraphNode *node) {
  nassertv((int)_inputs.size() < get_max_inputs());
  _inputs.push_back(node);
}

/**
 * Returns the number of inputs to this node.
 */
INLINE int AnimGraphNode::
get_num_inputs() const {
  return (int)_inputs.size();
}

/**
 * Returns the nth input to this node.
 */
INLINE AnimGraphNode *AnimGraphNode::
get_input(int n) const {
  return _inputs[n];
}

/**
 * Returns the output pose position of this node.
 */
INLINE const LPoint3 &AnimGraphNode::
get_position() const {
  return _position;
}

/**
 * Returns the output pose rotation of this node.
 */
INLINE const LQuaternion &AnimGraphNode::
get_rotation() const {
  return _rotation;
}

/**
 * Returns the output pose scale of this node.
 */
INLINE const LVector3 &AnimGraphNode::
get_scale() const {
  return _scale;
}

/**
 * Returns the output pose shear of this node.
 */
INLINE const LVector3 &AnimGraphNode::
get_shear() const {
  return _shear;
}
