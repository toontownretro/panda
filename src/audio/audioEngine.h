/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file audioEngine.h
 * @author brian
 * @date 2022-09-20
 */

#ifndef AUDIOENGINE_H
#define AUDIOENGINE_H

#include "pandabase.h"
#include "typedReferenceCount.h"
#include "luse.h"
#include "pointerTo.h"
#include "audioManager.h"
#include "memoryBase.h"
#include "audioSound.h"
#include "collideMask.h"
#include "traceInterface.h"

class AudioEngineProxy;

/**
 *
 */
class EXPCL_PANDA_AUDIO AudioEngine : public TypedReferenceCount {
  DECLARE_CLASS(AudioEngine, TypedReferenceCount);

PUBLISHED:
  static PT(AudioEngine) make_engine();

  virtual PT(AudioManager) make_manager(const std::string &name, AudioManager *parent = nullptr) = 0;

  virtual void set_3d_listener_attributes(const LPoint3 &pos, const LQuaternion &quat, const LVector3 &vel) = 0;
  virtual LPoint3 get_3d_listener_pos() const = 0;
  virtual LQuaternion get_3d_listener_quat() const = 0;
  virtual LVector3 get_3d_listener_velocity() const = 0;

  virtual void set_3d_unit_scale(PN_stdfloat scale) = 0;
  virtual PN_stdfloat get_3d_unit_scale() const = 0;

  virtual void set_tracer(TraceInterface *tracer, CollideMask mask);
  virtual void clear_tracer();

  virtual bool initialize() = 0;

  virtual void update() = 0;

  virtual void set_audio_probe_data(CPTA_uchar data);
  virtual void clear_audio_probe_data();

  virtual void set_audio_scene_data(CPTA_uchar verts, CPTA_uchar tris,
          CPTA_uchar tri_materials, CPTA_uchar materials);
  virtual void clear_audio_scene_data();

  virtual bool load_bank(const Filename &filename);
  virtual PT(AudioSound) get_event(const std::string &path);

public:
  static void register_engine_proxy(AudioEngineProxy *proxy);

private:
  static AudioEngineProxy *_engine_proxy;
};

/**
 *
 */
class AudioEngineProxy : public MemoryBase {
public:
  virtual PT(AudioEngine) make_engine() const = 0;
};

#include "audioEngine.I"

#endif // AUDIOENGINE_H
