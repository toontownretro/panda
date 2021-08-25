/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animLayer.h
 * @author brian
 * @date 2021-05-24
 */

#ifndef ANIMLAYER_H
#define ANIMLAYER_H

#include "pandabase.h"
#include "numeric_types.h"
#include "animEvalContext.h"

class AnimEventQueue;

static const int max_anim_layers = 15;

/**
 * An animation layer, used by the AnimSequencePlayer.
 */
class EXPCL_PANDA_ANIM AnimLayer {
PUBLISHED:
  enum PlayMode {
    PM_none,
    PM_pose,
    PM_loop,
    PM_play,
    PM_pingpong,
  };

  AnimLayer();

  void init(Character *character);

  bool is_active() const { return (_flags & F_active) != 0; }
  bool is_autokill() const { return (_flags & F_autokill) != 0; }
  bool is_killme() const { return (_flags & F_killme) != 0; }
  bool is_autoramp() const { return (_blend_in != 0.0f || _blend_out != 0.0f); }
  void killme() { _flags |= F_killme; }
  void dying() { _flags |= F_dying; _flags &= ~F_active; }
  bool is_dying() const { return (_flags & F_dying) != 0; }

  void dead() {
    _flags &= ~(F_dying | F_active);
    _sequence = -1;
    _order = -1;
    _weight = 0;
    _play_mode = PM_none;
    _cycle = 0;
    _prev_cycle = 0;
    _last_advance_time = 0;
  }

  bool is_abandoned() const;
  void mark_active();

  bool is_playing() const;
  void accumulate_cycle();
  PN_stdfloat clamp_cycle(PN_stdfloat c) const;

  void update();
  void calc_pose(AnimEvalContext &context, AnimEvalData &data, bool transition);

  void get_events(AnimEventQueue &queue, unsigned int type);

  INLINE PN_stdfloat get_fade_out(PN_stdfloat frame_time) const;

  // Client/server.
  PlayMode _play_mode;
  PN_stdfloat _start_cycle;
  PN_stdfloat _play_cycles;
  PN_stdfloat _play_rate;
  PN_stdfloat _cycle;

  int _sequence;
  int _sequence_parity;
  int _prev_sequence_parity;
  PN_stdfloat _prev_cycle;
  PN_stdfloat _weight;

  int _order;
  PN_stdfloat _layer_anim_time;
  PN_stdfloat _layer_fade_out_time;

  // Server only.
  enum Flags {
    F_active = 1 << 0,
    F_autokill = 1 << 1,
    F_killme = 1 << 2,
    F_dontrestore = 1 << 3,
    F_checkaccess = 1 << 4,
    F_dying = 1 << 5,
  };

  PN_stdfloat _unclamped_cycle;

  PN_stdfloat _ramp_weight;
  PN_stdfloat _kill_weight;

  int _flags;
  bool _sequence_finished;
  PN_stdfloat _blend_in;
  PN_stdfloat _blend_out;

  PN_stdfloat _kill_rate;
  PN_stdfloat _kill_delay;

  int _activity;

  int _priority;

  PN_stdfloat _last_event_check;
  PN_stdfloat _last_access;

  // For transitioning between animations in the layer.
  typedef pvector<AnimLayer> AnimLayers;
  AnimLayers _transition_queue;

  PN_stdfloat _last_advance_time;

  Character *_character;

  // The index of the channel playing on the layer the last time we checked
  // for events.
  int _last_event_channel;
  // The cycle of the layer the last time we checked for events.
  PN_stdfloat _last_event_cycle;
};

#include "animLayer.I"

#endif // ANIMLAYER_H
