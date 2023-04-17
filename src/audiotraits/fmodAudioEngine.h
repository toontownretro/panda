/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fmodAudioEngine.h
 * @author brian
 * @date 2022-09-19
 */

#ifndef FMODAUDIOENGINE_H
#define FMODAUDIOENGINE_H

#include "pandabase.h"
#include "typedReferenceCount.h"
#include "pointerTo.h"
#include "audioEngine.h"
#include "audioManager.h"
#include "audioSound.h"
#include "audioTracer.h"
#include "luse.h"
#include "fmodAudioSound.h"
#include "fmodSoundCache.h"
#include "dsp.h"
#include "job.h"

#include <fmod.hpp>

#ifdef HAVE_STEAM_AUDIO
#include <phonon.h>
#endif

class FMODAudioManager;
class DSP;

/**
 *
 */
class FMODAudioEngine : public AudioEngine {
  DECLARE_CLASS(FMODAudioEngine, AudioEngine);

public:
  virtual PT(AudioManager) make_manager(const std::string &name, AudioManager *parent) override;

  virtual void set_3d_listener_attributes(const LPoint3 &pos, const LQuaternion &quat, const LVector3 &vel) override;
  virtual LPoint3 get_3d_listener_pos() const override;
  virtual LQuaternion get_3d_listener_quat() const override;
  virtual LVector3 get_3d_listener_velocity() const override;

  virtual void set_3d_unit_scale(PN_stdfloat factor) override;
  virtual PN_stdfloat get_3d_unit_scale() const override;

  virtual void set_tracer(AudioTracer *tracer) override;
  virtual void clear_tracer() override;

  virtual void update() override;

  void add_manager(FMODAudioManager *mgr);
  void remove_manager(FMODAudioManager *mgr);

  void add_manager_to_dsp(DSP *dsp, FMODAudioManager *mgr);
  void remove_manager_from_dsp(DSP *dsp, FMODAudioManager *mgr);
  void update_dirty_dsps();

  FMOD_DSP_TYPE get_fmod_dsp_type(DSP::DSPType panda_type);
  FMOD::DSP *create_fmod_dsp(DSP *panda_dsp);
  void configure_dsp(DSP *panda_dsp, FMOD::DSP *dsp);

  INLINE FMOD::System *get_system() const;
  INLINE FMOD::ChannelGroup *get_master_channel_group() const;

  INLINE const std::string &get_dls_name() const;

  INLINE FMODSoundCache *get_sound_cache() const;

  INLINE bool is_using_steam_audio() const;

  bool calc_sound_occlusion(FMODAudioSound *sound, float *transmission);

  virtual void set_audio_probe_data(CPTA_uchar data) override;
  virtual void clear_audio_probe_data() override;

  virtual void set_audio_scene_data(CPTA_uchar verts, CPTA_uchar tris,
          CPTA_uchar tri_materials, CPTA_uchar materials);
  virtual void clear_audio_scene_data();

public:
  FMODAudioEngine();
  ~FMODAudioEngine();

  virtual bool initialize() override;

#ifdef HAVE_STEAM_AUDIO
  bool init_steam_audio();
  void shutdown_steam_audio();
  void do_steam_audio_direct_sim();
  void do_steam_audio_reflections_sim();
  void do_steam_audio_pathing_sim();
#endif

private:
  FMOD::System *_system;
  FMOD::ChannelGroup *_master_channel_group;

  PT(AudioTracer) _tracer;

  LPoint3 _listener_pos;
  LQuaternion _listener_quat;
  LVector3 _listener_vel;
  LightMutex _listener_transform_lock;
  UpdateSeq _listener_transform_seq;

  PN_stdfloat _unit_scale;

  // DLS filename for software MIDI synth.
  std::string _dls_name;

  PT(FMODSoundCache) _sound_cache;

  typedef pset<FMODAudioManager *, pointer_hash> ManagerList;
  ManagerList _managers;

  // We need this to support applying the same DSP onto multiple audio
  // managers.  We run a once-per-frame update method that iterates over all
  // the DSPs, and for each one, checks if the dirty flag is set.  If it is,
  // we configure the DSP on all audio managers that it has been applied to.
  typedef pflat_hash_map<DSP *, ManagerList, pointer_hash> DSPManagers;
  DSPManagers _dsp_managers;

#ifdef HAVE_STEAM_AUDIO
  // Steam Audio API objects.
  int _num_sims;
  IPLContext _ipl_context;
  IPLHRTF _ipl_hrtf;
  IPLSimulator _ipl_simulator;
  IPLSimulationSharedInputs _sim_inputs;
  IPLSource _ipl_listener_source;
  IPLSimulationInputs _listener_inputs;
  IPLProbeBatch _ipl_probe_batch;
  IPLScene _ipl_scene;
  IPLStaticMesh _ipl_scene_mesh;

  PT(Thread) _ipl_reflections_thread;
  PT(Job) _ipl_reflections_job;

  bool _steam_audio_initialized;

  // Handles to DSPs implemented in the Steam Audio FMOD plugin.
  unsigned int _ipl_plugin_handle;
  unsigned int _ipl_spatialize_handle;
  unsigned int _ipl_reverb_handle;
  unsigned int _ipl_mixer_return_handle;
#endif

  friend class FMODAudioSound;
  friend class FMODAudioManager;
  friend class SteamAudioThread;
};

/**
 *
 */
class FMODAudioEngineProxy : public AudioEngineProxy {
public:
  virtual PT(AudioEngine) make_engine() const override;
};

#include "fmodAudioEngine.I"

#endif // FMODAUDIOENGINE_H
