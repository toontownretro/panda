/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fmodAudioSound.cxx
 * @author cort
 * @date 2003-01-22
 * @author ben
 * @date 2003-10-22
 * Prior system by: cary
 * @author Stan Rosenbaum "Staque" - Spring 2006
 * @author lachbr
 * @date 2020-10-04
 */

#include "pandabase.h"
#include "dcast.h"

// Panda Headers
#include "config_audio.h"
#include "config_fmodAudio.h"
#include "fmodAudioSound.h"
#include "string_utils.h"
#include "subfileInfo.h"
#include "reMutexHolder.h"
#include "virtualFileSystem.h"
#include "fmodSoundCache.h"
#include "throw_event.h"
#include "vector_uchar.h"
#include "clockObject.h"

TypeHandle FMODAudioSound::_type_handle;

#define CHANNEL_INVALID(result) (result == FMOD_ERR_INVALID_HANDLE || result == FMOD_ERR_CHANNEL_STOLEN)

/**
 * Constructor All sound will DEFAULT load as a 2D sound unless otherwise
 * specified.
 */
FMODAudioSound::
FMODAudioSound(AudioManager *manager, VirtualFile *file, bool positional) {
  ReMutexHolder holder(FMODAudioManager::_lock);
  audio_debug("FMODAudioSound::FMODAudioSound() Creating new sound, filename: "
              << file->get_original_filename());

  _active = manager->get_active();
  _paused = false;
  _start_time = 0.0;
  _balance = 0.0;
  _volume = 1.0;
  _playrate = 1.0;
  _is_midi = false;
  _length = 0;
  _last_update_frame = 0;

  // 3D attributes of the sound.
  _location.x = 0;
  _location.y = 0;
  _location.z = 0;

  _velocity.x = 0;
  _velocity.y = 0;
  _velocity.z = 0;

  _min_dist = 1.0;
  _max_dist = 1000000000.0;

  // These set the speaker levels to a default if you are using a multichannel
  // setup.
  for (int i = 0; i < AudioManager::SPK_COUNT; i++) {
    _mix[i] = 1.0;
  }

  FMOD_RESULT result;

  // Assign the values we need
  FMODAudioManager *fmanager;
  DCAST_INTO_V(fmanager, manager);
  _manager = fmanager;

  _channel = nullptr;
  _file_name = file->get_original_filename();
  _file_name.set_binary();

  // Get the Speaker Mode [Important for later on.]
  result = _manager->get_speaker_mode(_speakermode);
  fmod_audio_errcheck("_system->getSpeakerMode()", result);

  _sound_handle = FMODSoundCache::get_global_ptr()->get_sound(_manager, file, positional);
  _sound = _sound_handle->get_sound();

  _is_midi = _file_name.get_extension() == "mid";

  // This is just to collect the defaults of the sound, so we don't Have to
  // query FMOD everytime for the info.  It is also important we get the
  // '_sample_frequency' variable here, for the 'set_play_rate()' and
  // 'get_play_rate()' methods later;

  result = _sound->getDefaults(&_sample_frequency, &_priority);
  fmod_audio_errcheck("_sound->getDefaults()", result);

  // Store off the original length of the sound without any play rate changes
  // applied.  We need this to figure out the loop points of MIDIs that have
  // been sped up.
  result = _sound->getLength(&_length, FMOD_TIMEUNIT_MS);
  fmod_audio_errcheck("_sound->getLength()", result);
}

/**
 * Initializes an FMODAudioSound from an existing FMODAudioSound.  Shares the
 * sound data but creates a new channel.
 */
FMODAudioSound::
FMODAudioSound(AudioManager *manager, FMODAudioSound *copy) {
  ReMutexHolder holder(FMODAudioManager::_lock);
  audio_debug("FMODAudioSound::FMODAudioSound() Creating channel from existing "
              "sound handle");

  _active = manager->get_active();
  _paused = false;
  _start_time = 0.0;
  _balance = copy->_balance;
  _volume = copy->_volume;
  _playrate = copy->_playrate;
  _is_midi = copy->_is_midi;
  _length = copy->_length;
  _last_update_frame = 0;

  // 3D attributes of the sound.
  _location.x = 0;
  _location.y = 0;
  _location.z = 0;

  _velocity.x = 0;
  _velocity.y = 0;
  _velocity.z = 0;

  _min_dist = copy->_min_dist;
  _max_dist = copy->_max_dist;

  _dsps = copy->_dsps;

  // These set the speaker levels to a default if you are using a multichannel
  // setup.
  for (int i = 0; i < AudioManager::SPK_COUNT; i++) {
    _mix[i] = copy->_mix[i];
  }

  FMOD_RESULT result;

  // Assign the values we need
  FMODAudioManager *fmanager;
  DCAST_INTO_V(fmanager, manager);
  _manager = fmanager;

  _channel = nullptr;
  _file_name = copy->_file_name;

  // Get the Speaker Mode [Important for later on.]
  result = _manager->get_speaker_mode(_speakermode);
  fmod_audio_errcheck("_system->getSpeakerMode()", result);

  _sound_handle = copy->_sound_handle;
  nassertv(_sound_handle != nullptr);

  _sound = _sound_handle->get_sound();
  nassertv(_sound != nullptr);

  _sample_frequency = copy->_sample_frequency;
  _priority = copy->_priority;
}

/**
 * DESTRUCTOR!!!
 */
FMODAudioSound::
~FMODAudioSound() {
  ReMutexHolder holder(FMODAudioManager::_lock);
  FMOD_RESULT result;

  audio_debug("Released FMODAudioSound\n");

  for (int i = 0; i < (int)_dsps.size(); i++) {
    if (_dsps[i] != nullptr) {
      result = _dsps[i]->release();
      fmod_audio_errcheck("release dsp on destruct", result);
    }
  }
  _dsps.clear();

  _manager->release_sound(this);
}

/**
 * Plays a sound.
 */
void FMODAudioSound::
play() {
  start_playing();
}

/**
 * Stop a sound
 */
void FMODAudioSound::
stop() {
  ReMutexHolder holder(FMODAudioManager::_lock);

  FMOD_RESULT result;

  if (_channel != nullptr) {
    result = _channel->stop();
    if (!CHANNEL_INVALID(result)) {
      fmod_audio_errcheck("_channel->stop()", result);
    }
    _channel = nullptr;
  }

  _start_time = 0.0;
  _paused = false;

  _manager->stopping_sound(this);
}

/**
 * Turns looping on or off.
 */
void FMODAudioSound::
set_loop(bool loop) {
  set_loop_count(loop ? 0 : 1);
}

/**
 * Returns whether looping is on or off.
 */
bool FMODAudioSound::
get_loop() const {
  return get_loop_count() != 1;
}

/**
 * Panda uses 0 to mean loop forever.  Fmod uses negative numbers to mean loop
 * forever.  (0 means don't loop, 1 means play twice, etc.  We must convert!
 */
void FMODAudioSound::
set_loop_count(unsigned long loop_count) {
  ReMutexHolder holder(FMODAudioManager::_lock);
  audio_debug("FMODAudioSound::set_loop_count()   Setting the sound's loop count to: " << loop_count);

  // LOCALS
  FMOD_RESULT result;

  if (loop_count == 0) {
    result = _sound->setLoopCount(-1);
    fmod_audio_errcheck("_sound->setLoopCount()", result);
    result =_sound->setMode(FMOD_LOOP_NORMAL);
    fmod_audio_errcheck("_sound->setMode()", result);
  } else if (loop_count == 1) {
    result = _sound->setLoopCount(1);
    fmod_audio_errcheck("_sound->setLoopCount()", result);
    result =_sound->setMode(FMOD_LOOP_OFF);
    fmod_audio_errcheck("_sound->setMode()", result);
  } else {
    result = _sound->setLoopCount(loop_count);
    fmod_audio_errcheck("_sound->setLoopCount()", result);
    result =_sound->setMode(FMOD_LOOP_NORMAL);
    fmod_audio_errcheck("_sound->setMode()", result);
  }

  audio_debug("FMODAudioSound::set_loop_count()   Sound's loop count should be set to: " << loop_count);
}

/**
 * Return how many times a sound will loop.
 */
unsigned long FMODAudioSound::
get_loop_count() const {
  ReMutexHolder holder(FMODAudioManager::_lock);
  FMOD_RESULT result;
  int loop_count;

  result = _sound->getLoopCount(&loop_count);
  fmod_audio_errcheck("_sound->getLoopCount()", result);

  if (loop_count <= 0) {
    return 0;
  } else {
    return (unsigned long)loop_count;
  }
}

/**
 * Sets the time at which the next play() operation will begin.  If we are
 * already playing, skips to that time immediatey.
 */
void FMODAudioSound::
set_time(PN_stdfloat start_time) {
  ReMutexHolder holder(FMODAudioManager::_lock);
  _start_time = start_time;

  if (status() == PLAYING) {
    // Already playing; skip to the indicated time.
    start_playing();
  }
}

/**
 * Gets the play position within the sound
 */
PN_stdfloat FMODAudioSound::
get_time() const {
  ReMutexHolder holder(FMODAudioManager::_lock);
  FMOD_RESULT result;
  unsigned int current_time_ms;

  if (_channel == nullptr) {
    return 0.0f;
  }

  result = _channel->getPosition(&current_time_ms, FMOD_TIMEUNIT_MS);
  if (CHANNEL_INVALID(result)) {
    ((FMODAudioSound *)this)->_channel = nullptr;
    return 0.0f;
  } else {
    fmod_audio_errcheck("_channel->getPosition()", result);
  }

  return current_time_ms * 0.001;
}

/**
 * 0.0 to 1.0 scale of volume converted to Fmod's internal 0.0 to 255.0 scale.
 */
void FMODAudioSound::
set_volume(PN_stdfloat vol) {
  ReMutexHolder holder(FMODAudioManager::_lock);
  _volume = vol;
  set_volume_on_channel();
}

/**
 * Gets the current volume of a sound.  1 is Max.  O is Min.
 */
PN_stdfloat FMODAudioSound::
get_volume() const {
  return _volume;
}

/**
 * Starts the sound playing at _start_time.
 */
void FMODAudioSound::
start_playing() {
  ReMutexHolder holder(FMODAudioManager::_lock);
  FMOD_RESULT result;

  if (!_active) {
    _paused = true;
    return;
  }

  _manager->starting_sound(this);

  int start_time_ms = (int)(_start_time * 1000);

  if (_channel != nullptr) {
    // try backing up current sound.
    result = _channel->setPosition(start_time_ms, FMOD_TIMEUNIT_MS);
    if (CHANNEL_INVALID(result)) {
      _channel = nullptr;
    } else {
      fmod_audio_errcheck("_channel->setPosition()", result);

      result = _channel->setPaused(false);
      fmod_audio_errcheck("_channel->setPaused()", result);
    }
  }

  if (_channel == nullptr) {
    result = _manager->_system->playSound(_sound, _manager->_channelgroup, true, &_channel);
    fmod_audio_errcheck("playSound()", result);
    nassertv(_channel != nullptr);

    result = _channel->setPosition(start_time_ms, FMOD_TIMEUNIT_MS);
    fmod_audio_errcheck("_channel->setPosition()", result);

    set_volume_on_channel();
    set_play_rate_on_channel();
    set_speaker_mix_or_balance_on_channel();
    set_3d_attributes_on_channel();
    set_dsps_on_channel();

    result = _channel->setPaused(false);
    fmod_audio_errcheck("_channel->setPaused()", result);
  }

  bool playing = false;
  result = _channel->isPlaying(&playing);
  fmod_audio_errcheck("_channel->isPlaying()", result);

  _start_time = 0.0;

  nassertv(playing);
}

/**
 * Set the volume on a prepared Sound channel.
 */
void FMODAudioSound::
set_volume_on_channel() {
  ReMutexHolder holder(FMODAudioManager::_lock);
  FMOD_RESULT result;

  if (_channel != nullptr) {
    result = _channel->setVolume(_volume);
    if (CHANNEL_INVALID(result)) {
      _channel = nullptr;
    } else {
      fmod_audio_errcheck("_channel->setVolume()", result);
    }
  }
}

/**
 * -1.0 to 1.0 scale
 */
void FMODAudioSound::
set_balance(PN_stdfloat bal) {
  ReMutexHolder holder(FMODAudioManager::_lock);
  _balance = bal;
  set_speaker_mix_or_balance_on_channel();
}

/**
 * -1.0 to 1.0 scale -1 should be all the way left.  1 is all the way to the
 * right.
 */
PN_stdfloat FMODAudioSound::
get_balance() const {
  return _balance;
}

/**
 * Sets the speed at which a sound plays back.  The rate is a multiple of the
 * sound, normal playback speed.  IE 2 would play back 2 times fast, 3 would
 * play 3 times, and so on.  This can also be set to a negative number so a
 * sound plays backwards.  But rememeber if the sound is not playing, you must
 * set the sound's time to its end to hear a song play backwards.
 */
void FMODAudioSound::
set_play_rate(PN_stdfloat rate) {
  ReMutexHolder holder(FMODAudioManager::_lock);
  _playrate = rate;
  set_play_rate_on_channel();
}

/**
 *
 */
PN_stdfloat FMODAudioSound::
get_play_rate() const {
  return _playrate;
}

/**
 * Set the play rate on a prepared Sound channel.
 */
void FMODAudioSound::
set_play_rate_on_channel() {
  ReMutexHolder holder(FMODAudioManager::_lock);
  FMOD_RESULT result;

  if (_is_midi) {
    // If this is a MIDI sequence, simply adjust the speed at which the song is
    // played.  This makes the song play faster without increasing the pitch.
    result = _sound->setMusicSpeed(_playrate);
    fmod_audio_errcheck("_sound->setMusicSpeed()", result);

    // We have to manually fix up the loop points when changing the speed of a
    // MIDI because FMOD does not handle this for us.
    result = _sound->setLoopPoints(0, FMOD_TIMEUNIT_MS, _length / _playrate, FMOD_TIMEUNIT_MS);
    fmod_audio_errcheck("_sound->setLoopPoints()", result);

  } else if (_channel != nullptr) {
    // We have to adjust the frequency for non-sequence sounds.  The sound will
    // play faster, but will also have an increase in pitch.

    result = _channel->setPitch(_playrate);
    if (CHANNEL_INVALID(result)) {
      _channel = nullptr;
    } else {
      fmod_audio_errcheck("_channel->setFrequency()", result);
    }
  }
}

/**
 * Get name of sound file
 */
const std::string& FMODAudioSound::
get_name() const {
  return _file_name;
}

/**
 * Returns the length of the sound in seconds.  Factors in the current play
 * rate.
 */
PN_stdfloat FMODAudioSound::
length() const {
  ReMutexHolder holder(FMODAudioManager::_lock);

  if (_playrate == 0.0f) {
    return 0.0f;
  }

  FMOD_RESULT result;
  unsigned int length;

  result = _sound->getLength(&length, FMOD_TIMEUNIT_MS);
  fmod_audio_errcheck("_sound->getLength()", result);

  return (((double)length) / 1000.0) / _playrate;
}

/**
 * Set position and velocity of this sound NOW LISTEN UP!!! THIS IS IMPORTANT!
 * Both Panda3D and FMOD use a left handed coordinate system.  But there is a
 * major difference!  In Panda3D the Y-Axis is going into the Screen and the
 * Z-Axis is going up.  In FMOD the Y-Axis is going up and the Z-Axis is going
 * into the screen.  The solution is simple, we just flip the Y and Z axis, as
 * we move coordinates from Panda to FMOD and back.  What does did mean to
 * average Panda user?  Nothing, they shouldn't notice anyway.  But if you
 * decide to do any 3D audio work in here you have to keep it in mind.  I told
 * you, so you can't say I didn't.
 */
void FMODAudioSound::
set_3d_attributes(PN_stdfloat px, PN_stdfloat py, PN_stdfloat pz, PN_stdfloat vx, PN_stdfloat vy, PN_stdfloat vz) {
  ReMutexHolder holder(FMODAudioManager::_lock);
  _location.x = px;
  _location.y = pz;
  _location.z = py;

  _velocity.x = vx;
  _velocity.y = vz;
  _velocity.z = vy;

  set_3d_attributes_on_channel();
}

/**
 *
 */
void FMODAudioSound::
set_3d_attributes_on_channel() {
  ReMutexHolder holder(FMODAudioManager::_lock);
  FMOD_RESULT result;
  FMOD_MODE soundMode;

  result = _sound->getMode(&soundMode);
  fmod_audio_errcheck("_sound->getMode()", result);

  if ((_channel != nullptr) && (soundMode & FMOD_3D) != 0) {
    result = _channel->set3DAttributes(&_location, &_velocity);
    if (CHANNEL_INVALID(result)) {
      _channel = nullptr;
    } else {
      fmod_audio_errcheck("_channel->set3DAttributes()", result);
    }
  }
}

/**
 * Get position and velocity of this sound Currently unimplemented.  Get the
 * attributes of the attached object.
 */
void FMODAudioSound::
get_3d_attributes(PN_stdfloat *px, PN_stdfloat *py, PN_stdfloat *pz, PN_stdfloat *vx, PN_stdfloat *vy, PN_stdfloat *vz) {
  audio_error("get3dAttributes: Currently unimplemented. Get the attributes of the attached object.");
}

/**
 * Set the distance that this sound begins to fall off.  Also affects the rate
 * it falls off.
 */
void FMODAudioSound::
set_3d_min_distance(PN_stdfloat dist) {
  ReMutexHolder holder(FMODAudioManager::_lock);
  FMOD_RESULT result;

  _min_dist = dist;

  result = _sound->set3DMinMaxDistance(dist, _max_dist);
  fmod_audio_errcheck("_sound->set3DMinMaxDistance()", result);
}

/**
 * Get the distance that this sound begins to fall off
 */
PN_stdfloat FMODAudioSound::
get_3d_min_distance() const {
  return _min_dist;
}

/**
 * Set the distance that this sound stops falling off
 */
void FMODAudioSound::
set_3d_max_distance(PN_stdfloat dist) {
  ReMutexHolder holder(FMODAudioManager::_lock);
  FMOD_RESULT result;

  _max_dist = dist;

  result = _sound->set3DMinMaxDistance(_min_dist, dist);
  fmod_audio_errcheck("_sound->set3DMinMaxDistance()", result);
}

/**
 * Get the distance that this sound stops falling off
 */
PN_stdfloat FMODAudioSound::
get_3d_max_distance() const {
  return _max_dist;
}

/**
 * In Multichannel Speaker systems [like Surround].
 *
 * Speakers which don't exist in some systems will simply be ignored.  But I
 * haven't been able to test this yet, so I am jsut letting you know.
 *
 * BTW This will also work in Stereo speaker systems, but since PANDA/FMOD has
 * a balance [pan] function what is the point?
 */
PN_stdfloat FMODAudioSound::
get_speaker_mix(int speaker) {
  ReMutexHolder holder(FMODAudioManager::_lock);

  if (_channel == nullptr || speaker < 0 || speaker >= AudioManager::SPK_COUNT) {
    return 0.0;
  }

  int in, out;
  float mix[32][32];
  FMOD_RESULT result;
  // First query the number of output speakers and input channels
  result = _channel->getMixMatrix(nullptr, &out, &in, 32);
  if (CHANNEL_INVALID(result)) {
    _channel = nullptr;
    return 0.0;

  } else {
    fmod_audio_errcheck("_channel->getMixMatrix()", result);

    // Now get the actual mix matrix
    result = _channel->getMixMatrix((float *)mix, &out, &in, 32);
    fmod_audio_errcheck("_channel->getMixMatrix()", result);

    return mix[speaker][0];
  }
}

/**
 * Sets the mix value of a speaker.
 */
void FMODAudioSound::
set_speaker_mix(int speaker, PN_stdfloat mix) {
  nassertv(speaker >= 0 && speaker < AudioManager::SPK_COUNT);

  ReMutexHolder holder(FMODAudioManager::_lock);

  _mix[speaker] = mix;

  set_speaker_mix_or_balance_on_channel();
}

/**
 * Sets the mix values for all speakers.
 */
void FMODAudioSound::
set_speaker_mix(PN_stdfloat frontleft, PN_stdfloat frontright,
                PN_stdfloat center, PN_stdfloat sub,
                PN_stdfloat backleft, PN_stdfloat backright,
                PN_stdfloat sideleft, PN_stdfloat sideright) {
  ReMutexHolder holder(FMODAudioManager::_lock);

  _mix[AudioManager::SPK_front_left] = frontleft;
  _mix[AudioManager::SPK_front_right] = frontright;
  _mix[AudioManager::SPK_front_center] = center;
  _mix[AudioManager::SPK_sub] = sub;
  _mix[AudioManager::SPK_surround_left] = sideleft;
  _mix[AudioManager::SPK_surround_right] = sideright;
  _mix[AudioManager::SPK_back_left] = backleft;
  _mix[AudioManager::SPK_back_right] = backright;

  set_speaker_mix_or_balance_on_channel();
}

/**
 * This is simply a safety catch.  If you are using a Stero speaker setup
 * Panda will only pay attention to 'set_balance()' command when setting
 * speaker balances.  Other wise it will use 'set_speaker_mix'. I put this in,
 * because other wise you end up with a sitation, where 'set_speaker_mix()' or
 * 'set_balace()' will override any previous speaker balance setups.  It all
 * depends on which was called last.
 */
void FMODAudioSound::
set_speaker_mix_or_balance_on_channel() {
  ReMutexHolder holder(FMODAudioManager::_lock);
  FMOD_RESULT result;
  FMOD_MODE soundMode;

  result = _sound->getMode(&soundMode);
  fmod_audio_errcheck("_sound->getMode()", result);

  if ((_channel != nullptr) && (soundMode & FMOD_3D) == 0) {
    if (_speakermode == FMOD_SPEAKERMODE_STEREO) {
      result = _channel->setPan(_balance);
    } else {
      result = _channel->setMixLevelsOutput(
        _mix[AudioManager::SPK_front_left],
        _mix[AudioManager::SPK_front_right],
        _mix[AudioManager::SPK_front_center],
        _mix[AudioManager::SPK_sub],
        _mix[AudioManager::SPK_surround_left],
        _mix[AudioManager::SPK_surround_right],
        _mix[AudioManager::SPK_back_left],
        _mix[AudioManager::SPK_back_right]);
    }
    if (CHANNEL_INVALID(result)) {
      _channel = nullptr;
    } else {
      fmod_audio_errcheck("_channel->setSpeakerMix()/setPan()", result);
    }
  }
}

/**
 * Sets the priority of a sound.  This is what FMOD uses to determine is a
 * sound will play if all the other real channels have been used up.
 */
int FMODAudioSound::
get_priority() {
  audio_debug("FMODAudioSound::get_priority()");
  return _priority;
}

/**
 * Sets the Sound Priority [Whether is will be played over other sound when
 * real audio channels become short.
 */
void FMODAudioSound::
set_priority(int priority) {
  ReMutexHolder holder(FMODAudioManager::_lock);

  audio_debug("FMODAudioSound::set_priority()");

  FMOD_RESULT result;

  _priority = priority;

  result = _sound->setDefaults(_sample_frequency, _priority);
  fmod_audio_errcheck("_sound->setDefaults()", result);
}

/**
 * Get status of the sound.
 */
AudioSound::SoundStatus FMODAudioSound::
status() const {
  ReMutexHolder holder(FMODAudioManager::_lock);
  FMOD_RESULT result;
  bool playingState;

  if (_channel == nullptr) {
    return READY;
  }

  result = _channel->isPlaying(&playingState);
  if (CHANNEL_INVALID(result)) {
    ((FMODAudioSound *)this)->_channel = nullptr;
    return READY;
  } else if ((result == FMOD_OK) && (playingState == true)) {
    return PLAYING;
  } else {
    return READY;
  }
}

/**
 * Sets whether the sound is marked "active".  By default, the active flag
 * true for all sounds.  If the active flag is set to false for any particular
 * sound, the sound will not be heard.
 */
void FMODAudioSound::
set_active(bool active) {
  ReMutexHolder holder(FMODAudioManager::_lock);
  if (_active != active) {
    _active = active;
    if (_active) {
      // ...activate the sound.
      if (_paused && get_loop_count()==0) {
        // ...this sound was looping when it was paused.
        _paused = false;
        play();
      }

    } else {
      // ...deactivate the sound.
      if (status() == PLAYING) {
        PN_stdfloat time = get_time();
        stop();
        if (get_loop_count() == 0) {
          // ...we're pausing a looping sound.
          _paused = true;
          _start_time = time;
        }
      }
    }
  }
}

/**
 * Returns whether the sound has been marked "active".
 */
bool FMODAudioSound::
get_active() const {
  return _active;
}

/**
 * Inserts the specified DSP filter into the DSP chain at the specified index.
 * Returns true if the DSP filter is supported by the audio implementation,
 * false otherwise.
 */
bool FMODAudioSound::
insert_dsp(int index, DSP *panda_dsp) {
  ReMutexHolder holder(FMODAudioManager::_lock);

  // If it's already in there, take it out and put it in the new spot.
  remove_dsp(panda_dsp);

  FMOD::DSP *dsp = _manager->create_fmod_dsp(panda_dsp);
  if (!dsp) {
    fmodAudio_cat.warning()
      << panda_dsp->get_type().get_name()
      << " unsupported by FMOD audio implementation.\n";
    return false;
  }

  // Keep track of our DSPs.
  _dsps.push_back(dsp);

  set_dsps_on_channel();

  return true;
}

/**
 * Removes the specified DSP filter from the DSP chain. Returns true if the
 * filter was in the DSP chain and was removed, false otherwise.
 */
bool FMODAudioSound::
remove_dsp(DSP *panda_dsp) {
  ReMutexHolder holder(FMODAudioManager::_lock);

  FMOD_RESULT ret;
  FMOD::DSP *dsp = nullptr;
  int index = -1;
  for (size_t i = 0; i < _dsps.size(); i++) {
    DSP *panda_dsp_check = nullptr;
    ret = _dsps[i]->getUserData((void **)&panda_dsp_check);
    if (ret == FMOD_OK && panda_dsp_check == panda_dsp) {
      dsp = _dsps[i];
      index = i;
      break;
    }
  }

  if (dsp == nullptr || index == -1) {
    return false;
  }

  ret = dsp->release();
  fmod_audio_errcheck("dsp->release()", ret);

  _dsps.erase(_dsps.begin() + index);

  set_dsps_on_channel();

  return true;
}

/**
 * Removes all DSP filters from the DSP chain.
 */
void FMODAudioSound::
remove_all_dsps() {
  ReMutexHolder holder(FMODAudioManager::_lock);

  FMOD_RESULT ret;

  for (size_t i = 0; i < _dsps.size(); i++) {
    if (_dsps[i] == nullptr) {
      continue;
    }

    ret = _dsps[i]->release();
    fmod_audio_errcheck("_dsps[i]->release()", ret);
  }

  _dsps.clear();

  set_dsps_on_channel();
}

/**
 * Returns the number of DSP filters present in the DSP chain.
 */
int FMODAudioSound::
get_num_dsps() const {
  // Can't use _channel->getNumDSPs() because that includes DSPs that are
  // created internally by FMOD.  We want to return the number of user-created
  // DSPs.

  return (int)_dsps.size();
}

/**
 *
 */
void FMODAudioSound::
set_dsps_on_channel() {
  ReMutexHolder holder(FMODAudioManager::_lock);

  if (_channel == nullptr) {
    return;
  }

  FMOD_RESULT ret;

  // First clear out existing ones.
  int num_chan_dsps = 0;
  ret = _channel->getNumDSPs(&num_chan_dsps);
  if (CHANNEL_INVALID(ret)) {
    _channel = nullptr;
    return;
  } else {
    fmod_audio_errcheck("_channel->getNumDSPs()", ret);
  }

  for (int i = num_chan_dsps - 1; i >= 0; i--) {
    FMOD::DSP *dsp = nullptr;
    ret = _channel->getDSP(i, &dsp);
    fmod_audio_errcheck("_channel->getDSP()", ret);

    if (ret != FMOD_OK || dsp == nullptr) {
      continue;
    }

    void *user_data = nullptr;
    ret = dsp->getUserData(&user_data);
    if (ret == FMOD_OK && user_data != nullptr) {
      // This is a DSP created by us, remove it.
      ret = _channel->removeDSP(dsp);
      fmod_audio_errcheck("_channel->removeDSP()", ret);
    }
  }

  // Now add ours in.
  for (int i = 0; i < (int)_dsps.size(); i++) {
    FMOD::DSP *dsp = _dsps[i];
    if (dsp == nullptr) {
      continue;
    }

    // Make sure the FMOD DSP is synchronized with the Panda descriptor.
    DSP *panda_dsp = nullptr;
    ret = dsp->getUserData((void **)&panda_dsp);
    fmod_audio_errcheck("dsp->getUserData()", ret);
    if (ret == FMOD_OK && panda_dsp != nullptr && panda_dsp->is_dirty()) {
      _manager->configure_dsp(panda_dsp, dsp);
      panda_dsp->clear_dirty();
    }

    ret = _channel->addDSP(i, dsp);
    fmod_audio_errcheck("_channel->addDSP()", ret);
  }
}

/**
 *
 */
void FMODAudioSound::
update() {
  // Update any DSPs that are dirty.
  FMOD_RESULT ret;
  int current_frame = ClockObject::get_global_clock()->get_frame_count();
  if (current_frame != _last_update_frame) {
    for (FMODDSPs::const_iterator it = _dsps.begin(); it != _dsps.end(); ++it) {
      FMOD::DSP *dsp = *it;
      if (dsp == nullptr) {
        continue;
      }
      DSP *panda_dsp = nullptr;
      ret = dsp->getUserData((void **)&panda_dsp);
      fmod_audio_errcheck("dsp->getUserData()", ret);
      if (ret == FMOD_OK && panda_dsp != nullptr && panda_dsp->is_dirty()) {
        _manager->configure_dsp(panda_dsp, dsp);
        panda_dsp->clear_dirty();
      }
    }
    _last_update_frame = current_frame;
  }
}

/**
 * Not implemented.
 */
void FMODAudioSound::
finished() {
  ReMutexHolder holder(FMODAudioManager::_lock);

  if (!get_finished_event().empty()) {
    throw_event(get_finished_event(), EventParameter(this));
  }

  stop();
}

/**
 * Assign a string for the finished event to be referenced
 * by in python by an accept method.
 *
 */
void FMODAudioSound::
set_finished_event(const std::string& event) {
  _finished_event = event;
}

/**
 * Return the string the finished event is referenced by.
 */
const std::string& FMODAudioSound::
get_finished_event() const {
  return _finished_event;
}

/**
 * Returns the FMODSoundHandle that the sound is referencing.
 */
FMODSoundHandle *FMODAudioSound::
get_sound_handle() const {
  return _sound_handle;
}
