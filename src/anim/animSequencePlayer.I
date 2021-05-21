/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animSequencePlayer.I
 * @author brian
 * @date 2021-05-19
 */

/**
 *
 */
INLINE PN_stdfloat AnimSequencePlayer::Layer::
get_fade_out(PN_stdfloat frame_time) const {
  PN_stdfloat s;
  if (_layer_fade_out_time <= 0.0f) {
    s = 0.0f;

  } else {
    // Blend in over 0.2 seconds.
    s = 1.0f - (frame_time - _layer_anim_time) / _layer_fade_out_time;
    if (s > 0.0f && s <= 1.0f) {
      // Do a nice spline curve.
      s = 3 * s * s - 2 * s * s * s;
    } else if (s > 1.0f) {
      // Shouldn't happen, but maybe frame_time is behind anim_time?
      s = 1.0f;
    }
  }
  return s;
}

/**
 * Sets the advance mode of the player.  If the mode is AM_auto, the player
 * will automatically advance the animation time and drive the cycle of the
 * playing sequence(s) at evaluation time.  If the mode is AM_manual, the user
 * is responsible for either explicitly calling advance() or supplying the
 * animation time and cycle of the playing sequence(s).
 */
INLINE void AnimSequencePlayer::
set_advance_mode(AdvanceMode mode) {
  _advance_mode = mode;
}

/**
 * Returns the advance mode of the player.  See set_advance_mode().
 */
INLINE AnimSequencePlayer::AdvanceMode AnimSequencePlayer::
get_advance_mode() const {
  return _advance_mode;
}

/**
 * Adds a new sequence to the player.  Returns the index of the added sequence.
 * If the given sequence was already added to the player, returns the already
 * existing index.
 */
INLINE int AnimSequencePlayer::
add_sequence(AnimSequence *seq) {
  Sequences::const_iterator it = std::find(_sequences.begin(), _sequences.end(), seq);
  if (it != _sequences.end()) {
    // Already in there.
    return (int)(it - _sequences.begin());
  }

  int index = (int)_sequences.size();
  _sequences.push_back(seq);
  return index;
}

/**
 * Returns the number of sequences in the player.
 */
INLINE int AnimSequencePlayer::
get_num_sequences() const {
  return (int)_sequences.size();
}

/**
 * Returns the nth sequence in the player.
 */
INLINE AnimSequence *AnimSequencePlayer::
get_sequence(int n) const {
  nassertr(n >= 0 && n < get_num_sequences(), nullptr);
  return _sequences[n];
}

/**
 * Sets the current sequence.
 */
INLINE void AnimSequencePlayer::
set_sequence(int n) {
  //nassertv(n >= 0 && n < get_num_sequences());
  _sequence = n;
}

/**
 * Returns the index of the currently playing sequence.
 */
INLINE int AnimSequencePlayer::
get_curr_sequence() const {
  return _sequence;
}

/**
 * Sets the cycle for the current sequence.
 */
INLINE void AnimSequencePlayer::
set_cycle(PN_stdfloat cycle) {
  _cycle = cycle;
}

/**
 * Returns the cycle of the current sequence.
 */
INLINE PN_stdfloat AnimSequencePlayer::
get_cycle() const {
  return _cycle;
}

/**
 * Sets the animation time.
 */
INLINE void AnimSequencePlayer::
set_anim_time(PN_stdfloat time) {
  _anim_time = time;
}

/**
 * Returns the current animation time.
 */
INLINE PN_stdfloat AnimSequencePlayer::
get_anim_time() const {
  return _anim_time;
}

/**
 * Sets the playback rate of the player.
 */
INLINE void AnimSequencePlayer::
set_play_rate(PN_stdfloat rate) {
  _play_rate = rate;
}

/**
 * Returns the playback rate of the player.
 */
INLINE PN_stdfloat AnimSequencePlayer::
get_play_rate() const {
  return _play_rate;
}

/**
 * Sets whether or not the player should blend sequence changes.  If false, the
 * player will snap to the new sequence when it changes.
 */
INLINE void AnimSequencePlayer::
set_transitions_enabled(bool enable) {
  _transitions_enabled = enable;
}

/**
 * Returns whether or not the player should blend sequence changes.  See
 * set_transitions_enabled().
 */
INLINE bool AnimSequencePlayer::
get_transitions_enabled() const {
  return _transitions_enabled;
}

/**
 * Returns true if the current sequence has finished playing, false otherwise.
 */
INLINE bool AnimSequencePlayer::
is_sequence_finished() const {
  return _sequence_finished;
}

/**
 * Returns true if the current sequence is looping, false otherwise.
 */
INLINE bool AnimSequencePlayer::
is_sequence_looping() const {
  return _sequence_loops;
}

/**
 *
 */
INLINE void AnimSequencePlayer::
set_new_sequence_parity(int parity) {
  _new_sequence_parity = parity;
}

/**
 *
 */
INLINE int AnimSequencePlayer::
get_new_sequence_parity() const {
  return _new_sequence_parity;
}

/**
 *
 */
INLINE int AnimSequencePlayer::
get_prev_sequence_parity() const {
  return _prev_sequence_parity;
}
