/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nullAudioEngine.h
 * @author brian
 * @date 2022-09-21
 */

#ifndef NULLAUDIOENGINE_H
#define NULLAUDIOENGINE_H

#include "pandabase.h"
#include "audioEngine.h"

/**
 *
 */
class EXPCL_PANDA_AUDIO NullAudioEngine : public AudioEngine {
  DECLARE_CLASS(NullAudioEngine, AudioEngine);

public:
  virtual PT(AudioManager) make_manager(const std::string &name, AudioManager *parent = nullptr) override;

  virtual void set_3d_listener_attributes(const LPoint3 &pos, const LQuaternion &quat, const LVector3 &vel) override;
  virtual LPoint3 get_3d_listener_pos() const override;
  virtual LQuaternion get_3d_listener_quat() const override;
  virtual LVector3 get_3d_listener_velocity() const override;

  virtual void set_3d_unit_scale(PN_stdfloat scale) override;
  virtual PN_stdfloat get_3d_unit_scale() const override;

  virtual bool initialize() override;

  virtual void update() override;
};

#include "nullAudioEngine.I"

#endif // NULLAUDIOENGINE_H
