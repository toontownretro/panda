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
int AnimAddNode::
get_max_inputs() const {
  return 2;
}

/**
 *
 */
void AnimAddNode::
evaluate(MovingPartMatrix *part, bool frame_blend_flag) {
  nassertv(get_num_inputs() == 2);

  if (_alpha == 0.0f) {
    // Not doing any adding.  Fast path.
    AnimGraphNode *base = _inputs[0];

    _position = base->get_position();
    _rotation = base->get_rotation();
    _scale = base->get_scale();
    _shear = base->get_shear();

  } else {
    AnimGraphNode *base = _inputs[0];
    base->evaluate(part, frame_blend_flag);

    AnimGraphNode *layer = _inputs[1];
    layer->evaluate(part, frame_blend_flag);

    _position = base->get_position() + (layer->get_position() * _alpha);

    LQuaternion add_quat = LQuaternion::ident_quat();
    LQuaternion::blend(add_quat, layer->get_rotation(), _alpha, add_quat);
    _rotation = base->get_rotation() * add_quat;

    // Not adding scale or shear.
    _shear = base->get_shear();
    _scale = base->get_scale();
  }
}
