/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animGraphNode.cxx
 * @author lachbr
 * @date 2021-02-18
 */

#include "animGraphNode.h"

TypeHandle AnimGraphNode::_type_handle;

JointTransformPool joint_transform_pool;

#define FOREACH_INPUT(func) \
  Inputs::const_iterator ii; \
  for (ii = _inputs.begin(); ii != _inputs.end(); ++ii) { \
    (*ii)->func; \
  }

/**
 * Blends between two context poses, stores result on this context.
 */
void AnimGraphEvalContext::
mix(const AnimGraphEvalContext &a, const AnimGraphEvalContext &b, PN_stdfloat frac) {
  PN_stdfloat e0 = 1.0f - frac;

  for (int i = 0; i < _num_joints; i++) {
    JointTransform &joint = _joints[i];
    const JointTransform &a_joint = a._joints[i];
    const JointTransform &b_joint = b._joints[i];

    joint._position = (a_joint._position * e0) + (b_joint._position * frac);
    joint._scale = (a_joint._scale * e0) + (b_joint._scale * frac);
    LQuaternion::slerp(a_joint._rotation, b_joint._rotation, frac, joint._rotation);
  }
}

/**
 *
 */
AnimGraphNode::
AnimGraphNode(const std::string &name) :
  Namable(name)
{
}

/**
 *
 */
AnimGraphNode::
~AnimGraphNode() {
}

/**
 * Notes a new child/input linking to this node.
 */
void AnimGraphNode::
add_child(AnimGraphNode *child) {
  Children::const_iterator it = std::find(_children.begin(), _children.end(), child);
  if (it != _children.end()) {
    return;
  }

  _children.push_back(child);
}

/**
 * Removes an existing child/input linking to this node.
 */
void AnimGraphNode::
remove_child(AnimGraphNode *child) {
  Children::iterator it = std::find(_children.begin(), _children.end(), child);
  if (it == _children.end()) {
    return;
  }

  _children.erase(it);
}

/**
 *
 */
void AnimGraphNode::
evaluate_anims(pvector<AnimBundle *> &anims, vector_stdfloat &weights, PN_stdfloat this_weight) {
  for (Children::const_iterator it = _children.begin(); it != _children.end(); ++it) {
    (*it)->evaluate_anims(anims, weights, this_weight);
  }
}
