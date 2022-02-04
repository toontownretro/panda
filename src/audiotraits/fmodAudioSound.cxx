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
#include "mathutil_misc.h"

TypeHandle FMODAudioSound::_type_handle;

#ifndef HAMMER_UNITS_TO_METERS
#define HAMMER_UNITS_TO_METERS 0.01905f
#endif

#define CHANNEL_INVALID(result) (result == FMOD_ERR_INVALID_HANDLE || result == FMOD_ERR_CHANNEL_STOLEN)

#ifdef HAVE_STEAM_AUDIO

// calculate gain based on atmospheric attenuation.
// as gain excedes threshold, round off (compress) towards 1.0 using spline
#define SND_GAIN_COMP_EXP_MAX	2.5f	// Increasing SND_GAIN_COMP_EXP_MAX fits compression curve more closely
                    // to original gain curve as it approaches 1.0.
#define SND_GAIN_COMP_EXP_MIN	0.8f

#define SND_GAIN_COMP_THRESH	0.5f		// gain value above which gain curve is rounded to approach 1.0

#define SND_DB_MAX				140.0f	// max db of any sound source
#define SND_DB_MED				90.0f	// db at which compression curve changes

#define SND_GAIN_MAX 1
#define SND_GAIN_MIN 0.01

#define SND_REFDB 60.0
#define SND_REFDIST 36.0

#define SNDLVL_TO_DIST_MULT( sndlvl ) ( sndlvl ? ((pow( 10.0f, SND_REFDB / 20 ) / pow( 10.0f, (float)sndlvl / 20 )) / SND_REFDIST) : 0 )
#define DIST_MULT_TO_SNDLVL( dist_mult ) (int)( dist_mult ? ( 20 * log10( pow( 10.0f, SND_REFDB / 20 ) / (dist_mult * SND_REFDIST) ) ) : 0 )

/**
 * Implements a custom distance attenuation for Steam Audio.
 * Scales the distance by the sound's distance multiplier.
 */
float
ipl_distance_atten(IPLfloat32 distance, void *user_data) {
  FMODAudioSound *sound = (FMODAudioSound *)user_data;

  // Meters to hammer units.
  distance /= HAMMER_UNITS_TO_METERS;

  PN_stdfloat gain = 1.0f;
  PN_stdfloat dist_factor = sound->get_3d_distance_factor();
  PN_stdfloat relative_dist = distance * dist_factor;
  if (relative_dist > 0.1f) {
    gain *= (1.0f / relative_dist);
  } else {
    gain *= 10.0f;
  }

  if (gain > SND_GAIN_COMP_THRESH) {
    PN_stdfloat snd_gain_comp_power = SND_GAIN_COMP_EXP_MAX;
    int sndlvl = DIST_MULT_TO_SNDLVL(dist_factor);
    PN_stdfloat y;

    if (sndlvl > SND_DB_MED) {
      snd_gain_comp_power = remap_val_clamped((float)sndlvl, SND_DB_MED, SND_DB_MAX, SND_GAIN_COMP_EXP_MAX, SND_GAIN_COMP_EXP_MIN);
    }

    y = -1.0f / (pow(SND_GAIN_COMP_THRESH, snd_gain_comp_power) * (SND_GAIN_COMP_THRESH - 1.0f));
    gain = 1.0f - 1.0f / (y * pow(gain, snd_gain_comp_power));
    gain *= SND_GAIN_MAX;
  }

  if (gain < SND_GAIN_MIN) {
    gain = SND_GAIN_MIN * (2.0f - relative_dist * SND_GAIN_MIN);

    if (gain <= 0.0f) {
      gain = 0.001f;
    }
  }

  return gain;
}

IPLVector3
fmod_vec_to_ipl(const FMOD_VECTOR &vec) {
  return IPLVector3{ vec.x, vec.y, -vec.z };
}

IPLVector3
ipl_cross(const IPLVector3& a, const IPLVector3& b) {
  IPLVector3 c;
  c.x = a.y * b.z - a.z * b.y;
  c.y = a.z * b.x - a.x * b.z;
  c.z = a.x * b.y - a.y * b.x;
  return c;
}

IPLVector3
ipl_unit_vector(IPLVector3 v) {
  float length = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
  if (length < 1e-2f)
      length = 1e-2f;

  return IPLVector3{v.x / length, v.y / length, v.z / length};
}


IPLCoordinateSpace3 fmod_coordinates_to_ipl(const FMOD_VECTOR &origin, const FMOD_VECTOR &forward, const FMOD_VECTOR &up) {
  IPLCoordinateSpace3 coords;
  coords.ahead = fmod_vec_to_ipl(forward);
  coords.up = fmod_vec_to_ipl(up);
  coords.origin = fmod_vec_to_ipl(origin);
  coords.right = ipl_unit_vector(ipl_cross(coords.ahead, coords.up));
  return coords;
}
#endif

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

  _up.x = 0;
  _up.y = 1;
  _up.z = 0;

  _forward.x = 0;
  _forward.y = 0;
  _forward.z = 1;

  _min_dist = 1.0;
  _max_dist = 1000000000.0;
  _dist_factor = 1.0f;

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

#ifdef HAVE_STEAM_AUDIO
  _sa_source = nullptr;
  _sa_spatial_dsp = nullptr;
#endif
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

  _up.x = 0;
  _up.y = 1;
  _up.z = 0;

  _forward.x = 0;
  _forward.y = 0;
  _forward.z = 1;

  _min_dist = copy->_min_dist;
  _max_dist = copy->_max_dist;
  _dist_factor = copy->_dist_factor;

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

#ifdef HAVE_STEAM_AUDIO
  _sa_source = nullptr;
  _sa_spatial_dsp = nullptr;
#endif
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

#ifdef HAVE_STEAM_AUDIO
  if (_sa_spatial_dsp != nullptr) {
    result = _sa_spatial_dsp->release();
    fmod_audio_errcheck("release Steam Audio spatializer DSP", result);
    _sa_spatial_dsp = nullptr;
  }
  if (_sa_source != nullptr) {
    iplSourceRemove(_sa_source, _manager->_sa_simulator);
    ++_manager->_next_sim_update;
    iplSourceRelease(&_sa_source);
    _sa_source = nullptr;
  }
#endif

  _manager->release_sound(this);
}

/**
 * Plays a sound.
 */
void FMODAudioSound::
play() {
  //start_playing();
  _manager->_queued_plays.insert(this);
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

#ifdef HAVE_STEAM_AUDIO
  //if (_sa_source != nullptr) {
  //  iplSourceRemove(_sa_source, _manager->_sa_simulator);
   // iplSimulatorCommit(_manager->_sa_simulator);
  //}
#endif

  _start_time = 0.0;
  _paused = false;

  _manager->_queued_plays.erase(this);

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
    //start_playing();
    _manager->_queued_plays.insert(this);
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
    set_dsps_on_channel();
    set_3d_attributes_on_channel();

#ifdef HAVE_STEAM_AUDIO
    if (_sa_source != nullptr) {
      iplSourceAdd(_sa_source, _manager->_sa_simulator);
      ++_manager->_next_sim_update;
    }
#endif

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
set_3d_attributes(PN_stdfloat px, PN_stdfloat py, PN_stdfloat pz, PN_stdfloat vx, PN_stdfloat vy, PN_stdfloat vz,
                  PN_stdfloat fx, PN_stdfloat fy, PN_stdfloat fz, PN_stdfloat ux, PN_stdfloat uy, PN_stdfloat uz) {
  ReMutexHolder holder(FMODAudioManager::_lock);

  // Inches to meters.
  _location.x = px * HAMMER_UNITS_TO_METERS;
  _location.y = pz * HAMMER_UNITS_TO_METERS;
  _location.z = py * HAMMER_UNITS_TO_METERS;

  _velocity.x = vx * HAMMER_UNITS_TO_METERS;
  _velocity.y = vz * HAMMER_UNITS_TO_METERS;
  _velocity.z = vy * HAMMER_UNITS_TO_METERS;

  _up.x = ux;
  _up.y = uz;
  _up.z = uy;

  _forward.x = fx;
  _forward.y = fz;
  _forward.z = fy;

#ifdef HAVE_STEAM_AUDIO
  if (_sa_source != nullptr) {
    // If the source is simulated, apply the new transform to the Steam Audio
    // source as well.
    _sa_inputs.source = fmod_coordinates_to_ipl(_location, _forward, _up);
    {
      ReMutexHolder sa_holder(FMODAudioManager::_sa_refl_lock);
      iplSourceSetInputs(_sa_source, _sa_inputs.flags, &_sa_inputs);
    }
  }
#endif

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

#ifdef HAVE_STEAM_AUDIO
  bool steam_audio_dsp = fmod_use_steam_audio && _sa_spatial_dsp != nullptr;
#else
  bool steam_audio_dsp = false;
#endif

  if (steam_audio_dsp) {
    // With Steam Audio the 3D attributes are set on the Steam Audio
    // spatializer DSP, which replaces the built-in FMOD positional audio.
    FMOD_DSP_PARAMETER_3DATTRIBUTES attr;
    attr.absolute.position = _location;
    attr.absolute.velocity = _velocity;
    attr.absolute.up = _up;
    attr.absolute.forward = _forward;
    // Steam Audio doesn't care about the relative 3D attributes.
    _sa_spatial_dsp->setParameterData(0, &attr, sizeof(attr));

  } else if ((_channel != nullptr) && (soundMode & FMOD_3D) != 0) {
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

  _min_dist = dist * HAMMER_UNITS_TO_METERS;

#ifdef HAVE_STEAM_AUDIO
  bool steam_audio_dsp = fmod_use_steam_audio && _sa_spatial_dsp != nullptr;
#else
  bool steam_audio_dsp = false;
#endif

  if (steam_audio_dsp) {
    _sa_spatial_dsp->setParameterFloat(12, _min_dist);

  } else {
    result = _sound->set3DMinMaxDistance(_min_dist, _max_dist);
    fmod_audio_errcheck("_sound->set3DMinMaxDistance()", result);
  }
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

  _max_dist = dist * HAMMER_UNITS_TO_METERS;

#ifdef HAVE_STEAM_AUDIO
  bool steam_audio_dsp = fmod_use_steam_audio && _sa_spatial_dsp != nullptr;
#else
  bool steam_audio_dsp = false;
#endif

  if (steam_audio_dsp) {
    _sa_spatial_dsp->setParameterFloat(13, _max_dist);

  } else {
    result = _sound->set3DMinMaxDistance(_min_dist, _max_dist);
    fmod_audio_errcheck("_sound->set3DMinMaxDistance()", result);
  }
}

/**
 * Get the distance that this sound stops falling off
 */
PN_stdfloat FMODAudioSound::
get_3d_max_distance() const {
  return _max_dist;
}

/**
 *
 */
void FMODAudioSound::
set_3d_distance_factor(PN_stdfloat factor) {
  _dist_factor = factor;
#ifdef HAVE_STEAM_AUDIO
  if (fmod_use_steam_audio && _sa_spatial_dsp != nullptr) {
    _sa_spatial_dsp->setParameterFloat(31, _dist_factor);
  }
#endif
}

/**
 *
 */
PN_stdfloat FMODAudioSound::
get_3d_distance_factor() const {
  return _dist_factor;
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

#ifdef HAVE_STEAM_AUDIO
  if (fmod_use_steam_audio) {
    if (_sa_spatial_dsp != nullptr) {
      // Index 1 to spatialize before the channel fader.
      ret = _channel->addDSP(FMOD_CHANNELCONTROL_DSP_TAIL, _sa_spatial_dsp);
      fmod_audio_errcheck("add SA spatial DSP", ret);
    }

    // Add the head (final processed DSP node) as an input to the Steam Audio reverb.
    FMOD::DSP *head;
    _channel->getDSP(FMOD_CHANNELCONTROL_DSP_HEAD, &head);
    _manager->_reverb_dsp->addInput(head, 0, FMOD_DSPCONNECTION_TYPE_STANDARD);
  }

#endif
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

/**
 * Configures the sound to be a Steam Audio source.
 * Can only be set up and configured once.  Currently you cannot change
 * any Steam Audio properties on the fly (except simple stuff like sound
 * position).
 */
void FMODAudioSound::
apply_steam_audio_properties(const SteamAudioProperties &props) {
#ifdef HAVE_STEAM_AUDIO
  if (!fmod_use_steam_audio || _sa_source != nullptr || _sa_spatial_dsp != nullptr) {
    // Already a Steam Audio source.
    return;
  }

  unsigned int flags = 0u;
  if (props._enable_occlusion || props._enable_transmission) {
    flags |= IPL_SIMULATIONFLAGS_DIRECT;
  }
  if (props._enable_reflections) {
    flags |= IPL_SIMULATIONFLAGS_REFLECTIONS;
  }
  if (props._enable_pathing) {
    flags |= IPL_SIMULATIONFLAGS_PATHING;
  }

  if (flags != 0u) {
    // Something on the sound actually needs simulation, so we need to create
    // an IPLSource and add it to our simulator.
    IPLSourceSettings src_settings;
    src_settings.flags = (IPLSimulationFlags)flags;
    IPLerror err = iplSourceCreate(_manager->_sa_simulator, &src_settings, &_sa_source);
    nassertv(err == IPL_STATUS_SUCCESS && _sa_source != nullptr);
    // Not going to add it until it starts playing.

    memset(&_sa_inputs, 0, sizeof(IPLSimulationInputs));

    _sa_inputs.flags = src_settings.flags;

    // We need a custom distance attenuation model to support the soundlevel
    // distance multiplier system.
    _sa_inputs.distanceAttenuationModel.type = IPL_DISTANCEATTENUATIONTYPE_CALLBACK;
    _sa_inputs.distanceAttenuationModel.callback = ipl_distance_atten;
    _sa_inputs.distanceAttenuationModel.userData = this;

    unsigned int direct_flags = 0u;
    if (props._enable_occlusion) {
      direct_flags |= IPL_DIRECTSIMULATIONFLAGS_OCCLUSION;
      if (props._enable_transmission) {
        direct_flags |= IPL_DIRECTSIMULATIONFLAGS_TRANSMISSION;
      }

      if (props._volumetric_occlusion) {
        _sa_inputs.occlusionType = IPL_OCCLUSIONTYPE_VOLUMETRIC;
      } else {
        _sa_inputs.occlusionType = IPL_OCCLUSIONTYPE_RAYCAST;
      }
      _sa_inputs.occlusionRadius = props._volumetric_occlusion_radius;
      _sa_inputs.numOcclusionSamples = 32;
    }
    _sa_inputs.directFlags = (IPLDirectSimulationFlags)direct_flags;

    if (props._enable_pathing) {
      _sa_inputs.baked = IPL_TRUE;
      _sa_inputs.bakedDataIdentifier.type = IPL_BAKEDDATATYPE_PATHING;
      _sa_inputs.bakedDataIdentifier.variation = IPL_BAKEDDATAVARIATION_DYNAMIC;
      _sa_inputs.pathingOrder = 1;
      _sa_inputs.pathingProbes = _manager->_sa_probe_batch;
      _sa_inputs.findAlternatePaths = IPL_FALSE;
    }

    _sa_inputs.source = fmod_coordinates_to_ipl(_location, _forward, _up);

    {
      ReMutexHolder holder(FMODAudioManager::_sa_refl_lock);
      iplSourceSetInputs(_sa_source, _sa_inputs.flags, &_sa_inputs);
    }
  }

  // Create and configure the spatialization DSP filter.
  FMOD_RESULT result = _manager->_system->createDSPByPlugin(_manager->_sa_spatialize_handle, &_sa_spatial_dsp);
  if (result != FMOD_OK) {
    fmod_audio_errcheck("create Steam Audio spatializer DSP", result);
    return;
  }

  _sa_spatial_dsp->setUserData(this);

  FMOD_DSP_PARAMETER_3DATTRIBUTES attr;
  attr.absolute.position = _location;
  attr.absolute.velocity = _velocity;
  attr.absolute.up = _up;
  attr.absolute.forward = _forward;
  // Steam Audio doesn't care about the relative 3D attributes.
  _sa_spatial_dsp->setParameterData(0, &attr, sizeof(attr)); // SOURCE_POSITION
  _sa_spatial_dsp->setParameterInt(2, props._enable_distance_atten ? 2 : 0);
  _sa_spatial_dsp->setParameterInt(3, props._enable_air_absorption ? 1 : 0);
  _sa_spatial_dsp->setParameterInt(4, props._enable_directivity ? 1 : 0);
  _sa_spatial_dsp->setParameterInt(5, props._enable_occlusion ? 1 : 0);
  _sa_spatial_dsp->setParameterInt(6, props._enable_transmission ? 1 : 0);
  _sa_spatial_dsp->setParameterBool(7, props._enable_reflections);
  _sa_spatial_dsp->setParameterBool(8, props._enable_pathing);
  _sa_spatial_dsp->setParameterInt(9, props._bilinear_hrtf ? 1 : 0);
  _sa_spatial_dsp->setParameterFloat(12, _min_dist); // DISTANCEATTEN_MINDIST
  _sa_spatial_dsp->setParameterFloat(13, _max_dist); // DISTANCEATTEN_MAXDIST
  _sa_spatial_dsp->setParameterFloat(18, props._directivity_dipole_weight);
  _sa_spatial_dsp->setParameterFloat(19, props._directivity_dipole_power);
  _sa_spatial_dsp->setParameterFloat(25, 1.0f); // DIRECT_MIXLEVEL
  _sa_spatial_dsp->setParameterBool(26, props._binaural_reflections);
  _sa_spatial_dsp->setParameterFloat(27, 1.0f); // REFLECTIONS_MIXLEVEL
  _sa_spatial_dsp->setParameterBool(28, props._binaural_pathing);
  _sa_spatial_dsp->setParameterFloat(29, 1.0f); // PATHING_MIXLEVEL
  // Link back to the IPLSource so the DSP can render simulation results.
  _sa_spatial_dsp->setParameterData(30, &_sa_source, sizeof(&_sa_source)); // SIMULATION_OUTPUTS
  _sa_spatial_dsp->setParameterFloat(31, _dist_factor); // DISTANCEATTEN_DISTANCEFACTOR
#endif
}
