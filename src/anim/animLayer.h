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

class AnimSequencePlayer;

static const int max_anim_layers = 15;

/**
 * An animation layer, used by the AnimSequencePlayer.
 */
class EXPCL_PANDA_ANIM AnimLayer {
PUBLISHED:
  AnimLayer();

  void init(AnimSequencePlayer *player);

  bool is_active() const { return (_flags & F_active) != 0; }
  bool is_autokill() const { return (_flags & F_autokill) != 0; }
  bool is_killme() const { return (_flags & F_killme) != 0; }
  bool is_autoramp() const { return (_blend_in != 0.0f || _blend_out != 0.0f); }
  void killme() { _flags |= F_killme; }
  void dying() { _flags |= F_dying; }
  bool is_dying() const { return (_flags & F_dying) != 0; }
  void dead() { _flags &= ~F_dying; }

  bool is_abandoned() const;
  void mark_active();

  void advance(PN_stdfloat interval, AnimSequencePlayer *owner);

  INLINE PN_stdfloat get_fade_out(PN_stdfloat frame_time) const;

  // Client/server.
  int _sequence;
  int _sequence_parity;
  PN_stdfloat _prev_cycle;
  PN_stdfloat _weight;
  PN_stdfloat _play_rate;
  PN_stdfloat _cycle;
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

  int _flags;
  bool _sequence_finished;
  bool _looping;
  PN_stdfloat _blend_in;
  PN_stdfloat _blend_out;

  PN_stdfloat _kill_rate;
  PN_stdfloat _kill_delay;

  int _activity;

  int _priority;

  PN_stdfloat _last_event_check;
  PN_stdfloat _last_access;

  AnimSequencePlayer *_player;
};

#include "animLayer.I"

#endif // ANIMLAYER_H
