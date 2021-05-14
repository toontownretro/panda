/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animOverlayNode.cxx
 * @author brian
 * @date 2021-05-08
 */

#include "animOverlayNode.h"

TypeHandle AnimOverlayNode::_type_handle;

/**
 *
 */
void AnimOverlayNode::
evaluate(AnimGraphEvalContext &context) {

  // Initialize each joint to the bind pose.
  //for (int i = 0; i < context._num_joints; i++) {
  //  context._joints[i]._position = context._parts[i]._default_pos;
  //  context._joints[i]._rotation = context._parts[i]._default_quat;
  //  context._joints[i]._scale = context._parts[i]._default_scale;
  //}

  if (_a != nullptr) {
    // Evaluate the base layer.
    _a->evaluate(context);
  }

  if (_b != nullptr) {
    // Now evaluate the overlay layer.
    _b->evaluate(context);
  }
}
