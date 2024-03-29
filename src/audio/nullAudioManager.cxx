/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nullAudioManager.cxx
 * @author skyler
 * @date 2001-06-06
 * Prior system by: cary
 */

#include "nullAudioManager.h"

TypeHandle NullAudioManager::_type_handle;

/**
 *
 */
NullAudioManager::
NullAudioManager() {
  audio_info("NullAudioManager");
}

/**
 *
 */
NullAudioManager::
~NullAudioManager() {
  // intentionally blank.
}

/**
 *
 */
bool NullAudioManager::
is_valid() {
  return false;
}

/**
 *
 */
PT(AudioSound) NullAudioManager::
get_sound(const Filename &, bool positional, bool stream) {
  return get_null_sound();
}

/**
 *
 */
PT(AudioSound) NullAudioManager::
get_sound(MovieAudio *sound, bool positional, bool stream) {
  return get_null_sound();
}

/**
 *
 */
PT(AudioSound) NullAudioManager::
get_sound(AudioSound *sound) {
  return get_null_sound();
}

/**
 *
 */
void NullAudioManager::
uncache_sound(const Filename &) {
  // intentionally blank.
}

/**
 *
 */
void NullAudioManager::
clear_cache() {
  // intentionally blank.
}

/**
 *
 */
void NullAudioManager::
set_cache_limit(unsigned int) {
  // intentionally blank.
}

/**
 *
 */
unsigned int NullAudioManager::
get_cache_limit() const {
  // intentionally blank.
  return 0;
}

/**
 *
 */
void NullAudioManager::
set_volume(PN_stdfloat) {
  // intentionally blank.
}

/**
 *
 */
PN_stdfloat NullAudioManager::
get_volume() const {
  return 0;
}

/**
 *
 */
void NullAudioManager::
set_play_rate(PN_stdfloat) {
  // intentionally blank.
}

/**
 *
 */
PN_stdfloat NullAudioManager::
get_play_rate() const {
  return 0;
}

/**
 *
 */
void NullAudioManager::
set_active(bool) {
  // intentionally blank.
}

/**
 *
 */
bool NullAudioManager::
get_active() const {
  return 0;
}

/**
 *
 */
void NullAudioManager::
set_concurrent_sound_limit(unsigned int) {
  // intentionally blank.
}

/**
 *
 */
unsigned int NullAudioManager::
get_concurrent_sound_limit() const {
  return 0;
}

/**
 *
 */
void NullAudioManager::
reduce_sounds_playing_to(unsigned int) {
  // intentionally blank.
}

/**
 *
 */
void NullAudioManager::
stop_all_sounds() {
  // intentionally blank.
}
