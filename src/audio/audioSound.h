/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file audioSound.h
 * @author skyler
 * @date 2001-06-06
 * Prior system by: cary
 */

#ifndef AUDIOSOUND_H
#define AUDIOSOUND_H

#include "config_audio.h"
#include "typedReferenceCount.h"
#include "pointerTo.h"
#include "filterProperties.h"
#include "cmath.h"
#include "luse.h"

class AudioManager;
class DSP;
class SteamAudioProperties;

class EXPCL_PANDA_AUDIO AudioSound : public TypedReferenceCount {
PUBLISHED:
  enum Note {
    N_c = 0,
    N_c_sharp = 1,
    N_d_flat = N_c_sharp,
    N_d = 2,
    N_d_sharp = 3,
    N_e_flat = N_d_sharp,
    N_e = 4,
    N_f = 5,
    N_f_sharp = 6,
    N_g_flat = N_f_sharp,
    N_g = 7,
    N_g_sharp = 8,
    N_a_flat = N_g_sharp,
    N_a = 9,
    N_a_sharp = 10,
    N_b_flat = N_a_sharp,
    N_b = 11,

    N_COUNT,
  };

  virtual ~AudioSound();

  // For best compatibility, set the loop_count, volume, and balance, prior to
  // calling play().  You may set them while they're playing, but it's
  // implementation specific whether you get the results.  - Calling play() a
  // second time on the same sound before it is finished will start the sound
  // again (creating a skipping or stuttering effect).
  virtual void play() = 0;
  virtual void stop() = 0;

  // loop: false = play once; true = play forever.  inits to false.
  virtual void set_loop(bool loop=true) = 0;
  virtual bool get_loop() const = 0;

  // loop_count: 0 = forever; 1 = play once; n = play n times.  inits to 1.
  virtual void set_loop_count(unsigned long loop_count=1) = 0;
  virtual unsigned long get_loop_count() const = 0;

  // loop_start: 0 = beginning.  expressed in seconds.  inits to 0.
  virtual void set_loop_start(PN_stdfloat loop_start=0) = 0;
  virtual PN_stdfloat get_loop_start() const = 0;

  /**
   * Control time position within the sound, in seconds.  This is similar (in
   * concept) to the seek position within a file.  The value starts at 0.0 (the
   * default) and ends at the value given by the length() method.
   *
   * In the past, this call did nothing if the sound was currently playing, and
   * it was necessary to call play() to effect the change.  This is no longer
   * the case; the time change takes effect immediately.
   *
   * If a sound is playing, calling get_time() repeatedly will return different
   * results over time.  e.g.
   * @code
   * PN_stdfloat percent_complete = s.get_time() / s.length();
   * @endcode
   */
  virtual void set_time(PN_stdfloat start_time=0.0) = 0;
  virtual PN_stdfloat get_time() const = 0;

  // 0 = minimum; 1.0 = maximum.  inits to 1.0.
  virtual void set_volume(PN_stdfloat volume=1.0) = 0;
  virtual PN_stdfloat get_volume() const = 0;

  // -1.0 is hard left 0.0 is centered 1.0 is hard right inits to 0.0.
  virtual void set_balance(PN_stdfloat balance_right=0.0) = 0;
  virtual PN_stdfloat get_balance() const = 0;

  // play_rate is any positive PN_stdfloat value.  inits to 1.0.
  virtual void set_play_rate(PN_stdfloat play_rate=1.0f) = 0;
  virtual PN_stdfloat get_play_rate() const = 0;
  // Music-oriented methods for adjusting play rate.
  INLINE void set_semitone_offset(PN_stdfloat offset);
  INLINE void set_tone_offset(PN_stdfloat offset);
  INLINE void set_semisemitone_offset(PN_stdfloat offset);
  INLINE void set_octave_offset(PN_stdfloat offset);
  INLINE void set_note_offset(Note base_note, int base_octave, Note target_note, int target_octave);

  // inits to manager's state.
  virtual void set_active(bool flag=true) = 0;
  virtual bool get_active() const = 0;

  // Set (or clear) the event that will be thrown when the sound finishes
  // playing.  To clear the event, pass an empty string.
  virtual void set_finished_event(const std::string& event) = 0;
  virtual const std::string& get_finished_event() const = 0;

  // There is no set_name(), this is intentional.
  virtual const std::string& get_name() const = 0;

  // return: playing time in seconds.
  virtual PN_stdfloat length() const = 0;

  // Controls the position of this sound's emitter.  px, py and pz are the
  // emitter's position.  vx, vy and vz are the emitter's velocity in UNITS
  // PER SECOND (default: meters).
  virtual void set_3d_attributes(const LPoint3 &pos, const LQuaternion &quat, const LVector3 &vel);
  virtual LPoint3 get_3d_position() const;
  virtual LQuaternion get_3d_quat() const;
  virtual LVector3 get_3d_velocity() const;

  // Controls the direction of this sound emitter. Currently implemented only for OpenAL.
  virtual void set_3d_direction(LVector3 d);
  virtual LVector3 get_3d_direction() const;

  // Controls the distance (in units) that this sound begins to fall off.
  // Also affects the rate it falls off.  Default is 1.0 CloserFaster, <1.0
  // FartherSlower, >1.0
  virtual void set_3d_min_distance(PN_stdfloat dist);
  virtual PN_stdfloat get_3d_min_distance() const;

  virtual PN_stdfloat get_sound_frequency() const;

  // Sets the angle of the inner cone of a directional sound source. In the zone inside of the inner cone
  // sound is emitted with the (normal) volume set by set_volume().
  virtual void set_3d_cone_inner_angle(PN_stdfloat angle);
  virtual PN_stdfloat get_3d_cone_inner_angle() const;

  // Sets the angle of the outer cone of a directional sound source. In the zone between
  // the inner and the outer cone the volume is attenuated.
  virtual void set_3d_cone_outer_angle(PN_stdfloat angle);
  virtual PN_stdfloat get_3d_cone_outer_angle() const;

  // Sets a factor applied to the volume set by set_volume() for the zone outside the outer cone.
  // By default this is 0 (so no sound is heard inside the outer zone).
  virtual void set_3d_cone_outer_gain(PN_stdfloat gain);
  virtual PN_stdfloat get_3d_cone_outer_gain() const;

  virtual int get_priority();
  virtual void set_priority(int priority);

  // DSP methods
  INLINE bool add_dsp_to_head(DSP *dsp);
  INLINE bool add_dsp_to_tail(DSP *dsp);
  virtual bool insert_dsp(int index, DSP *dsp);
  virtual bool remove_dsp(DSP *dsp);
  virtual void remove_all_dsps();
  virtual int get_num_dsps() const;

  virtual void apply_steam_audio_properties(const SteamAudioProperties &props);

  virtual void set_loop_range(PN_stdfloat start, PN_stdfloat end = -1.0f);

  enum SoundStatus { BAD, READY, PLAYING };
  virtual SoundStatus status() const = 0;

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out) const;

PUBLISHED:
  MAKE_PROPERTY(time, get_time, set_time);
  MAKE_PROPERTY(volume, get_volume, set_volume);
  MAKE_PROPERTY(balance, get_balance, set_balance);
  MAKE_PROPERTY(play_rate, get_play_rate, set_play_rate);
  MAKE_PROPERTY(active, get_active, set_active);
  MAKE_PROPERTY(name, get_name);

protected:
  AudioSound();

  friend class AudioManager;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "AudioSound",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

inline std::ostream &
operator << (std::ostream &out, const AudioSound &sound) {
  sound.output(out);
  return out;
}

#include "audioSound.I"

EXPCL_PANDA_AUDIO std::ostream &
operator << (std::ostream &out, AudioSound::SoundStatus status);

#endif /* AUDIOSOUND_H */
