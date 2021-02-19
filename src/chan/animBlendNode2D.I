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
  nassertv(point[0] >= -1 && point[0] <= 1 && point[1] >= -1 && point[1] <= 1);

  _inputs.push_back(node);
  _input_points.push_back(point);
  _input_weights.push_back(0.0f);

  _has_triangles = false;
}

/**
 * Returns the location of the AnimControl at the indicated index.
 */
LPoint2 AnimBlendNode2D::
get_input_point(int n) const {
  nassertr(n >= 0 && n < (int)_input_points.size(), LPoint2());

  return _input_points[n];
}

/**
 * Sets the input X coordinate.
 */
INLINE void AnimBlendNode2D::
set_input_x(PN_stdfloat x) {
  if (x != _input_coord[0]) {
    _input_coord_changed = true;
  }
  _input_coord[0] = x;
}

/**
 * Sets the input Y coordinate.
 */
INLINE void AnimBlendNode2D::
set_input_y(PN_stdfloat y) {
  if (y != _input_coord[1]) {
    _input_coord_changed = true;
  }
  _input_coord[1] = y;
}
