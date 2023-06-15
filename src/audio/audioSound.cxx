/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file audioSound.cxx
 * @author skyler
 * @date 2001-06-06
 * Prior system by: cary
 */

#include "audioSound.h"

using std::ostream;

TypeHandle AudioSound::_type_handle;

/**
 *
 */
AudioSound::
~AudioSound() {
}

/**
 *
 */
AudioSound::
AudioSound() {
  // Intentionally blank.
}

/**
 *
 */
void AudioSound::
set_3d_attributes(const LPoint3 &pos, const LQuaternion &quat, const LVector3 &vel) {
  // Intentionally blank.
}

/**
 *
 */
LPoint3 AudioSound::
get_3d_position() const {
  return LPoint3();
}

/**
 *
 */
LQuaternion AudioSound::
get_3d_quat() const {
  return LQuaternion::ident_quat();
}

/**
 *
 */
LVector3 AudioSound::
get_3d_velocity() const {
  return LVector3();
}

/**
 *
 */
void AudioSound::
set_3d_direction(LVector3 d) {
  // Intentionally blank.
}

LVector3 AudioSound::
get_3d_direction() const {
  // Intentionally blank.
  return { 0.0f, 0.0f, 0.0f };
}

void AudioSound::
set_3d_min_distance(PN_stdfloat dist) {
  // Intentionally blank.
}

/**
 *
 */
PN_stdfloat AudioSound::
get_3d_min_distance() const {
  // Intentionally blank.
  return 0.0f;
}

/**
 * Returns the default frequency/sample rate of the audio file.
 */
PN_stdfloat AudioSound::
get_sound_frequency() const {
  return 0.0f;
}

void AudioSound::
set_3d_cone_inner_angle(PN_stdfloat angle) {
  // Intentionally blank.
}

PN_stdfloat AudioSound::
get_3d_cone_inner_angle() const {
  // Intentionally blank.
  return 0.0f;
}

void AudioSound::
set_3d_cone_outer_angle(PN_stdfloat angle) {
  // Intentionally blank.
}

PN_stdfloat AudioSound::
get_3d_cone_outer_angle() const {
  // Intentionally blank.
  return 0.0f;
}

void AudioSound::
set_3d_cone_outer_gain(PN_stdfloat gain) {
  // Intentionally blank.
}

PN_stdfloat AudioSound::
get_3d_cone_outer_gain() const {
  // Intentionally blank.
  return 0.0f;
}

/**
 *
 */
int AudioSound::
get_priority() {
  // intentionally blank
  return 0;
}

/**
 *
 */
void AudioSound::
set_priority(int priority) {
  // intentionally blank
}

/**
 * Inserts the specified DSP filter into the DSP chain at the specified index.
 * Returns true if the DSP filter is supported by the audio implementation,
 * false otherwise.
 */
bool AudioSound::
insert_dsp(int index, DSP *dsp) {
  // Must be implemented by audio implementation.
  return false;
}

/**
 * Removes the specified DSP filter from the DSP chain. Returns true if the
 * filter was in the DSP chain and was removed, false otherwise.
 */
bool AudioSound::
remove_dsp(DSP *dsp) {
  // Must be implemented by audio implementation.
  return false;
}

/**
 * Removes all DSP filters from the DSP chain.
 */
void AudioSound::
remove_all_dsps() {
  // Must be implemented in audio implementation.
}

/**
 * Returns the number of DSP filters present in the DSP chain.
 */
int AudioSound::
get_num_dsps() const {
  return 0;
}

/**
 * Applies the set of specified Steam Audio simulation and spatialization
 * properties to the sound.
 */
void AudioSound::
apply_steam_audio_properties(const SteamAudioProperties &props) {
}

/**
 * Specifies the loop range of the sound.  This is used to constrain loops
 * to a specific section of the sound, rather than looping the entire sound.
 * An example of this would be a single music file that contains an intro and
 * a looping section.
 *
 * The start and end points are in seconds.  If end is < 0 or < start, it is
 * implicitly set to the length of the sound.
 *
 * This is currently only implemented in FMOD.
 */
void AudioSound::
set_loop_range(PN_stdfloat start, PN_stdfloat end) {
}

/**
 *
 */
void AudioSound::
output(ostream &out) const {
  out << get_type() << " " << get_name() << " " << status();
}

/**
 *
 */
void AudioSound::
write(ostream &out) const {
  out << (*this) << "\n";
}

/**
 *
 */
ostream &
operator << (ostream &out, AudioSound::SoundStatus status) {
  switch (status) {
  case AudioSound::BAD:
    return out << "BAD";

  case AudioSound::READY:
    return out << "READY";

  case AudioSound::PLAYING:
    return out << "PLAYING";
  }

  return out << "**invalid AudioSound::SoundStatus(" << (int)status << ")**";
}
