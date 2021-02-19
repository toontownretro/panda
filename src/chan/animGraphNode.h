/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animGraphNode.h
 * @author lachbr
 * @date 2021-02-18
 */

#ifndef ANIMGRAPHNODE_H
#define ANIMGRAPHNODE_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "namable.h"
#include "luse.h"
#include "pointerTo.h"
#include "pvector.h"

class MovingPartMatrix;
class AnimChannelBase;

/**
 * The fundamental base class for all nodes in the animation graph.  Each node
 * produces a single output from one more more inputs.
 */
class EXPCL_PANDA_CHAN AnimGraphNode : public TypedWritableReferenceCount, public Namable {
PUBLISHED:
  AnimGraphNode(const std::string &name);

  INLINE void add_input(AnimGraphNode *input);
  INLINE int get_num_inputs() const;
  INLINE AnimGraphNode *get_input(int n) const;

  INLINE const LPoint3 &get_position() const;
  INLINE const LQuaternion &get_rotation() const;
  INLINE const LVector3 &get_scale() const;
  INLINE const LVector3 &get_shear() const;

  virtual int get_max_inputs() const;

  // Replication of AnimControl interfaces that simply call into all the
  // inputs.  When an AnimSampleNode is reached, actually calls the identical
  // AnimControl method.
  virtual void play();
  virtual void play(double from, double to);
  virtual void loop(bool restart);
  virtual void loop(bool restart, double from, double to);
  virtual void pingpong(bool restart);
  virtual void pingpong(bool restart, double from, double to);
  virtual void stop();
  virtual void pose(double frame);
  virtual void set_play_rate(double play_rate);

public:
  virtual void wait_pending();
  virtual void mark_channels(bool frame_blend_flag);
  virtual bool channel_has_changed(AnimChannelBase *channel, bool frame_blend_flag) const;

  virtual void evaluate(MovingPartMatrix *part, bool frame_blend_flag);

protected:
  LPoint3 _position;
  LQuaternion _rotation;
  LVector3 _scale;
  LVector3 _shear;

  typedef pvector<PT(AnimGraphNode)> Inputs;
  Inputs _inputs;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    Namable::init_type();
    register_type(_type_handle, "AnimGraphNode",
                  TypedWritableReferenceCount::get_class_type(),
                  Namable::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "animGraphNode.I"

#endif // ANIMGRAPHNODE_H
