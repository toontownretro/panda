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
void AnimMixNode::
evaluate(AnimGraphEvalContext &context) {
  nassertv(_a != nullptr && _b != nullptr);

  if (_alpha <= 0.001f) {
    // Fully in input 0.

    _a->evaluate(context);

  } else if (_alpha >= 0.999f) {
    // Fully in input 1.

    _b->evaluate(context);

  } else {
    // Blend between the two.

    AnimGraphEvalContext a_ctx(context);
    _a->evaluate(a_ctx);

    AnimGraphEvalContext b_ctx(context);
    _b->evaluate(b_ctx);

    context.mix(a_ctx, b_ctx, _alpha);
  }
}
