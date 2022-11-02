/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file particleRenderer2.cxx
 * @author brian
 * @date 2022-04-04
 */

#include "particleRenderer2.h"
#include "particleSystem2.h"
#include "geomVertexData.h"
#include "geomVertexWriter.h"
#include "geomVertexArrayFormat.h"
#include "geomVertexFormat.h"
#include "geomPoints.h"
#include "omniBoundingVolume.h"
#include "boundingBox.h"
#include "renderState.h"
#include "materialAttrib.h"
#include "material.h"
#include "materialParamBool.h"
#include "colorBlendAttrib.h"

TypeHandle ParticleRenderer2::_type_handle;
TypeHandle SpriteParticleRenderer2::_type_handle;

/**
 *
 */
SpriteParticleRenderer2::
SpriteParticleRenderer2() :
  _render_state(RenderState::make_empty()),
  _is_animated(false),
  _rgb_modulated_by_alpha(false),
  _fit_anim_to_particle_lifespan(false),
  _anim_play_rate(1.0f)
{
}

/**
 *
 */
SpriteParticleRenderer2::
SpriteParticleRenderer2(const SpriteParticleRenderer2 &copy) :
  _render_state(copy._render_state),
  _is_animated(copy._is_animated),
  _rgb_modulated_by_alpha(copy._rgb_modulated_by_alpha),
  _sprite_base_texture(copy._sprite_base_texture),
  _fit_anim_to_particle_lifespan(copy._fit_anim_to_particle_lifespan),
  _anim_play_rate(copy._anim_play_rate)
{
}

/**
 *
 */
PT(ParticleRenderer2) SpriteParticleRenderer2::
make_copy() const {
  return new SpriteParticleRenderer2(*this);
}

/**
 *
 */
void SpriteParticleRenderer2::
set_render_state(const RenderState *state) {
  _render_state = state;
}

/**
 * Specifies whether or not texture animation frame rates should be adjusted
 * so the animation ends at the same time of the particle.  If true,
 * _anim_play_rate is ignored.
 */
void SpriteParticleRenderer2::
set_fit_animations_to_particle_lifespan(bool flag) {
  _fit_anim_to_particle_lifespan = flag;
}

/**
 * Sets the play rate of texture animations.  This value is ignored
 * if _fit_anim_to_particle_lifespan is true.
 */
void SpriteParticleRenderer2::
set_animation_play_rate(PN_stdfloat rate) {
  _anim_play_rate = rate;
}

/**
 *
 */
void SpriteParticleRenderer2::
initialize(const NodePath &parent, ParticleSystem2 *system) {
  // Determine if the particle should use texture animation.
  const MaterialAttrib *mattr;
   _render_state->get_attrib_def(mattr);
  Material *mat = mattr->get_material();
  if (mat != nullptr) {
    MaterialParamTexture *base_tex_p = (MaterialParamTexture *)mat->get_param("base_texture");
    if (base_tex_p != nullptr) {
      _sprite_base_texture = base_tex_p;
      if (base_tex_p->get_num_animations() > 0) {
        _is_animated = true;
      }
    }
  }
  CPT(RenderState) state = _render_state->compose(mattr->get_modifier_state());
  if (state->has_attrib(ColorBlendAttrib::get_class_slot())) {
    // If we have an explicit color blend equation, the RGB of
    // particles is modulated by the alpha.
    _rgb_modulated_by_alpha = true;
  }

  // Setup vertex format.
  PT(GeomVertexArrayFormat) array_format;
  array_format = new GeomVertexArrayFormat
  (InternalName::get_vertex(), 3, Geom::NT_stdfloat, Geom::C_point,
    InternalName::get_color(), 4, Geom::NT_uint8, Geom::C_color);
  array_format->add_column(InternalName::get_size(), 2, Geom::NT_stdfloat, Geom::C_other);
  array_format->add_column(InternalName::get_rotate(), 1, Geom::NT_stdfloat, Geom::C_other);
  // Add the animation data column, but only if we're actually animated.
  if (_is_animated) {
    array_format->add_column(InternalName::make("anim_data"), 4, Geom::NT_stdfloat, Geom::C_other);
    array_format->add_column(InternalName::make("anim_data2"), 3, Geom::NT_stdfloat, Geom::C_other);
  }
  CPT(GeomVertexFormat) format = GeomVertexFormat::register_format
    (new GeomVertexFormat(array_format));

  // Initialize geometry.
  PT(GeomVertexData) vdata = new GeomVertexData("sprite-particles-data", format, GeomEnums::UH_dynamic);
  vdata->set_num_rows(system->_particles.size());
  _vdata = vdata;

  PT(GeomPoints) prim = new GeomPoints(GeomEnums::UH_dynamic);
  PT(Geom) geom = new Geom(vdata);
  geom->add_primitive(prim);
  //prim->add_consecutive_vertices(0, system->_particles.size());
  //_index_buffer = new GeomVertexArrayData(GeomPrimitive::get_index_format(GeomEnums::NT_uint16), GeomEnums::UH_dynamic);
  //prim->set_vertices(_index_buffer);
  _geom_node = new GeomNode("sprite-particles");
  _geom_node->add_geom(geom, _render_state);
  //_geom_node->set_bounds(new OmniBoundingVolume); // TEMPORARY
  _geom_np = system->_np.attach_new_node(_geom_node);
  _prim = prim;
}

/**
 *
 */
void SpriteParticleRenderer2::
update(ParticleSystem2 *system) {

  //{
  //  PT(GeomVertexArrayDataHandle) ihandle = _index_buffer->modify_handle();
  //  uint16_t *indices = (uint16_t *)ihandle->get_write_pointer();
  //  for (size_t i = 0; i < system->_particles.size(); ++i) {
//
  //  }
  //}

  // Update vertex buffer to contain the data for all the alive particles.
  GeomVertexWriter vwriter(_vdata, InternalName::get_vertex());
  GeomVertexWriter cwriter(_vdata, InternalName::get_color());
  GeomVertexWriter swriter(_vdata, InternalName::get_size());
  GeomVertexWriter rwriter(_vdata, InternalName::get_rotate());
  GeomVertexWriter awriter(_vdata, InternalName::make("anim_data"));
  GeomVertexWriter a2writer(_vdata, InternalName::make("anim_data2"));

  LPoint3 mins(9999999);
  LPoint3 maxs(-9999999);

  int num_alive = 0;
  for (size_t i = 0; i < system->_particles.size(); ++i) {
    const Particle *p = &system->_particles[i];
    if (!p->_alive) {
      continue;
    }

    vwriter.set_data3f(p->_pos);
    if (_rgb_modulated_by_alpha) {
      cwriter.set_data4f(p->_color * p->_color[3]);
    } else {
      cwriter.set_data4f(p->_color);
    }
    swriter.set_data2f(p->_scale);
    rwriter.set_data1f(p->_rotation);
    if (awriter.has_column() && _sprite_base_texture != nullptr) {
      // Write particle data needed to compute texture animation.
      // TODO: Texture animation data could be passed as a uniform rather
      // than per-vertex.  Would reduce memory and CPU overhead here.

      // Fit FPS of animation to particle lifespan.
      const MaterialParamTexture::AnimData *adata = _sprite_base_texture->get_animation(p->_anim_index);
      PN_stdfloat fps = adata->_fps;
      if (_fit_anim_to_particle_lifespan) {
        PN_stdfloat duration = (PN_stdfloat)adata->_num_frames / (PN_stdfloat)adata->_fps;
        fps *= duration / p->_duration;
      } else {
        fps *= _anim_play_rate;
      }

      awriter.set_data4f(p->_anim_index, fps, p->_spawn_time + system->_start_time, adata->_first_frame);
      a2writer.set_data3f(adata->_num_frames, adata->_loop, adata->_interp);
    }

    mins = mins.fmin(p->_pos - LPoint3(p->_scale[0]));
    mins = mins.fmin(p->_pos - LPoint3(p->_scale[1]));
    maxs = maxs.fmax(p->_pos + LPoint3(p->_scale[0]));
    maxs = maxs.fmax(p->_pos + LPoint3(p->_scale[1]));

    num_alive++;
  }

  // Set the primitive to render all alive particles consecutively.
  _prim->set_nonindexed_vertices(0, num_alive);

  _geom_node->set_bounds(new BoundingBox(mins, maxs));
}

/**
 *
 */
void SpriteParticleRenderer2::
shutdown(ParticleSystem2 *system) {
  _geom_node = nullptr;
  _vdata = nullptr;
  _prim = nullptr;
  if (!_geom_np.is_empty()) {
    _geom_np.remove_node();
  }
}

/**
 *
 */
void SpriteParticleRenderer2::
write_datagram(BamWriter *manager, Datagram &me) {
  manager->write_pointer(me, _render_state);
  me.add_bool(_fit_anim_to_particle_lifespan);
  me.add_stdfloat(_anim_play_rate);
}

/**
 *
 */
void SpriteParticleRenderer2::
fillin(DatagramIterator &scan, BamReader *manager) {
  manager->read_pointer(scan);
  _fit_anim_to_particle_lifespan = scan.get_bool();
  _anim_play_rate = scan.get_stdfloat();
}

/**
 *
 */
int SpriteParticleRenderer2::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = ParticleRenderer2::complete_pointers(p_list, manager);
  _render_state = DCAST(RenderState, p_list[pi++]);
  manager->finalize_now((RenderState *)_render_state.p());
  return pi;
}

/**
 *
 */
TypedWritable *SpriteParticleRenderer2::
make_from_bam(const FactoryParams &params) {
  SpriteParticleRenderer2 *obj = new SpriteParticleRenderer2;

  BamReader *manager;
  DatagramIterator scan;
  parse_params(params, scan, manager);

  obj->fillin(scan, manager);
  return obj;
}

/**
 *
 */
void SpriteParticleRenderer2::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}
