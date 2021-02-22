/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animAddNode.cxx
 * @author lachbr
 * @date 2021-02-18
 */

#include "animAddNode.h"

TypeHandle AnimAddNode::_type_handle;

/**
 *
 */
AnimAddNode::
AnimAddNode(const std::string &name) :
  AnimGraphNode(name),
  _alpha(1.0f) {
}

/**
 *
 */
void AnimAddNode::
evaluate(AnimGraphEvalContext &context) {
  nassertv(_base != nullptr && _add != nullptr);

  if (_alpha <= 0.001f) {
    // Not doing any adding.  Fast path.
    _base->evaluate(context);

  } else {
    AnimGraphEvalContext base_ctx(context);
    _base->evaluate(base_ctx);

    AnimGraphEvalContext add_ctx(context);
    _add->evaluate(add_ctx);

    for (size_t i = 0; i < context._joints.size(); i++) {
      JointTransform &joint = context._joints[i];
      JointTransform &base_joint = base_ctx._joints[i];
      JointTransform &add_joint = add_ctx._joints[i];

      joint._position = base_joint._position + (add_joint._position * _alpha);
      LQuaternion add_quat = LQuaternion::ident_quat();
      LQuaternion::blend(add_quat, add_joint._rotation, _alpha, add_quat);
      joint._rotation = base_joint._rotation * add_quat;

      // Not adding scale or shear.
      joint._scale = base_joint._scale;
      joint._shear = base_joint._shear;
    }
  }
}
