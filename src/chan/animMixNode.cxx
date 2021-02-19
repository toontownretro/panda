/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animMixNode.cxx
 * @author lachbr
 * @date 2021-02-18
 */

#include "animMixNode.h"

TypeHandle AnimMixNode::_type_handle;

/**
 *
 */
AnimMixNode::
AnimMixNode(const std::string &name) :
  AnimGraphNode(name),
  _alpha(0.0f)
{
}

/**
 *
 */
int AnimMixNode::
get_max_inputs() const {
  return 2;
}

/**
 *
 */
void AnimMixNode::
evaluate(MovingPartMatrix *part, bool frame_blend_flag) {
  nassertv(get_num_inputs() == 2);

  if (_alpha == 0.0f) {
    // Fully in input 0.

    AnimGraphNode *i0 = _inputs[0];
    i0->evaluate(part, frame_blend_flag);

    _position = i0->get_position();
    _rotation = i0->get_rotation();
    _scale = i0->get_scale();
    _shear = i0->get_shear();

  } else if (_alpha == 1.0f) {
    // Fully in input 1.

    AnimGraphNode *i1 = _inputs[1];
    i1->evaluate(part, frame_blend_flag);

    _position = i1->get_position();
    _rotation = i1->get_rotation();
    _scale = i1->get_scale();
    _shear = i1->get_shear();

  } else {
    // Blend between the two.

    AnimGraphNode *i0 = _inputs[0];
    i0->evaluate(part, frame_blend_flag);

    AnimGraphNode *i1 = _inputs[1];
    i1->evaluate(part, frame_blend_flag);

    _position = i0->get_position() + (i1->get_position() - i0->get_position()) * _alpha;
    LQuaternion::slerp(i0->get_rotation(), i1->get_rotation(), _alpha, _rotation);
    _scale = i0->get_scale() + (i1->get_scale() - i0->get_scale()) * _alpha;
    _shear = i0->get_shear() + (i1->get_shear() - i0->get_shear()) * _alpha;
  }
}
