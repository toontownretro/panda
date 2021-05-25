/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animSequencePlayer.h
 * @author brian
 * @date 2021-05-19
 */

#ifndef ANIMSEQUENCEPLAYER_H
#define ANIMSEQUENCEPLAYER_H

#include "pandabase.h"
#include "animGraphNode.h"
#include "animSequence.h"
#include "character.h"
#include "animLayer.h"

/**
 * An anim node that plays AnimSequences.  Plays one base sequence at a time,
 * but can optionally transition between sequence changes.  Also supports
 * playing up to 15 animation layers.
 */
class EXPCL_PANDA_ANIM AnimSequencePlayer final : public AnimGraphNode {
PUBLISHED:

  enum AdvanceMode {
    // The player should automatically advance the cycle when evaluating.
    AM_auto,
    // The user is responsible for advancing the cycle.
    AM_manual,
  };

  AnimSequencePlayer(const std::string &name, Character *character);

  INLINE void set_character(Character *character);
  INLINE Character *get_character() const;

  INLINE void set_advance_mode(AdvanceMode mode);
  INLINE AdvanceMode get_advance_mode() const;

  void reset_sequence(int sequence);
  void reset_sequence_info();

  INLINE void set_sequence(int seq);
  INLINE int get_curr_sequence() const;

  PN_stdfloat get_sequence_cycle_rate(int sequence) const;
  PN_stdfloat get_last_visible_cycle(int sequence) const;
  PN_stdfloat get_anim_time_interval() const;
  PN_stdfloat clamp_cycle(PN_stdfloat cycle, bool is_looping) const;

  void advance();

  INLINE void set_cycle(PN_stdfloat cycle);
  INLINE PN_stdfloat get_cycle() const;

  INLINE void set_anim_time(PN_stdfloat time);
  INLINE PN_stdfloat get_anim_time() const;

  INLINE void set_play_rate(PN_stdfloat rate);
  INLINE PN_stdfloat get_play_rate() const;

  INLINE void set_transitions_enabled(bool enable);
  INLINE bool get_transitions_enabled() const;

  INLINE bool is_sequence_looping() const;
  INLINE bool is_sequence_finished() const;

  INLINE void set_new_sequence_parity(int parity);
  INLINE int get_new_sequence_parity() const;
  INLINE int get_prev_sequence_parity() const;

  int add_gesture_sequence(int sequence, bool auto_kill = true);
  int add_gesture_sequence(int sequence, PN_stdfloat duration, bool auto_kill = true);
  int add_gesture(int activity, int sequence, bool auto_kill = true);
  int add_gesture_with_duration(int activity, int sequence, PN_stdfloat duration, bool auto_kill = true);
  bool is_playing_gesture(int activity) const;
  void reset_layer(int layer, int activity, int sequence, bool auto_kill = true);
  void restart_gesture(int activity, bool add_if_missing = true, bool auto_kill = true);
  void remove_gesture(int activity);
  void remove_all_gestures();

  int add_layered_sequence(int sequence, int priority);

  void set_layer_priority(int layer, int priority);

  bool is_valid_layer(int layer) const;

  int allocate_layer(int priority);

  void set_layer_duration(int layer, PN_stdfloat duration);
  PN_stdfloat get_layer_duration(int layer) const;

  void set_layer_cycle(int layer, PN_stdfloat cycle);
  void set_layer_cycle(int layer, PN_stdfloat cycle, PN_stdfloat prev_cycle);
  PN_stdfloat get_layer_cycle(int layer) const;

  void set_layer_prev_cycle(int layer, PN_stdfloat cycle);
  PN_stdfloat get_layer_prev_cycle(int layer) const;

  void set_layer_play_rate(int layer, PN_stdfloat rate);
  PN_stdfloat get_layer_play_rate(int layer) const;

  void set_layer_weight(int layer, PN_stdfloat weight);
  PN_stdfloat get_layer_weight(int layer) const;

  void set_layer_blend_in(int layer, PN_stdfloat blend_in);
  PN_stdfloat get_layer_blend_in(int layer) const;

  void set_layer_blend_out(int layer, PN_stdfloat blend_out);
  PN_stdfloat get_layer_blend_out(int layer) const;

  void set_layer_order(int layer, int order);
  int get_layer_order(int layer) const;

  void set_layer_auto_kill(int layer, bool auto_kill);
  bool get_layer_auto_kill(int layer) const;

  void set_layer_looping(int layer, bool looping);
  bool get_layer_looping(int layer) const;

  void set_layer_no_restore(int layer, bool no_restore);
  bool get_layer_no_resture(int layer) const;

  void mark_layer_active(int layer);

  int get_layer_activity(int layer) const;

  void set_layer_sequence(int layer, int seq);
  int get_layer_sequence(int layer) const;

  int find_gesture_layer(int activity) const;

  void remove_layer(int layer, PN_stdfloat kill_rate = 0.2f, PN_stdfloat kill_delay = 0.0f);
  void fast_remove_layer(int layer);

  AnimLayer *get_layer(int index);
  void set_num_layers(int count);
  int get_num_layers() const;

  bool has_active_layer() const;

public:
  virtual void evaluate(AnimGraphEvalContext &context) override;

private:
  typedef pvector<AnimLayer> LayerVec;

  // Used for layered animations.
  LayerVec _layers;
  // Used for sequence transitions, if enabled.
  LayerVec _sequence_queue;

  PN_stdfloat _prev_anim_time;
  PN_stdfloat _anim_time;
  PN_stdfloat _cycle;
  PN_stdfloat _play_rate;
  bool _sequence_finished;
  bool _sequence_loops;
  int _sequence;

  int _new_sequence_parity;
  int _prev_sequence_parity;

  unsigned long _seed;

  bool _transitions_enabled;

  AdvanceMode _advance_mode;

  Character *_character;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AnimGraphNode::init_type();
    register_type(_type_handle, "AnimSequencePlayer",
                  AnimGraphNode::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "animSequencePlayer.I"

#endif // ANIMSEQUENCEPLAYER_H
