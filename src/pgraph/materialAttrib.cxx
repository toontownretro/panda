/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialAttrib.cxx
 * @author lachbr
 * @date 2021-03-21
 */

#include "materialAttrib.h"
#include "virtualFileSystem.h"
#include "materialPool.h"
#include "config_pgraph.h"

#include "transparencyAttrib.h"
#include "colorAttrib.h"
#include "colorScaleAttrib.h"
#include "depthTestAttrib.h"
#include "depthWriteAttrib.h"
#include "alphaTestAttrib.h"
#include "renderModeAttrib.h"
#include "colorBlendAttrib.h"
#include "lightAttrib.h"
#include "fogAttrib.h"
#include "cullBinAttrib.h"
#include "cullFaceAttrib.h"

TypeHandle MaterialAttrib::_type_handle;
int MaterialAttrib::_attrib_slot;

/**
 * Constructs the RenderState object that contains the attributes modified by
 * the material itself, such as transparency, color scale, etc.
 */
void MaterialAttrib::
create_modifier_state() {
  nassertv(_material != nullptr);

  _modifier_state = RenderState::make_empty();
  if (_material->_attrib_flags & Material::F_transparency) {
    _modifier_state = _modifier_state->set_attrib(
      TransparencyAttrib::make((TransparencyAttrib::Mode)_material->_transparency_mode));
  }
  if (_material->_attrib_flags & Material::F_color) {
    _modifier_state = _modifier_state->set_attrib(
      ColorAttrib::make_flat(_material->_color));
  }
  if (_material->_attrib_flags & Material::F_color_scale) {
    _modifier_state = _modifier_state->set_attrib(
      ColorScaleAttrib::make(_material->_color_scale));
  }
  if (_material->_attrib_flags & Material::F_alpha_test) {
    _modifier_state = _modifier_state->set_attrib(
      AlphaTestAttrib::make((RenderAttrib::PandaCompareFunc)_material->_alpha_test_mode, _material->_alpha_test_ref));
  }
  if (_material->_attrib_flags & Material::F_cull_face) {
    _modifier_state = _modifier_state->set_attrib(
      CullFaceAttrib::make((CullFaceAttrib::Mode)_material->_cull_face_mode));
  }
  if (_material->_attrib_flags & Material::F_depth_write) {
    _modifier_state = _modifier_state->set_attrib(
      DepthWriteAttrib::make((DepthWriteAttrib::Mode)_material->_depth_write));
  }
  if (_material->_attrib_flags & Material::F_depth_test) {
    _modifier_state = _modifier_state->set_attrib(
      DepthTestAttrib::make((RenderAttrib::PandaCompareFunc)_material->_depth_test_mode));
  }
  if (_material->_attrib_flags & Material::F_light) {
    if (_material->_light_off) {
      _modifier_state = _modifier_state->set_attrib(
        LightAttrib::make_all_off());
    }
  }
  if (_material->_attrib_flags & Material::F_fog) {
    if (_material->_fog_off) {
      _modifier_state = _modifier_state->set_attrib(
        FogAttrib::make_off());
    }
  }
  if (_material->_attrib_flags & Material::F_bin) {
    _modifier_state = _modifier_state->set_attrib(
      CullBinAttrib::make(_material->_bin_name, _material->_bin_sort));
  }
  if (_material->_attrib_flags & Material::F_render_mode) {
    _modifier_state = _modifier_state->set_attrib(
      RenderModeAttrib::make((RenderModeAttrib::Mode)_material->_render_mode));
  }
  if (_material->_attrib_flags & Material::F_color_blend) {
    switch (_material->_color_blend_mode) {
    case 0:
      _modifier_state = _modifier_state->set_attrib(
        ColorBlendAttrib::make_off());
      break;
    case 1:
      // Modulate color blend.
      _modifier_state = _modifier_state->set_attrib(
        ColorBlendAttrib::make(ColorBlendAttrib::M_add, ColorBlendAttrib::O_fbuffer_color, ColorBlendAttrib::O_incoming_color));
      break;
    case 2:
      // Additive color blend.
      _modifier_state = _modifier_state->set_attrib(
        ColorBlendAttrib::make(ColorBlendAttrib::M_add, ColorBlendAttrib::O_one, ColorBlendAttrib::O_one));
      break;
    default:
      break;
    }
  }
}

/**
 * Returns a new MaterialAttrib that contains no material.
 */
CPT(RenderAttrib) MaterialAttrib::
make_off() {
  MaterialAttrib *attr = new MaterialAttrib;
  attr->_is_off = true;

  return return_new(attr);
}

/**
 * Returns a new MaterialAttrib that references the indicated Material.
 */
CPT(RenderAttrib) MaterialAttrib::
make(Material *material) {
  MaterialAttrib *attr = new MaterialAttrib;
  attr->_material = material;

  return return_new(attr);
}

/**
 * Returns the default MaterialAttrib.
 */
CPT(RenderAttrib) MaterialAttrib::
make_default() {
  return RenderAttribRegistry::get_global_ptr()->get_slot_default(_attrib_slot);
}

/**
 *
 */
int MaterialAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const MaterialAttrib *that = (const MaterialAttrib *)other;

  if (_material != that->_material) {
    return _material < that->_material ? -1 : 1;
  }

  return 0;
}

/**
 *
 */
size_t MaterialAttrib::
get_hash_impl() const {
  return pointer_hash::add_hash(0, _material);
}

/**
 *
 */
void MaterialAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 *
 */
void MaterialAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);
  dg.add_bool(_is_off);
  manager->write_pointer(dg, _material);
}

/**
 *
 */
TypedWritable *MaterialAttrib::
make_from_bam(const FactoryParams &params) {
  MaterialAttrib *attr = new MaterialAttrib;
  DatagramIterator scan;
  BamReader *manager;
  parse_params(params, scan, manager);

  attr->fillin(scan, manager);
  return attr;
}

/**
 *
 */
void MaterialAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);
  _is_off = scan.get_bool();
  manager->read_pointer(scan);
}

/**
 *
 */
int MaterialAttrib::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = RenderAttrib::complete_pointers(p_list, manager);

  Material *mat;
  DCAST_INTO_R(mat, p_list[pi++], pi);
  _material = mat;

  return pi;
}
