// Filename: nullAudioManager.h
// Created by:  skyler (June 6, 2001)
// Prior system by: cary
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef __NULL_AUDIO_MANAGER_H__
#define __NULL_AUDIO_MANAGER_H__

#include "audioManager.h"
#include "nullAudioSound.h"

class EXPCL_PANDA NullAudioManager : public AudioManager {
  // All of these methods are stubbed out to some degree.
  // If you're looking for a starting place for a new AudioManager,
  // please consider looking at the milesAudioManager.
  
public:
  NullAudioManager();
  virtual ~NullAudioManager();
  
  virtual PT(AudioSound) get_sound(const string&);
  virtual void drop_sound(const string&);

  virtual void set_volume(float);
  virtual float get_volume();
  
  virtual void set_active(bool);
  virtual bool get_active();
};

#endif /* __NULL_AUDIO_MANAGER_H__ */
