/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animSequence.I
 * @author lachbr
 * @date 2021-03-01
 */

/**
 *
 */
INLINE AnimSequence::
AnimSequence(const std::string &name, AnimGraphNode *base) :
  AnimGraphNode(name)
{
  if (base != nullptr) {
    set_base(base);
  }
}

/**
 * Returns the effective AnimControl for this sequence.  That is, the
 * AnimControl below this node with the highest frame count.
 */
INLINE AnimControl *AnimSequence::
get_effective_control() const {
  return _effective_control;
}

/**
 * Sets the animation sub-graph that should be used as the base pose for this
 * sequence.
 */
INLINE void AnimSequence::
set_base(AnimGraphNode *node) {
  if (_base != nullptr) {
    remove_child(_base);
  }

  _base = node;
  add_child(_base);

  compute_effective_control();
}

/**
 * Adds a sub-graph that should be treated as an additive layer onto the base
 * pose of this sequence.
 */
INLINE void AnimSequence::
add_layer(AnimGraphNode *node) {
  _add_layers.push_back(node);
  add_child(node);

  compute_effective_control();
}
