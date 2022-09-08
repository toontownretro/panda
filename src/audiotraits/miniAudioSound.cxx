/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file miniAudioSound.cxx
 * @author brian
 * @date 2022-09-06
 */

#include "miniAudioSound.h"
#include "miniaudio.h"
#include "virtualFile.h"
#include "miniAudioManager.h"
#include "config_miniaudio.h"

IMPLEMENT_CLASS(MiniAudioSound);

/**
 *
 */
MiniAudioSound::
MiniAudioSound(VirtualFile *file, bool positional, MiniAudioManager *mgr) {
  ma_uint32 flags = 0;
  std::streamsize file_size = file->get_file_size();
  if (miniaudio_preload_threshold >= 0 && file_size > (std::streamsize)miniaudio_preload_threshold) {
    if (miniaudio_cat.is_debug()) {
      miniaudio_cat.debug()
        << file->get_filename() << " is " << file_size << " bytes, streaming from disk\n";
    }
    flags |= MA_SOUND_FLAG_STREAM;
  }
  if (miniaudio_load_and_decode) {
    if (miniaudio_cat.is_debug()) {
      miniaudio_cat.debug()
        << "load and decode\n";
    }
    flags |= MA_SOUND_FLAG_DECODE;
  }
  if (!positional) {
    if (miniaudio_cat.is_debug()) {
      miniaudio_cat.debug()
        << "no spatialization\n";
    }
    flags |= MA_SOUND_FLAG_NO_SPATIALIZATION;
  }
  if (miniaudio_cat.is_debug()) {
    miniaudio_cat.debug()
      << "init sound from file: " << file->get_filename() << ", flags " << flags << "\n";
  }
  _sound = (ma_sound *)PANDA_MALLOC_SINGLE(sizeof(ma_sound));
  ma_result result = ma_sound_init_from_file(mgr->_ma_engine, file->get_filename().get_fullpath().c_str(),
    flags, mgr->_sound_group, nullptr, _sound);
  if (result != MA_SUCCESS) {
    miniaudio_cat.error()
      << "Could not init sound from file " << file->get_filename() << ": " << result << "\n";
  }

  _name = file->get_filename();
}

/**
 *
 */
MiniAudioSound::
MiniAudioSound(MiniAudioSound *other, MiniAudioManager *mgr) :
  _name(other->_name) {

  _sound = (ma_sound *)PANDA_MALLOC_SINGLE(sizeof(ma_sound));
  ma_result result = ma_sound_init_copy(mgr->_ma_engine, other->_sound, 0, mgr->_sound_group, _sound);
  if (result != MA_SUCCESS) {
    miniaudio_cat.error()
      << "Could not init sound copy of " << other->get_name() << ": " << result << "\n";
  }
}

/**
 *
 */
MiniAudioSound::
~MiniAudioSound() {
  if (_sound != nullptr) {
    ma_sound_stop(_sound);
    ma_sound_uninit(_sound);
    PANDA_FREE_SINGLE(_sound);
    _sound = nullptr;
  }
}

/**
 *
 */
void MiniAudioSound::
play() {
  ma_sound_start(_sound);
}

/**
 *
 */
void MiniAudioSound::
stop() {
  ma_sound_stop(_sound);
}

/**
 *
 */
void MiniAudioSound::
set_time(PN_stdfloat time) {
  ma_uint32 sample_rate;
  ma_result result;
  result = ma_sound_get_data_format(_sound, nullptr, nullptr, &sample_rate, nullptr, 0);
  nassertv(result == MA_SUCCESS);
  result = ma_sound_seek_to_pcm_frame(_sound, time * sample_rate);
  nassertv(result == MA_SUCCESS);
}

/**
 *
 */
PN_stdfloat MiniAudioSound::
get_time() const {
  float cursor;
  ma_result result = ma_sound_get_cursor_in_seconds(_sound, &cursor);
  nassertr(result == MA_SUCCESS, 0.0f);
  return cursor;
}

/**
 *
 */
void MiniAudioSound::
set_volume(PN_stdfloat volume) {
  ma_sound_set_volume(_sound, volume);
}

/**
 *
 */
PN_stdfloat MiniAudioSound::
get_volume() const {
  return ma_sound_get_volume(_sound);
}

/**
 *
 */
void MiniAudioSound::
set_balance(PN_stdfloat balance) {
  ma_sound_set_pan(_sound, balance);
}

/**
 *
 */
PN_stdfloat MiniAudioSound::
get_balance() const {
  return ma_sound_get_pan(_sound);
}

/**
 *
 */
void MiniAudioSound::
set_play_rate(PN_stdfloat play_rate) {
  ma_sound_set_pitch(_sound, play_rate);
}

/**
 *
 */
PN_stdfloat MiniAudioSound::
get_play_rate() const {
  return ma_sound_get_pitch(_sound);
}

/**
 *
 */
void MiniAudioSound::
set_loop(bool loop) {
  ma_sound_set_looping(_sound, loop);
}

/**
 *
 */
bool MiniAudioSound::
get_loop() const {
  return ma_sound_is_looping(_sound);
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
 * Note, this is global across all MiniAudioSounds referencing the same sound
 * data.
 */
void MiniAudioSound::
set_loop_range(PN_stdfloat start, PN_stdfloat end) {
  float length;
  ma_result result = ma_sound_get_length_in_seconds(_sound, &length);
  nassertv(result == MA_SUCCESS);

  nassertv(start <= length && start >= 0.0f);

  if (end < 0.0f || end < start) {
    end = length;
  }

  ma_uint32 sample_rate;
  result = ma_sound_get_data_format(_sound, nullptr, nullptr, &sample_rate, nullptr, 0);
  nassertv(result == MA_SUCCESS);

  result = ma_data_source_set_loop_point_in_pcm_frames(
    ma_sound_get_data_source(_sound), start * sample_rate, end * sample_rate);
  nassertv(result == MA_SUCCESS);
}

/**
 *
 */
void MiniAudioSound::
set_active(bool flag) {
}

/**
 *
 */
bool MiniAudioSound::
get_active() const {
  return true;
}

/**
 *
 */
void MiniAudioSound::
set_finished_event(const std::string &event) {
  _finished_event = event;
}

/**
 *
 */
const std::string &MiniAudioSound::
get_finished_event() const {
  return _finished_event;
}

/**
 *
 */
const std::string &MiniAudioSound::
get_name() const {
  return _name;
}

/**
 *
 */
PN_stdfloat MiniAudioSound::
length() const {
  float length;
  ma_result result = ma_sound_get_length_in_seconds(_sound, &length);
  nassertr(result == MA_SUCCESS, 0.01f);
  return length;
}

/**
 *
 */
AudioSound::SoundStatus MiniAudioSound::
status() const {
  if (_sound == nullptr) {
    return AudioSound::BAD;

  } else if (ma_sound_is_playing(_sound)) {
    return AudioSound::PLAYING;

  } else {
    return AudioSound::READY;
  }
}
