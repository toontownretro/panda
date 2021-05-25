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
 * Sets the Character that the player should play sequences from.
 */
INLINE void AnimSequencePlayer::
set_character(Character *character) {
  nassertv(character->get_num_sequences() > 0);
  _character = character;
  // Reset the sequences and layers.
  reset_sequence(0);
  remove_all_gestures();
}

/**
 * Returns the Character that the player is playing sequences from.
 */
INLINE Character *AnimSequencePlayer::
get_character() const {
  return _character;
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
