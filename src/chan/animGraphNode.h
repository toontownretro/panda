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
class PartBundle;

class JointTransform {
public:
  JointTransform() = default;
  JointTransform(const JointTransform &copy) :
    _position(copy._position),
    _rotation(copy._rotation),
    _scale(copy._scale),
    _shear(copy._shear)
  {
  }

  JointTransform(JointTransform &&other) :
    _position(std::move(other._position)),
    _rotation(std::move(other._rotation)),
    _scale(std::move(other._scale)),
    _shear(std::move(other._shear))
  {
  }

  void operator = (JointTransform &&other) {
    _position = std::move(other._position);
    _rotation = std::move(other._rotation);
    _scale = std::move(other._scale);
    _shear = std::move(other._shear);
  }

  LVector3 _position;
  LQuaternion _rotation;
  LVector3 _scale;
  LVector3 _shear;
};

class AnimGraphEvalContext {
public:
  AnimGraphEvalContext(MovingPartMatrix **parts, int num_parts,
                       bool frame_blend_flag) {
    _joints.resize(num_parts);
    _parts = parts;
    _frame_blend = frame_blend_flag;
  }

  AnimGraphEvalContext(const AnimGraphEvalContext &copy) :
    _frame_blend(copy._frame_blend),
    _parts(copy._parts)
  {
    _joints.resize(copy._joints.size());
  }

  AnimGraphEvalContext(AnimGraphEvalContext &&other) :
    _frame_blend(std::move(other._frame_blend)),
    _joints(std::move(other._joints)),
    _parts(std::move(other._parts))
  {
  }

  void operator = (AnimGraphEvalContext &&other) {
    _frame_blend = std::move(other._frame_blend);
    _joints = std::move(other._joints);
    _parts = std::move(other._parts);
  }

  void mix(const AnimGraphEvalContext &a, const AnimGraphEvalContext &b, PN_stdfloat c);

  typedef pvector<JointTransform> JointTransforms;
  JointTransforms _joints;

  bool _frame_blend;
  MovingPartMatrix **_parts;
};

/**
 * The fundamental base class for all nodes in the animation graph.  Each node
 * produces a single output from one more more inputs.
 */
class EXPCL_PANDA_CHAN AnimGraphNode : public TypedWritableReferenceCount, public Namable {
PUBLISHED:
  AnimGraphNode(const std::string &name);

public:
  virtual void evaluate(AnimGraphEvalContext &context) = 0;

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
