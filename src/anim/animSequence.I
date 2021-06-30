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
  _fade_out(0.2f),
  _activity(0),
  _activity_weight(1.0f),
  _num_frames(1),
  _frame_rate(30)
{
  if (base != nullptr) {
    set_base(base);
  }
}

/**
 * Returns the number of events in the sequence.
 */
INLINE int AnimSequence::
get_num_events() const {
  return (int)_events.size();
}

/**
 * Returns the nth event of the sequence.
 */
INLINE const AnimSequence::AnimEvent &AnimSequence::
get_event(int n) const {
  return _events[n];
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
 * Sets the activity index associated with this sequence.  The animation system
 * doesn't make use of this, the activity is intended to be interpreted by
 * game code, but it's stored on the sequence for convenience.
 */
INLINE void AnimSequence::
set_activity(int activity, PN_stdfloat weight) {
  _activity = activity;
  _activity_weight = weight;
}

/**
 * Returns the activity index associated with the sequence.
 */
INLINE int AnimSequence::
get_activity() const {
  return _activity;
}

/**
 * Returns the weight of the activity associated with the sequence.
 */
INLINE PN_stdfloat AnimSequence::
get_activity_weight() const {
  return _activity_weight;
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
add_layer(AnimGraphNode *seq, PN_stdfloat start_frame, PN_stdfloat peak_frame,
          PN_stdfloat tail_frame, PN_stdfloat end_frame, bool spline,
          bool no_blend, bool xfade, int pose_param) {
  Layer layer;
  layer._seq = seq;

  if (pose_param == -1) {
    PN_stdfloat num_frames = std::max(1.0f, get_num_frames() - 1.0f);
    layer._start = start_frame / num_frames;
    layer._peak = peak_frame / num_frames;
    layer._tail = tail_frame / num_frames;
    layer._end = end_frame / num_frames;

  } else {
    layer._start = start_frame;
    layer._peak = peak_frame;
    layer._tail = tail_frame;
    layer._end = end_frame;
  }

  layer._spline = spline;
  layer._no_blend = no_blend;
  layer._xfade = xfade;
  layer._pose_parameter = pose_param;

  _layers.push_back(layer);
}
