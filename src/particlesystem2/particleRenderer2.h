/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file particleRenderer2.h
 * @author brian
 * @date 2022-04-04
 */

#ifndef PARTICLERENDERER2_H
#define PARTICLERENDERER2_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "nodePath.h"
#include "geomNode.h"
#include "geomVertexData.h"
#include "geomVertexArrayData.h"
#include "geomPoints.h"

class ParticleSystem2;

/**
 *
 */
class EXPCL_PANDA_PARTICLESYSTEM2 ParticleRenderer2 : public TypedWritableReferenceCount {
PUBLISHED:
  ParticleRenderer2() = default;

public:
  virtual void initialize(const NodePath &parent, ParticleSystem2 *system)=0;
  virtual void update(ParticleSystem2 *system)=0;
  virtual void shutdown(ParticleSystem2 *system)=0;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "ParticleRenderer2",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

/**
 *
 */
class EXPCL_PANDA_PARTICLESYSTEM2 SpriteParticleRenderer2 : public ParticleRenderer2 {
PUBLISHED:
  SpriteParticleRenderer2();

  void set_render_state(const RenderState *state);

public:
  virtual void initialize(const NodePath &parent, ParticleSystem2 *system) override;
  virtual void update(ParticleSystem2 *system) override;
  virtual void shutdown(ParticleSystem2 *system) override;

private:
  NodePath _geom_np;
  PT(GeomNode) _geom_node;
  PT(GeomVertexData) _vdata;
  PT(GeomPoints) _prim;

  CPT(RenderState) _render_state;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ParticleRenderer2::init_type();
    register_type(_type_handle, "SpriteParticleRenderer2",
                  ParticleRenderer2::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "particleRenderer2.I"

#endif // PARTICLERENDERER2_H
