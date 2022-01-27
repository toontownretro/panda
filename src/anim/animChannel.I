/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannel.I
 * @author brian
 * @date 2021-08-04
 */

/**
 * Sets the number of frames in the channel.
 */
INLINE void AnimChannel::
set_num_frames(int count) {
  if (count < 1) {
    count = 1;
  }
  _num_frames = count;
}

/**
 * Returns the number of frames in the channel.
 */
INLINE int AnimChannel::
get_num_frames() const {
  return _num_frames;
}

/**
 * Sets the frame rate of the channel.
 */
INLINE void AnimChannel::
set_frame_rate(PN_stdfloat fps) {
  if (fps <= 0.0f) {
    fps = 1.0f;
  }
  _fps = fps;
}

/**
 * Returns the frame rate of the channel.
 */
INLINE PN_stdfloat AnimChannel::
get_frame_rate() const {
  return _fps;
}

/**
 * Returns the cycles-per-second of the channel.  This is just the reciprocal
 * of the duration of the channel.  The character instance is required because
 * blended channel types use pose parameter values to blend the lengths.
 */
INLINE PN_stdfloat AnimChannel::
get_cycle_rate(Character *character) const {
  PN_stdfloat length = get_length(character);
  if (length > 0.0f) {
    return 1.0f / length;
  } else {
    return 1.0f / 0.1f;
  }
}

/**
 * Sets the indicated channel flags.
 */
INLINE void AnimChannel::
set_flags(unsigned int flags) {
  _flags |= flags;
}

/**
 * Returns true if the indicated channel flags are set.
 */
INLINE bool AnimChannel::
has_flags(unsigned int flags) const {
  return (_flags & flags) != 0;
}

/**
 * Returns the set of channel flags that are set.
 */
INLINE unsigned int AnimChannel::
get_flags() const {
  return _flags;
}

/**
 * Clears the indicated channel flags.
 */
INLINE void AnimChannel::
clear_flags(unsigned int flags) {
  _flags &= (~flags);
}

/**
 * Returns the number of events in the channel.
 */
INLINE int AnimChannel::
get_num_events() const {
  return (int)_events.size();
}

/**
 * Returns the nth event of the channel.
 */
INLINE const AnimChannel::Event &AnimChannel::
get_event(int n) const {
  assert(n >= 0 && n < (int)_events.size());
  return _events[n];
}

/**
 * Returns the number of IK locks on the channel.
 */
INLINE int AnimChannel::
get_num_ik_locks() const {
  return (int)_ik_locks.size();
}

/**
 * Returns the nth IK lock on the channel.
 */
INLINE const AnimChannel::IKLock &AnimChannel::
get_ik_lock(int n) const {
  assert(n >= 0 && n < (int)_ik_locks.size());
  return _ik_locks[n];
}

/**
 * Associates the channel with the indicated activity with the given weight.
 */
INLINE void AnimChannel::
add_activity(int activity, PN_stdfloat weight) {
  _activities.push_back({ activity, weight });
}

/**
 * Returns the number of activities the channel is associated with.
 */
INLINE int AnimChannel::
get_num_activities() const {
  return (int)_activities.size();
}

/**
 * Returns the nth activity the channel is associated with.
 */
INLINE int AnimChannel::
get_activity(int n) const {
  nassertr(n >= 0 && n < (int)_activities.size(), -1);
  return _activities[n].activity;
}

/**
 * Returns the weight of the nth activity the channel is associated with.
 */
INLINE PN_stdfloat AnimChannel::
get_activity_weight(int n) const {
  nassertr(n >= 0 && n < (int)_activities.size(), 0.0f);
  return _activities[n].weight;
}

/**
 * Sets a list of per-joint weights to use for the evaluated pose of this
 * channel.
 */
INLINE void AnimChannel::
set_weight_list(WeightList *list) {
  _weights = list;
}

/**
 * Returns the list of per-joint weights being used for the evaluated pose of
 * this channel.
 */
INLINE WeightList *AnimChannel::
get_weight_list() const {
  return _weights;
}

/**
 * Sets the duration of a transition into this channel.
 */
INLINE void AnimChannel::
set_fade_in(PN_stdfloat time) {
  _fade_in = time;
}

/**
 * Returns the duration of a transition into this channel.
 */
INLINE PN_stdfloat AnimChannel::
get_fade_in() const {
  return _fade_in;
}

/**
 * Sets the duration of a transition out of this channel.
 */
INLINE void AnimChannel::
set_fade_out(PN_stdfloat time) {
  _fade_out = time;
}

/**
 * Returns the duration of a transition out of this channel.
 */
INLINE PN_stdfloat AnimChannel::
get_fade_out() const {
  return _fade_out;
}
