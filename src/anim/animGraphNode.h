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
#include "plist.h"
#include "characterJoint.h"

static constexpr int max_joints = 256;

template<class T, int count_per_alloc>
class MemoryPool {
public:
  T *alloc() {
    T *p;

    if (!_free_blocks.empty()) {
      p = _free_blocks.back();
      //memset(p, 0, sizeof(T) * count_per_alloc);
      _free_blocks.pop_back();

    } else {
      p = new T[count_per_alloc];
      //memset(p, 0, sizeof(T) * count_per_alloc);
    }

    return p;
  }

  void free(T *p) {
    _free_blocks.push_back(p);
  }

private:
  plist<T *> _free_blocks;
};

class JointTransform {
public:
  JointTransform() = default;
  JointTransform(const JointTransform &copy) :
    _position(copy._position),
    _rotation(copy._rotation),
    _scale(copy._scale)
  {
  }

  JointTransform(JointTransform &&other) :
    _position(std::move(other._position)),
    _rotation(std::move(other._rotation)),
    _scale(std::move(other._scale))
  {
  }

  void operator = (JointTransform &&other) {
    _position = std::move(other._position);
    _rotation = std::move(other._rotation);
    _scale = std::move(other._scale);
  }

  LVector3 _position;
  LQuaternion _rotation;
  LVector3 _scale;
};

typedef MemoryPool<JointTransform, max_joints> JointTransformPool;
extern JointTransformPool joint_transform_pool;

class AnimGraphEvalContext {
public:
  AnimGraphEvalContext(CharacterJoint *parts, int num_parts,
                       bool frame_blend_flag) {
    _joints = joint_transform_pool.alloc();
    _num_joints = num_parts;
    _parts = parts;
    _frame_blend = frame_blend_flag;
  }

  AnimGraphEvalContext(const AnimGraphEvalContext &copy) :
    _frame_blend(copy._frame_blend),
    _parts(copy._parts),
    _num_joints(copy._num_joints)
  {
    _joints = joint_transform_pool.alloc();
  }

  void steal(AnimGraphEvalContext &other) {
    _joints = other._joints;
    other._joints = nullptr;
  }

  ~AnimGraphEvalContext() {
    if (_joints != nullptr) {
      joint_transform_pool.free(_joints);
    }
  }

  void mix(const AnimGraphEvalContext &a, const AnimGraphEvalContext &b, PN_stdfloat c);

  JointTransform *_joints;
  int _num_joints;

  bool _frame_blend;

  // Character's joint list.
  CharacterJoint *_parts;
};

/**
 * The fundamental base class for all nodes in the animation graph.  Each node
 * produces a single output from one more more inputs.
 */
class EXPCL_PANDA_ANIM AnimGraphNode : public TypedWritableReferenceCount, public Namable {
PUBLISHED:
  AnimGraphNode(const std::string &name);
  virtual ~AnimGraphNode();

  INLINE int get_num_children() const;
  INLINE AnimGraphNode *get_child(int n) const;

protected:
  void add_child(AnimGraphNode *child);
  void remove_child(AnimGraphNode *child);

public:
  virtual void evaluate(AnimGraphEvalContext &context) = 0;

private:
  typedef pvector<PT(AnimGraphNode)> Children;
  Children _children;

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
