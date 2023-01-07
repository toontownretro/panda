/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file audioSound.I
 * @author jyelon
 * @date 2007-08-01
 */

/**
 * Adjusts the play rate / frequency of the sound by specifying a semitone
 * offset.  A semitone is the difference in pitch between two adjacent notes
 * on a keyboard.  For instance, C to C#.  There are 12 semitones in one
 * octave.
 *
 * An offset of -1 makes the sound play one semitone/note below the base
 * frequency, 1 makes it play one semitone/note above.  0 maintains the
 * base frequency.
 */
INLINE void AudioSound::
set_semitone_offset(PN_stdfloat offset) {
  set_play_rate(cpow(2.0f, offset / 12.0f));
}

/**
 *
 */
INLINE void AudioSound::
set_tone_offset(PN_stdfloat offset) {
  // One tone is two semitones.
  set_semitone_offset(offset * 2.0f);
}

/**
 *
 */
INLINE void AudioSound::
set_semisemitone_offset(PN_stdfloat offset) {
  // Two semisemitones is one semitone.
  set_semitone_offset(offset * 0.5f);
}

/**
 *
 */
INLINE void AudioSound::
set_octave_offset(PN_stdfloat offset) {
  // One octave is 12 semitones.
  set_semitone_offset(offset * 12.0f);
}

/**
 * Adjusts the play rate / frequency of the sound to go from the base note
 * and octave to the specified target note and octave.
 *
 * The play rate is set to the semitone offset between the base and target
 * notes.
 */
INLINE void AudioSound::
set_note_offset(Note base_note, int base_octave, Note target_note, int target_octave) {
  int octave_diff = target_octave - base_octave;
  int note_diff = (int)target_note - (int)base_note;
  set_semitone_offset(octave_diff * 12.0f + note_diff);
}

/**
 * Adds the specified DSP filter to the beginning of the DSP chain.  Returns
 * true if the DSP filter is supported by the audio implementation, false
 * otherwise.
 */
bool AudioSound::
add_dsp_to_head(DSP *dsp) {
  return insert_dsp(0, dsp);
}

/**
 * Adds the specified DSP filter to the end of the DSP chain.  Returns
 * true if the DSP filter is supported by the audio implementation, false
 * otherwise.
 */
bool AudioSound::
add_dsp_to_tail(DSP *dsp) {
  return insert_dsp(get_num_dsps() - 1, dsp);
}

/**
 * Returns true if this was created as a positional sound.
 */
INLINE bool AudioSound::
is_positional() const {
  return _positional;
}
