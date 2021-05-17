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
  AnimGraphNode(name),
  _flags(F_none),
  _fade_in(0.2f),
  _fade_out(0.2f)
{
  if (base != nullptr) {
    set_base(base);
  }
}

/**
 * Returns the length of the sequence in seconds.
 */
INLINE PN_stdfloat AnimSequence::
get_length() const {
  if (_effective_control == nullptr) {
    return 0.0f;
  }

  return (PN_stdfloat)_effective_control->get_num_frames() /
    (_effective_control->get_anim()->get_base_frame_rate() * _effective_control->get_play_rate());
}

/**
 *
 */
INLINE void AnimSequence::
set_fade_in(PN_stdfloat time) {
  _fade_in = time;
}

/**
 *
 */
INLINE PN_stdfloat AnimSequence::
get_fade_in() const {
  return _fade_in;
}

/**
 *
 */
INLINE void AnimSequence::
set_fade_out(PN_stdfloat time) {
  _fade_out = time;
}

/**
 *
 */
INLINE PN_stdfloat AnimSequence::
get_fade_out() const {
  return _fade_out;
}

/**
 *
 */
INLINE void AnimSequence::
set_flags(unsigned int flags) {
  _flags |= flags;
}

/**
 *
 */
INLINE bool AnimSequence::
has_flags(unsigned int flags) const {
  return (_flags & flags) != 0;
}

/**
 *
 */
INLINE unsigned int AnimSequence::
get_flags() const {
  return _flags;
}

/**
 *
 */
INLINE void AnimSequence::
clear_flags(unsigned int flags) {
  _flags &= (~flags);
}

/**
 * Sets a list of per-joint weights to use for the evaluated pose of this
 * sequence.
 */
INLINE void AnimSequence::
set_weight_list(WeightList *list) {
  _weights = list;
}

/**
 * Returns the list of per-joint weights being used for the evaluated pose of
 * this sequence.
 */
INLINE WeightList *AnimSequence::
get_weight_list() const {
  return _weights;
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
 * Adds a sub-graph that should be treated as an additive or blended layer
 * onto the base pose of this sequence.
 */
INLINE void AnimSequence::
add_layer(AnimGraphNode *seq, int start_frame, int peak_frame,
          int tail_frame, int end_frame, bool spline,
          bool no_blend) {
  Layer layer;
  layer._seq = seq;
  layer._start_frame = start_frame;
  layer._peak_frame = peak_frame;
  layer._tail_frame = tail_frame;
  layer._end_frame = end_frame;
  layer._spline = spline;
  layer._no_blend = no_blend;

  _layers.push_back(layer);

  add_child(layer._seq);

  compute_effective_control();
}
