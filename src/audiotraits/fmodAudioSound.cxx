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
#include "fmodAudioEngine.h"

TypeHandle FMODAudioSound::_type_handle;

#define CHANNEL_INVALID(result) (result == FMOD_ERR_INVALID_HANDLE || result == FMOD_ERR_CHANNEL_STOLEN)

#ifdef HAVE_STEAM_AUDIO

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

extern FMOD_VECTOR lvec_to_fmod(const LVecBase3 &vec);

/**
 * Constructor All sound will DEFAULT load as a 2D sound unless otherwise
 * specified.
 */
FMODAudioSound::
FMODAudioSound(FMODAudioManager *manager, FMODSoundHandle *handle) :
  _manager(manager),
  _sound_handle(handle),
  _sound(handle->get_sound()),
  _paused(false),
  _active(manager->get_active()),
  _balance(0.0f),
  _start_time(0.0f),
  _volume(1.0f),
  _playrate(1.0f),
  _is_midi(false),
  _length(0),
  _channel(nullptr),
  _pos(0.0f),
  _vel(0.0f),
  _quat(LQuaternion::ident_quat()),
  _min_dist(1.0f),
  _file_name(handle->get_orig_filename()),
  _sa_spatial_dsp(nullptr)
{
  ReMutexHolder holder(FMODAudioManager::_lock);
  audio_debug("FMODAudioSound::FMODAudioSound() Creating new sound from handle: "
              << handle->get_orig_filename());

  FMOD_RESULT result;

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

  // By default, the loop range is the entire sound.  The user can constrain
  // this range later.  We need to remember the loop range for fixing up MIDIs
  // with faster or slower play rates.
  _loop_start = 0;
  _loop_end = _length;

  _sa_spatial_dsp = nullptr;
}

/**
 * Initializes an FMODAudioSound from an existing FMODAudioSound.  Shares the
 * sound data but creates a new channel.
 */
FMODAudioSound::
FMODAudioSound(FMODAudioManager *manager, FMODAudioSound *copy) {
  ReMutexHolder holder(FMODAudioManager::_lock);
  audio_debug("FMODAudioSound::FMODAudioSound() Creating channel from existing "
              "sound handle");

  _manager = manager;
  _active = manager->get_active();
  _paused = false;
  _start_time = 0.0;
  _balance = copy->_balance;
  _volume = copy->_volume;
  _playrate = copy->_playrate;
  _is_midi = copy->_is_midi;
  _length = copy->_length;
  _loop_start = copy->_loop_start;
  _loop_end = copy->_loop_end;
  _pos.set(0.0f, 0.0f, 0.0f);
  _vel.set(0.0f, 0.0f, 0.0f);
  _quat = LQuaternion::ident_quat();

  _min_dist = copy->_min_dist;

  _dsps = copy->_dsps;

  FMOD_RESULT result;

  // Assign the values we need
  FMODAudioManager *fmanager;
  DCAST_INTO_V(fmanager, manager);
  _manager = fmanager;

  _channel = nullptr;
  _file_name = copy->_file_name;

  _sound_handle = copy->_sound_handle;
  nassertv(_sound_handle != nullptr);

  _sound = _sound_handle->get_sound();
  nassertv(_sound != nullptr);

  _sample_frequency = copy->_sample_frequency;
  _priority = copy->_priority;

  _sa_spatial_dsp = nullptr;
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

  if (_sa_spatial_dsp != nullptr) {
    result = _sa_spatial_dsp->release();
    fmod_audio_errcheck("release Steam Audio spatializer DSP", result);
    _sa_spatial_dsp = nullptr;
  }

  _manager->release_sound(this);
}

/**
 * Plays a sound.
 */
void FMODAudioSound::
play() {
  start_playing();
  //_manager->_queued_plays.insert(this);
}

/**
 * Stop a sound
 */
void FMODAudioSound::
stop() {
  ReMutexHolder holder(FMODAudioManager::_lock);

  if (!get_finished_event().empty()) {
    throw_event(get_finished_event(), EventParameter(this));
  }

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

  //_manager->_queued_plays.erase(this);

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
    //_manager->_queued_plays.insert(this);
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
    FMOD::System *sys = _manager->_engine->get_system();
    result = sys->playSound(_sound, _manager->_channelgroup, true, &_channel);
    fmod_audio_errcheck("playSound()", result);
    nassertv(_channel != nullptr);

    result = _channel->setPosition(start_time_ms, FMOD_TIMEUNIT_MS);
    fmod_audio_errcheck("_channel->setPosition()", result);

    set_volume_on_channel();
    set_play_rate_on_channel();
    set_speaker_mix_or_balance_on_channel();
    set_dsps_on_channel();
    set_3d_attributes_on_channel();

    result = _channel->setPaused(false);
    fmod_audio_errcheck("_channel->setPaused()", result);
  }

  bool playing = false;
  result = _channel->isPlaying(&playing);
  fmod_audio_errcheck("_channel->isPlaying()", result);

  _start_time = 0.0;

  //nassertv(playing);
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
    result = _sound->setLoopPoints((unsigned int)((float)_loop_start / _playrate), FMOD_TIMEUNIT_MS,
                                   (unsigned int)((float)_loop_end / _playrate), FMOD_TIMEUNIT_MS);
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
set_3d_attributes(const LPoint3 &pos, const LQuaternion &quat, const LVector3 &vel) {
  ReMutexHolder holder(FMODAudioManager::_lock);

  FMODAudioEngine *engine = _manager->_engine;
  PN_stdfloat unit_scale = engine->get_3d_unit_scale();

  // Game units to meters.
  _pos = pos / unit_scale;
  _vel = vel / unit_scale;
  _quat = quat;

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
    attr.absolute.position = lvec_to_fmod(_pos);
    attr.absolute.velocity = lvec_to_fmod(_vel);
    LVector3 up, fwd;
    up = _quat.get_up();
    fwd = _quat.get_forward();
    attr.absolute.up = lvec_to_fmod(up);
    attr.absolute.forward = lvec_to_fmod(fwd);
    // Steam Audio doesn't care about the relative 3D attributes.
    _sa_spatial_dsp->setParameterData(0, &attr, sizeof(attr));

  } else if ((_channel != nullptr) && (soundMode & FMOD_3D) != 0) {
    FMOD_VECTOR pos, vel;
    pos = lvec_to_fmod(_pos);
    vel = lvec_to_fmod(_vel);
    result = _channel->set3DAttributes(&pos, &vel);
    if (CHANNEL_INVALID(result)) {
      _channel = nullptr;
    } else {
      fmod_audio_errcheck("_channel->set3DAttributes()", result);
    }
  }
}

/**
 *
 */
LPoint3 FMODAudioSound::
get_3d_position() const {
  return _pos * _manager->_engine->get_3d_unit_scale();
}

/**
 *
 */
LQuaternion FMODAudioSound::
get_3d_quat() const {
  return _quat;
}

/**
 *
 */
LVector3 FMODAudioSound::
get_3d_velocity() const {
  return _vel * _manager->_engine->get_3d_unit_scale();
}

/**
 * Set the distance that this sound begins to fall off.  Also affects the rate
 * it falls off.
 */
void FMODAudioSound::
set_3d_min_distance(PN_stdfloat dist) {
  ReMutexHolder holder(FMODAudioManager::_lock);
  FMOD_RESULT result;

  _min_dist = dist / _manager->_engine->get_3d_unit_scale();

#ifdef HAVE_STEAM_AUDIO
  bool steam_audio_dsp = fmod_use_steam_audio && _sa_spatial_dsp != nullptr;
#else
  bool steam_audio_dsp = false;
#endif

  if (steam_audio_dsp) {
    _sa_spatial_dsp->setParameterFloat(12, _min_dist);

  } else {
    result = _sound->set3DMinMaxDistance(_min_dist, 100000000.0f);
    fmod_audio_errcheck("_sound->set3DMinMaxDistance()", result);
  }
}

/**
 * Get the distance that this sound begins to fall off
 */
PN_stdfloat FMODAudioSound::
get_3d_min_distance() const {
  return _min_dist * _manager->_engine->get_3d_unit_scale();
}

/**
 * Returns the base frequency/sample rate of the audio file.
 */
PN_stdfloat FMODAudioSound::
get_sound_frequency() const {
  return _sample_frequency;
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
  //ReMutexHolder holder(FMODAudioManager::_lock);
  //FMOD_RESULT result;
  //FMOD_MODE soundMode;

  //result = _sound->getMode(&soundMode);
  //fmod_audio_errcheck("_sound->getMode()", result);

  //if ((_channel != nullptr) && ((soundMode & FMOD_3D) == 0) && (_sa_spatial_dsp == nullptr)) {
  //  result = _channel->setPan(_balance);
  //  if (CHANNEL_INVALID(result)) {
  //    _channel = nullptr;
  //  } else {
  //    fmod_audio_errcheck("_channel->setSpeakerMix()/setPan()", result);
  //  }
  //}
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

  FMOD::DSP *dsp = _manager->_engine->create_fmod_dsp(panda_dsp);
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
      _manager->_engine->configure_dsp(panda_dsp, dsp);
      panda_dsp->clear_dirty();
    }

    ret = _channel->addDSP(i, dsp);
    fmod_audio_errcheck("_channel->addDSP()", ret);
  }

#if 0//def HAVE_STEAM_AUDIO
  if (fmod_use_steam_audio) {
    if (_sa_spatial_dsp != nullptr) {
      // Index 1 to spatialize before the channel fader.
      ret = _channel->addDSP(FMOD_CHANNELCONTROL_DSP_TAIL, _sa_spatial_dsp);
      fmod_audio_errcheck("add SA spatial DSP", ret);
    }

    // Add the head (final processed DSP node) as an input to the Steam Audio reverb.
    FMOD::DSP *head;
    _channel->getDSP(FMOD_CHANNELCONTROL_DSP_HEAD, &head);
    _manager->_reverb_dsp->addInput(head, 0, FMOD_DSPCONNECTION_TYPE_SEND);
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
  for (FMODDSPs::const_iterator it = _dsps.begin(); it != _dsps.end(); ++it) {
    FMOD::DSP *dsp = *it;
    if (dsp == nullptr) {
      continue;
    }
    DSP *panda_dsp = nullptr;
    ret = dsp->getUserData((void **)&panda_dsp);
    fmod_audio_errcheck("dsp->getUserData()", ret);
    if (ret == FMOD_OK && panda_dsp != nullptr && panda_dsp->is_dirty()) {
      _manager->_engine->configure_dsp(panda_dsp, dsp);
      panda_dsp->clear_dirty();
    }
  }
}

/**
 * Not implemented.
 */
void FMODAudioSound::
finished() {
  ReMutexHolder holder(FMODAudioManager::_lock);

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
#if 0//def HAVE_STEAM_AUDIO
  if (!fmod_use_steam_audio || _sa_spatial_dsp != nullptr) {
    // Already a Steam Audio source.
    return;
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
  _sa_spatial_dsp->setActive(true);
  _sa_spatial_dsp->setParameterData(0, &attr, sizeof(attr)); // SOURCE_POSITION
  _sa_spatial_dsp->setParameterInt(2, props._enable_distance_atten ? 1 : 0);
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
  _sa_spatial_dsp->setParameterFloat(20, 1.0f); // OCCLUSION
  _sa_spatial_dsp->setParameterFloat(25, 1.0f); // DIRECT_MIXLEVEL
  _sa_spatial_dsp->setParameterBool(26, props._binaural_reflections);
  _sa_spatial_dsp->setParameterFloat(27, 1.0f); // REFLECTIONS_MIXLEVEL
  _sa_spatial_dsp->setParameterBool(28, props._binaural_pathing);
  _sa_spatial_dsp->setParameterFloat(29, 1.0f); // PATHING_MIXLEVEL
  // Link back to the IPLSource so the DSP can render simulation results.
  //_sa_spatial_dsp->setParameterData(30, &_sa_source, sizeof(&_sa_source)); // SIMULATION_OUTPUTS

  //if (props._enable_occlusion) {
  //  bool calculated;
  //  float gain = _manager->calc_sound_occlusion(this, calculated);
  //  if (calculated) {
  //    _sa_spatial_dsp->setParameterFloat(20, gain);
  //  }
  //}
#endif
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
void FMODAudioSound::
set_loop_range(PN_stdfloat start, PN_stdfloat end) {
  nassertv(_sound != nullptr);

  PN_stdfloat length_sec = _length / 1000.0f;

  nassertv(start <= length_sec);
  if (end < 0.0f || end <= start) {
    end = length_sec;
  }

  _loop_start = (unsigned int)(start * 1000.0f);
  _loop_end = (unsigned int)(end * 1000.0f);

  FMOD_RESULT hr;
  if (_is_midi) {
    // MIDIs need to factor in current music speed.
    hr = _sound->setLoopPoints((unsigned int)((float)_loop_start / _playrate), FMOD_TIMEUNIT_MS,
                               (unsigned int)((float)_loop_end / _playrate), FMOD_TIMEUNIT_MS);
  } else {
    hr = _sound->setLoopPoints(_loop_start, FMOD_TIMEUNIT_MS, _loop_end, FMOD_TIMEUNIT_MS);
  }
  fmod_audio_errcheck("_sound->setLoopPoints()", hr);
}
