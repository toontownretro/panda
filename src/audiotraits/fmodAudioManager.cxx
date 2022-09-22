/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fmodAudioManager.cxx
 * @author cort
 * @date 2003-01-22
 * Prior system by: cary
 * @author Stan Rosenbaum "Staque" - Spring 2006
 * @author lachbr
 * @date 2020-10-04
 */

#include "pandabase.h"
#include "config_audio.h"
#include "config_fmodAudio.h"
#include "dcast.h"

// Panda headers.
#include "clockObject.h"
#include "config_audio.h"
#include "config_putil.h"
#include "fmodAudioManager.h"
#include "fmodAudioSound.h"
#include "filename.h"
#include "virtualFileSystem.h"
#include "reMutexHolder.h"
#include "pStatCollector.h"
#include "pStatTimer.h"
#include "nullAudioSound.h"
#include "memoryHook.h"
#include "rayTraceScene.h"
#include "look_at.h"

// Panda DSP types.
#include "load_dso.h"
#include "thread.h"
#include "pmutex.h"
#include "clockObject.h"
#include "fmodAudioEngine.h"

// FMOD Headers.
#include <fmod.hpp>
#include <fmod_errors.h>

static PStatCollector get_sound_coll("App:FMOD:GetSound");
static PStatCollector get_sound_resolve_coll("App:FMOD:GetSound:ResolveFilename");
static PStatCollector get_sound_get_file("App:FMOD:GetSound:GetFile");
static PStatCollector get_sound_create_coll("App:FMOD:GetSound:CreateSound");
static PStatCollector get_sound_insert_coll("App:FMOD:GetSound:InsertSound");

TypeHandle FMODAudioManager::_type_handle;
ReMutex FMODAudioManager::_lock;

/**
 *
 */
FMODAudioManager::
FMODAudioManager(const std::string &name, FMODAudioManager *parent, FMODAudioEngine *engine) :
  _engine(engine)
{
  ReMutexHolder holder(_lock);
  FMOD_RESULT result;

  FMOD::System *system = _engine->get_system();

  _engine->add_manager(this);

  _concurrent_sound_limit = 0;

  _active = true;

  _is_valid = true;

  result = system->createChannelGroup(name.c_str(), &_channelgroup);
  fmod_audio_errcheck("_system->createChannelGroup()", result);
  if (parent != nullptr) {
    result = parent->_channelgroup->addGroup(_channelgroup);
    fmod_audio_errcheck("add channelgroup child", result);
  }
}

/**
 *
 */
FMODAudioManager::
~FMODAudioManager() {
  ReMutexHolder holder(_lock);

  // Be sure to delete associated sounds before deleting the manager!
  FMOD_RESULT result;

  // Release all of our sounds
  _sounds_playing.clear();

  _all_sounds.clear();

  // Release all DSPs
  remove_all_dsps();

  // Remove me from the managers list.
  _engine->remove_manager(this);

  if (_channelgroup != nullptr) {
    _channelgroup->release();
    _channelgroup = nullptr;
  }
}

/**
 * Inserts the specified DSP filter into the DSP chain at the specified index.
 * Returns true if the DSP filter is supported by the audio implementation,
 * false otherwise.
 */
bool FMODAudioManager::
insert_dsp(int index, DSP *panda_dsp) {
  ReMutexHolder holder(_lock);

  // If it's already in there, take it out and put it in the new spot.
  remove_dsp(panda_dsp);

  FMOD::DSP *dsp = _engine->create_fmod_dsp(panda_dsp);
  if (!dsp) {
    fmodAudio_cat.warning()
      << panda_dsp->get_type().get_name()
      << " unsupported by FMOD audio implementation.\n";
    return false;
  }

  FMOD_RESULT ret;
  ret = _channelgroup->addDSP(index, dsp);
  fmod_audio_errcheck("_channelgroup->addDSP()", ret);

  // Keep track of our DSPs.
  _engine->add_manager_to_dsp(panda_dsp, this);
  _dsps[panda_dsp] = dsp;

  return true;
}

/**
 * Removes the specified DSP filter from the DSP chain. Returns true if the
 * filter was in the DSP chain and was removed, false otherwise.
 */
bool FMODAudioManager::
remove_dsp(DSP *panda_dsp) {
  ReMutexHolder holder(_lock);

  auto itr = _dsps.find(panda_dsp);
  if (itr == _dsps.end()) {
    return false;
  }

  FMOD::DSP *dsp = itr->second;

  FMOD_RESULT ret;
  ret = _channelgroup->removeDSP(dsp);
  fmod_audio_errcheck("_channelGroup->removeDSP()", ret);

  ret = dsp->release();
  fmod_audio_errcheck("dsp->release()", ret);

  _engine->remove_manager_from_dsp(panda_dsp, this);
  _dsps.erase(itr);

  return true;
}

/**
 * Removes all DSP filters from the DSP chain.
 */
void FMODAudioManager::
remove_all_dsps() {
  ReMutexHolder holder(_lock);

  for (auto itr = _dsps.begin(); itr != _dsps.end(); itr++) {
    DSP *panda_dsp = itr->first;
    FMOD::DSP *fmod_dsp = itr->second;
    FMOD_RESULT ret;

    ret = _channelgroup->removeDSP(fmod_dsp);
    fmod_audio_errcheck("_channelgroup->removeDSP()", ret);

    ret = fmod_dsp->release();
    fmod_audio_errcheck("fmod_dsp->release()", ret);

    _engine->remove_manager_from_dsp(panda_dsp, this);
  }

  _dsps.clear();
}

/**
 * Returns the number of DSP filters present in the DSP chain.
 */
int FMODAudioManager::
get_num_dsps() const {
  // Can't use _channelgroup->getNumDSPs() because that includes DSPs that are
  // created internally by FMOD.  We want to return the number of user-created
  // DSPs.

  return (int)_dsps.size();
}

/**
 * This just check to make sure the FMOD System is up and running correctly.
 */
bool FMODAudioManager::
is_valid() {
  return _is_valid;
}

/**
 * This is what creates a sound instance.
 */
PT(AudioSound) FMODAudioManager::
get_sound(const Filename &file_name, bool positional, bool stream) {
  ReMutexHolder holder(_lock);

  PStatTimer timer(get_sound_coll);
  // Get the FMOD::Sound object containing the audio data,
  // will be cached and shared between multiple AudioSounds
  // referencing the same filename.
  PT(FMODSoundHandle) handle = _engine->get_sound_cache()->get_sound(file_name, positional, stream);
  if (handle != nullptr) {
    // Build a new AudioSound from the audio data.
    get_sound_create_coll.start();
    PT(FMODAudioSound) sound = new FMODAudioSound(this, handle);
    get_sound_create_coll.stop();

    get_sound_insert_coll.start();
    _all_sounds.insert(sound);
    get_sound_insert_coll.stop();

    return sound;

  } else {
    audio_error("createSound(" << file_name << "): File not found or cannot be loaded.");
    return get_null_sound();
  }
}

/**
 *
 */
PT(AudioSound) FMODAudioManager::
get_sound(AudioSound *source) {
  AudioSound *null_sound = get_null_sound();
  if (source == null_sound) {
    return null_sound;
  }

  FMODAudioSound *fmod_source;
  DCAST_INTO_R(fmod_source, source, null_sound);

  PT(FMODAudioSound) sound = new FMODAudioSound(this, fmod_source);
  _all_sounds.insert(sound);
  return sound;
}

/**
 * Creates a sound from a MovieAudio.
 */
PT(AudioSound) FMODAudioManager::
get_sound(MovieAudio *source, bool positional, bool stream) {
  ReMutexHolder holder(_lock);

  PStatTimer timer(get_sound_coll);
  // Get the FMOD::Sound object containing the audio data,
  // will be cached and shared between multiple AudioSounds
  // referencing the same filename.
  PT(FMODSoundHandle) handle = _engine->get_sound_cache()->get_sound(source, positional, stream);
  if (handle != nullptr) {
    // Build a new AudioSound from the audio data.
    get_sound_create_coll.start();
    PT(FMODAudioSound) sound = new FMODAudioSound(this, handle);
    get_sound_create_coll.stop();

    get_sound_insert_coll.start();
    _all_sounds.insert(sound);
    get_sound_insert_coll.stop();

    return sound;

  } else {
    audio_error("createSound(" << source->get_filename() << "): MovieAudio file not found or cannot be loaded.");
    return get_null_sound();
  }
}

/**
 * Sets the volume of the AudioManager.  It is not an override, but a
 * multiplier.
 */
void FMODAudioManager::
set_volume(PN_stdfloat volume) {
  ReMutexHolder holder(_lock);
  FMOD_RESULT result;
  result = _channelgroup->setVolume(volume);
  fmod_audio_errcheck("_channelgroup->setVolume()", result);
}

/**
 * Returns the AudioManager's volume.
 */
PN_stdfloat FMODAudioManager::
get_volume() const {
  ReMutexHolder holder(_lock);
  float volume;
  FMOD_RESULT result;
  result = _channelgroup->getVolume(&volume);
  fmod_audio_errcheck("_channelgroup->getVolume()", result);
  return (PN_stdfloat)volume;
}

/**
 * Turn on/off.
 */
void FMODAudioManager::
set_active(bool active) {
  ReMutexHolder holder(_lock);
  if (_active != active) {
    _active = active;

    // Tell our AudioSounds to adjust:
    for (AllSounds::iterator i = _all_sounds.begin();
         i != _all_sounds.end();
         ++i) {
      (*i)->set_active(_active);
    }
  }
}

/**
 *
 */
bool FMODAudioManager::
get_active() const {
  return _active;
}

/**
 * Stop playback on all sounds managed by this manager.
 */
void FMODAudioManager::
stop_all_sounds() {
  ReMutexHolder holder(_lock);
  // We have to walk through this list with some care, since stopping a sound
  // may also remove it from the set (if there are no other references to the
  // sound).
  AllSounds::iterator i;
  i = _all_sounds.begin();
  while (i != _all_sounds.end()) {
    AllSounds::iterator next = i;
    ++next;

    (*i)->stop();
    i = next;
  }
}

/**
 * Perform all per-frame update functions.
 */
void FMODAudioManager::
update() {
  ReMutexHolder holder(_lock);

  // Call finished() and release our reference to sounds that have finished
  // playing.
  update_sounds();
}

/**
 *
 */
void FMODAudioManager::
set_concurrent_sound_limit(unsigned int limit) {
  ReMutexHolder holder(_lock);
  _concurrent_sound_limit = limit;
  reduce_sounds_playing_to(_concurrent_sound_limit);
}

/**
 *
 */
unsigned int FMODAudioManager::
get_concurrent_sound_limit() const {
  return _concurrent_sound_limit;
}

/**
 *
 */
void FMODAudioManager::
reduce_sounds_playing_to(unsigned int count) {
  ReMutexHolder holder(_lock);

  // first give all sounds that have finished a chance to stop, so that these
  // get stopped first
  update_sounds();

  int limit = _sounds_playing.size() - count;
  while (limit-- > 0) {
    SoundsPlaying::iterator sound = _sounds_playing.begin();
    nassertv(sound != _sounds_playing.end());
    // When the user stops a sound, there is still a PT in the user's hand.
    // When we stop a sound here, however, this can remove the last PT.  This
    // can cause an ugly recursion where stop calls the destructor, and the
    // destructor calls stop.  To avoid this, we create a temporary PT, stop
    // the sound, and then release the PT.
    PT(FMODAudioSound) s = (*sound);
    s->stop();
  }
}

/**
 * NOT USED FOR FMOD!!! Clears a sound out of the sound cache.
 */
void FMODAudioManager::
uncache_sound(const Filename &file_name) {
  audio_debug("FMODAudioManager::uncache_sound(\""<<file_name<<"\")");
}

/**
 * NOT USED FOR FMOD!!! Clear out the sound cache.
 */
void FMODAudioManager::
clear_cache() {
  audio_debug("FMODAudioManager::clear_cache()");
}

/**
 * NOT USED FOR FMOD!!! Set the number of sounds that the cache can hold.
 */
void FMODAudioManager::
set_cache_limit(unsigned int count) {
  audio_debug("FMODAudioManager::set_cache_limit(count="<<count<<")");
}

/**
 * NOT USED FOR FMOD!!! Gets the number of sounds that the cache can hold.
 */
unsigned int FMODAudioManager::
get_cache_limit() const {
  audio_debug("FMODAudioManager::get_cache_limit() returning ");
  return 0;
}

/**
 * Returns the FMOD DSP associated with the Panda DSP, or nullptr if it doesn't
 * exist.
 */
FMOD::DSP *FMODAudioManager::
get_fmod_dsp(DSP *panda_dsp) const {
  auto itr = _dsps.find(panda_dsp);
  if (itr == _dsps.end()) {
    return nullptr;
  }

  return itr->second;
}

/**
 * Inform the manager that a sound is about to play.  The manager will add
 * this sound to the table of sounds that are playing.
 */
void FMODAudioManager::
starting_sound(FMODAudioSound *sound) {
  ReMutexHolder holder(_lock);

  // If the sound is already in there, don't do anything.
  if (_sounds_playing.find(sound) != _sounds_playing.end()) {
    return;
  }

  // first give all sounds that have finished a chance to stop, so that these
  // get stopped first
  update_sounds();

  if (_concurrent_sound_limit) {
    reduce_sounds_playing_to(_concurrent_sound_limit-1); // because we're about to add one
  }

  _sounds_playing.insert(sound);
}

/**
 * Inform the manager that a sound is finished or someone called stop on the
 * sound (this should not be called if a sound is only paused).
 */
void FMODAudioManager::
stopping_sound(FMODAudioSound *sound) {
  ReMutexHolder holder(_lock);
  _sounds_playing.erase(sound); // This could case the sound to destruct.
}

/**
 * Removes the indicated sound from the manager's list of sounds.
 */
void FMODAudioManager::
release_sound(FMODAudioSound *sound) {
  ReMutexHolder holder(_lock);
  AllSounds::iterator ai = _all_sounds.find(sound);
  if (ai != _all_sounds.end()) {
    _all_sounds.erase(ai);
  }
}

/**
 * Calls finished() on any sounds that have finished playing.
 */
void FMODAudioManager::
update_sounds() {
  ReMutexHolder holder(_lock);

  // Update any dirty DSPs applied to our sounds.
  for (AllSounds::iterator it = _all_sounds.begin(); it != _all_sounds.end();
       ++it) {
    (*it)->update();
  }

  // See if any of our playing sounds have ended we must first collect a
  // seperate list of finished sounds and then iterated over those again
  // calling their finished method.  We can't call finished() within a loop
  // iterating over _sounds_playing since finished() modifies _sounds_playing
  SoundsPlaying sounds_finished;

  SoundsPlaying::iterator i = _sounds_playing.begin();
  for (; i != _sounds_playing.end(); ++i) {
    FMODAudioSound *sound = (*i);
    if (sound->status() != AudioSound::PLAYING) {
      sounds_finished.insert(*i);
    }
  }

  i = sounds_finished.begin();
  for (; i != sounds_finished.end(); ++i) {
    (**i).finished();
  }
}
