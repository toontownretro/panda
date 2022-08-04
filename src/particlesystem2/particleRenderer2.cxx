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

TypeHandle ParticleRenderer2::_type_handle;
TypeHandle SpriteParticleRenderer2::_type_handle;

/**
 *
 */
SpriteParticleRenderer2::
SpriteParticleRenderer2() :
  _render_state(RenderState::make_empty())
{
}

/**
 *
 */
void SpriteParticleRenderer2::
set_render_state(const RenderState *state) {
  _render_state = state;
}

/**
 *
 */
void SpriteParticleRenderer2::
initialize(const NodePath &parent, ParticleSystem2 *system) {
  // Setup vertex format.
  PT(GeomVertexArrayFormat) array_format;
  array_format = new GeomVertexArrayFormat
  (InternalName::get_vertex(), 3, Geom::NT_stdfloat, Geom::C_point,
    InternalName::get_color(), 4, Geom::NT_uint8, Geom::C_color);
  array_format->add_column(InternalName::get_size(), 2, Geom::NT_stdfloat, Geom::C_other);
  array_format->add_column(InternalName::get_rotate(), 1, Geom::NT_stdfloat, Geom::C_other);
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
  _geom_np = parent.attach_new_node(_geom_node);
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

  LPoint3 mins(9999999);
  LPoint3 maxs(-9999999);

  int num_alive = 0;
  for (size_t i = 0; i < system->_particles.size(); ++i) {
    const Particle *p = &system->_particles[i];
    if (!p->_alive) {
      continue;
    }

    vwriter.set_data3f(p->_pos);
    cwriter.set_data4f(p->_color);
    swriter.set_data2f(p->_scale);
    rwriter.set_data1f(p->_rotation);

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
