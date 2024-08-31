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
#include "materialParamTexture.h"
#include "pointerTo.h"
#include "qpLight.h"
#include "bitArray.h"
#include "pvector.h"

class ParticleSystem2;

/**
 *
 */
class EXPCL_PANDA_PARTICLESYSTEM2 ParticleRenderer2 : public TypedWritableReferenceCount {
PUBLISHED:
  ParticleRenderer2() = default;

  virtual PT(ParticleRenderer2) make_copy() const=0;

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
  SpriteParticleRenderer2(const SpriteParticleRenderer2 &copy);

  virtual PT(ParticleRenderer2) make_copy() const override;

  void set_render_state(const RenderState *state);
  void set_fit_animations_to_particle_lifespan(bool flag);
  void set_animation_play_rate(PN_stdfloat rate);

  void set_trail(bool flag);
  void set_trail_length_fade_in_time(PN_stdfloat time);
  void set_trail_max_length(PN_stdfloat length);
  void set_trail_min_length(PN_stdfloat length);

  INLINE NodePath get_geom_np() const;
  INLINE GeomNode *get_geom_node() const;
  INLINE const RenderState *get_render_state() const;
  INLINE bool get_fit_animations_to_particle_lifespan() const;
  INLINE PN_stdfloat get_animation_play_rate() const;

public:
  virtual void initialize(const NodePath &parent, ParticleSystem2 *system) override;
  virtual void update(ParticleSystem2 *system) override;
  virtual void shutdown(ParticleSystem2 *system) override;

  virtual void write_datagram(BamWriter *manager, Datagram &me) override;
  virtual void fillin(DatagramIterator &scan, BamReader *manager) override;
  virtual int complete_pointers(TypedWritable **p_list, BamReader *manager) override;
  static void register_with_read_factory();
  static TypedWritable *make_from_bam(const FactoryParams &params);

private:
  NodePath _geom_np;
  PT(GeomNode) _geom_node;
  PT(GeomVertexData) _vdata;
  PT(GeomPoints) _prim;

  bool _is_animated;
  bool _rgb_modulated_by_alpha;
  CPT(RenderState) _render_state;
  bool _computed_lighting;

  bool _fit_anim_to_particle_lifespan;
  PN_stdfloat _anim_play_rate;
  PT(MaterialParamTexture) _sprite_base_texture;

  // Trail parameters.
  bool _trail_enable;
  PN_stdfloat _trail_length_fade_in_time;
  PN_stdfloat _trail_max_length;
  PN_stdfloat _trail_min_length;

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

/**
 * Very fancy renderer that emits a dynamic light for each particle.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 LightParticleRenderer : public ParticleRenderer2 {
PUBLISHED:
  LightParticleRenderer() = default;
  LightParticleRenderer(const LightParticleRenderer &copy);

  virtual PT(ParticleRenderer2) make_copy() const override;

  INLINE void set_light_intensity_scale(PN_stdfloat scale) { _light_intensity_scale = scale; }
  INLINE PN_stdfloat get_light_intensity_scale() const { return _light_intensity_scale; }

  INLINE void set_light_atten_radius(PN_stdfloat radius) { _light_atten_radius = radius; }
  INLINE PN_stdfloat get_light_atten_radius() const { return _light_atten_radius; }

public:
  virtual void initialize(const NodePath &parent, ParticleSystem2 *system) override;
  virtual void update(ParticleSystem2 *system) override;
  virtual void shutdown(ParticleSystem2 *system) override;

public:
  virtual void write_datagram(BamWriter *manager, Datagram &me) override;
  virtual void fillin(DatagramIterator &scan, BamReader *manager) override;
  static void register_with_read_factory();
  static TypedWritable *make_from_bam(const FactoryParams &params);

private:
  pvector<qpLight> _particle_lights;
  BitArray _last_p_states;
  PN_stdfloat _light_intensity_scale = 1.0f;
  PN_stdfloat _light_atten_radius = 1.0f;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ParticleRenderer2::init_type();
    register_type(_type_handle, "LightParticleRenderer",
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
