/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eyeballNode.h
 * @author lachbr
 * @date 2021-03-24
 */

#ifndef EYEBALLNODE_H
#define EYEBALLNODE_H

#include "pandaNode.h"
#include "pta_LVecBase3.h"
#include "pta_LVecBase4.h"
#include "weakNodePath.h"
#include "weakPointerTo.h"
#include "transformState.h"

class Character;

/**
 * A node that represents a single eyeball of some human or creature.  Can be
 * given a look target and eye origin that is used to calculate U/V texture
 * matrices for the eyeball shader.
 */
class EXPCL_PANDA_ANIM EyeballNode final : public PandaNode {
PUBLISHED:
  EyeballNode(const std::string &name, Character *character, int parent_joint);
  EyeballNode(const EyeballNode &copy);

  INLINE void set_character(Character *character, int parent_joint);
  INLINE int get_parent_joint() const;
  INLINE Character *get_character() const;

  INLINE void set_eye_offset(const LPoint3 &offset);
  INLINE const TransformState *get_eye_offset() const;

  INLINE void set_view_target(NodePath node, const LPoint3 &offset);

  INLINE void set_eye_shift(const LVector3 &shift);
  INLINE void set_z_offset(PN_stdfloat offset);
  INLINE void set_radius(PN_stdfloat radius);
  INLINE void set_iris_scale(PN_stdfloat scale);
  INLINE void set_eye_size(PN_stdfloat size);

  INLINE void set_debug_enabled(bool enable);

public:
  virtual PandaNode *make_copy() const override;
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data) override;
  virtual bool is_renderable() const override;
  virtual bool safe_to_flatten() const override;
  virtual bool safe_to_combine() const override;

private:
  WPT(Character) _character;
  int _parent_joint;

  // Offset of eye from parent joint.
  CPT(TransformState) _eye_offset;

  // World-space position of view target -- what the eye should look at.
  WeakNodePath _view_target;
  CPT(TransformState) _view_offset;

  // This is what gets calculated and passed to the shader.

  // The world-space position of the eye.
  PTA_LVecBase3 _eye_origin;

  // U/V texture projection matrices.
  PTA_LVecBase4 _iris_projection_u;
  PTA_LVecBase4 _iris_projection_v;

  LVector3 _eye_shift;

  PN_stdfloat _z_offset;
  PN_stdfloat _radius;
  PN_stdfloat _iris_scale;
  PN_stdfloat _eye_size;

  bool _debug_enabled;

  int _last_update_frame;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "EyeballNode",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "eyeballNode.I"

#endif // EYEBALLNODE_H
