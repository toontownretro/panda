/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fmodAudioEngine.I
 * @author brian
 * @date 2022-09-19
 */

/**
 * Returns the FMOD::System object.
 */
INLINE FMOD::System *FMODAudioEngine::
get_system() const {
  return _system;
}

/**
 * Returns the top-level FMOD::ChannelGroup object.  All manager ChannelGroups
 * are children of this ChannelGroup.
 */
INLINE FMOD::ChannelGroup *FMODAudioEngine::
get_master_channel_group() const {
  return _master_channel_group;
}

/**
 *
 */
INLINE const std::string &FMODAudioEngine::
get_dls_name() const {
  return _dls_name;
}

/**
 *
 */
INLINE FMODSoundCache *FMODAudioEngine::
get_sound_cache() const {
  return _sound_cache;
}

/**
 *
 */
INLINE bool FMODAudioEngine::
is_using_steam_audio() const {
#ifndef HAVE_STEAM_AUDIO
  return false;
#else
  return _ipl_context != nullptr;
#endif
}
