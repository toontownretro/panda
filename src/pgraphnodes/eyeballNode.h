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

/**
 * A node that represents a single eyeball of some human or creature.  Can be
 * given a look target and eye origin that is used to calculate U/V texture
 * matrices for the eyeball shader.
 */
class EXPCL_PANDA_PGRAPHNODES EyeballNode : public PandaNode {
PUBLISHED:
  EyeballNode(const std::string &name);
  EyeballNode(const EyeballNode &copy);

  INLINE void set_view_target(const LPoint3 &target);

  INLINE void set_eye_shift(const LVector3 &shift);
  INLINE void set_z_offset(PN_stdfloat offset);
  INLINE void set_radius(PN_stdfloat radius);
  INLINE void set_iris_scale(PN_stdfloat scale);
  INLINE void set_eye_size(PN_stdfloat size);

  INLINE void set_debug_enabled(bool enable);

public:
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data) override;
  virtual bool is_renderable() const;
  virtual bool safe_to_flatten() const;
  virtual bool safe_to_combine() const;

private:
  // World-space position of view target -- what the eye should look at.
  LPoint3 _view_target;

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
