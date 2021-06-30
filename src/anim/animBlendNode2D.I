/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animBlendNode2D.I
 * @author lachbr
 * @date 2021-02-18
 */

/**
 * Adds a new input to the node at the indicated location.  The location
 * must be in the range of -1 to 1.
 */
void AnimBlendNode2D::
add_input(AnimGraphNode *node, const LPoint2 &point) {
  nassertv(point[0] >= 0 && point[0] <= 1 && point[1] >= 0 && point[1] <= 1);

  Input input;
  input._node = node;
  input._point = point;
  input._weight = 0.0f;
  _inputs.push_back(std::move(input));

  add_child(node);

  _has_triangles = false;
}

/**
 * Returns the AnimGraphNode for the input at the indicated index.
 */
INLINE AnimGraphNode *AnimBlendNode2D::
get_input_node(int n) const {
  nassertr(n >= 0 && n < (int)_inputs.size(), nullptr);

  return _inputs[n]._node;
}

/**
 * Returns the location of the AnimControl at the indicated index.
 */
LPoint2 AnimBlendNode2D::
get_input_point(int n) const {
  nassertr(n >= 0 && n < (int)_inputs.size(), LPoint2());

  return _inputs[n]._point;
}

/**
 * Sets the index of the pose parameter used for the X input coordinate.
 */
INLINE void AnimBlendNode2D::
set_input_x(int param) {
  _x_param = param;
}

/**
 * Sets the index of the pose parameter used for the Y input coordinate.
 */
INLINE void AnimBlendNode2D::
set_input_y(int param) {
  _y_param = param;
}
