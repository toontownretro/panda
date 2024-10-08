/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lightLensNode.cxx
 * @author drose
 * @date 2002-03-26
 */

#include "config_pgraphnodes.h"
#include "lightLensNode.h"
#include "bamWriter.h"
#include "bamReader.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "renderState.h"
#include "cullFaceAttrib.h"
#include "colorWriteAttrib.h"
#include "cullBinAttrib.h"
#include "colorBlendAttrib.h"
#include "antialiasAttrib.h"
#include "colorAttrib.h"
#include "colorScaleAttrib.h"
#include "fogAttrib.h"
#include "shaderAttrib.h"
#include "lightAttrib.h"
#include "graphicsStateGuardianBase.h"
#include "depthTestAttrib.h"
#include "depthOffsetAttrib.h"
#include "depthBiasAttrib.h"

TypeHandle LightLensNode::_type_handle;

/**
 *
 */
LightLensNode::
LightLensNode(const std::string &name, Lens *lens) :
  Camera(name, lens),
  _used_by_auto_shader(false),
  _attrib_count(0)
{
  set_active(false);
  _shadow_caster = false;

  if (shadow_map_size.get_num_words() == 1) {
    _sb_size.set(shadow_map_size[0], shadow_map_size[0]);
  } else if (shadow_map_size.get_num_words() >= 2) {
    _sb_size.set(shadow_map_size[0], shadow_map_size[1]);
  } else {
    _sb_size.set(512, 512);
  }

  _sb_sort = shadow_buffer_sort;

  _depth_bias = shadow_depth_bias;
  _normal_offset_scale = shadow_normal_offset_scale;
  _softness_factor = shadow_softness_factor;
  _normal_offset_uv_space = shadow_normal_offset_uv_space;

  // Turn off lots of state that we don't need in the shadow render pass.
  // This helps speed up the pass because there will be fewer state changes.
  CPT(RenderState) state = RenderState::make_empty();
  state = state->set_attrib(AntialiasAttrib::make(AntialiasAttrib::M_off), 100);
  state = state->set_attrib(ColorAttrib::make_off(), 100);
  state = state->set_attrib(ColorBlendAttrib::make_off(), 100);
  state = state->set_attrib(ColorScaleAttrib::make_off(), 100);
  state = state->set_attrib(ColorWriteAttrib::make(ColorWriteAttrib::C_off), 100);
  state = state->set_attrib(CullBinAttrib::make_default(), 100);
  // Backface culling helps eliminate artifacts.
  state = state->set_attrib(CullFaceAttrib::make_reverse());
  state = state->set_attrib(FogAttrib::make_off(), 100);
  state = state->set_attrib(DepthOffsetAttrib::make(0), 100);
  state = state->set_attrib(DepthBiasAttrib::make(0.0f, 0.0f, 0.0f), 100);
  // Render it using the depth-only shader.
  state = state->set_attrib(ShaderAttrib::make("Depth"), 100);

  set_initial_state(state);
}

/**
 *
 */
LightLensNode::
~LightLensNode() {
  set_active(false);
  clear_shadow_buffers();

  // If this triggers, the number of attrib_ref() didn't match the number of
  // attrib_unref() calls, probably indicating a bug in LightAttrib.
  nassertv(AtomicAdjust::get(_attrib_count) == 0);
}

/**
 *
 */
LightLensNode::
LightLensNode(const LightLensNode &copy) :
  Light(copy),
  Camera(copy),
  _sb_size(copy._sb_size),
  _shadow_caster(copy._shadow_caster),
  _sb_sort(copy._sb_sort),
  _used_by_auto_shader(false),
  _attrib_count(0)
{
  if (_shadow_caster) {
    setup_shadow_map();
  }
}

/**
 * Sets the flag indicating whether this light should cast shadows or not.
 * This is the variant without buffer size, meaning that the current buffer
 * size will be kept (512x512 is the default). Note that enabling shadows will
 * require the shader generator to be enabled on the scene.
 */
void LightLensNode::
set_shadow_caster(bool caster) {
  if (_shadow_caster && !caster) {
    clear_shadow_buffers();
  }
  if (_shadow_caster != caster && _used_by_auto_shader) {
    // Make sure any shaders using this light are regenerated.
    GraphicsStateGuardianBase::mark_rehash_generated_shaders();
  }
  _shadow_caster = caster;
  set_active(caster);
  if (caster) {
    setup_shadow_map();
    set_light_state();
  }
}

/**
 * Sets the flag indicating whether this light should cast shadows or not.
 * The xsize and ysize parameters specify the size of the shadow buffer that
 * will be set up.  This flavor uses the sort value already set on the light,
 * which will be the default value specified by the shadow-buffer-sort PRC
 * variable if you have not already set a new sort value on the light.
 */
void LightLensNode::
set_shadow_caster(bool caster, int buffer_xsize, int buffer_ysize) {
  set_shadow_caster(caster, buffer_xsize, buffer_ysize, _sb_sort);
}

/**
 * Sets the flag indicating whether this light should cast shadows or not.
 * The xsize and ysize parameters specify the size of the shadow buffer that
 * will be set up, the sort parameter specifies the sort.  Note that enabling
 * shadows will require the shader generator to be enabled on the scene.
 */
void LightLensNode::
set_shadow_caster(bool caster, int buffer_xsize, int buffer_ysize, int buffer_sort) {
  if ((_shadow_caster && !caster) || buffer_xsize != _sb_size[0] || buffer_ysize != _sb_size[1]) {
    clear_shadow_buffers();
  }
  if (_shadow_caster != caster && _used_by_auto_shader) {
    // Make sure any shaders using this light are regenerated.
    GraphicsStateGuardianBase::mark_rehash_generated_shaders();
  }
  _shadow_caster = caster;
  _sb_size.set(buffer_xsize, buffer_ysize);

  if (buffer_sort != _sb_sort) {
    ShadowBuffers::iterator it;
    for(it = _sbuffers.begin(); it != _sbuffers.end(); ++it) {
      (*it).second->set_sort(buffer_sort);
    }
    _sb_sort = buffer_sort;
  }
  set_active(caster);
  if (caster) {
    setup_shadow_map();
    set_light_state();
  }
}

/**
 * Clears the shadow buffers, meaning they will be automatically recreated
 * when the Shader Generator needs them.
 */
void LightLensNode::
clear_shadow_buffers() {
  if (_shadow_map) {
    // Clear it to all ones, so that any shaders that might still be using
    // it will see the shadows being disabled.
    _shadow_map->clear_image();
  }

  ShadowBuffers::iterator it;
  for(it = _sbuffers.begin(); it != _sbuffers.end(); ++it) {
    (*it).first->remove_window((*it).second);
  }
  _sbuffers.clear();
}

/**
 * Creates the shadow map texture.  Can be overridden.
 */
void LightLensNode::
setup_shadow_map() {
  if (_shadow_map != nullptr &&
      _shadow_map->get_x_size() == _sb_size[0] &&
      _shadow_map->get_y_size() == _sb_size[1]) {
    // Nothing to do.
    return;
  }

  if (_shadow_map == nullptr) {
    _shadow_map = new Texture(get_name());
  }

  _shadow_map->setup_2d_texture(_sb_size[0], _sb_size[1], Texture::T_unsigned_byte, Texture::F_depth_component);
  _shadow_map->set_clear_color(LColor(1));
  _shadow_map->set_wrap_u(SamplerState::WM_border_color);
  _shadow_map->set_wrap_v(SamplerState::WM_border_color);
  _shadow_map->set_border_color(LColor(1));
  _shadow_map->set_minfilter(SamplerState::FT_nearest);
  _shadow_map->set_magfilter(SamplerState::FT_nearest);
}

/**
 * Ensures that the camera's initial state includes the light.
 */
void LightLensNode::
set_light_state() {
  // Make sure the shader knows the light that we are rendering shadows for.
  CPT(RenderState) state = get_initial_state();
  CPT(RenderAttrib) light_attr = LightAttrib::make();
  light_attr = DCAST(LightAttrib, light_attr)->add_on_light(NodePath(this));
  state = state->set_attrib(light_attr, 100);
  set_initial_state(state);
}

/**
 * This is called when the light is added to a LightAttrib.
 */
void LightLensNode::
attrib_ref() {
  AtomicAdjust::inc(_attrib_count);
}

/**
 * This is called when the light is removed from a LightAttrib.
 */
void LightLensNode::
attrib_unref() {
  // When it is removed from the last LightAttrib, destroy the shadow buffers.
  // This is necessary to break the circular reference that the buffer holds
  // on this node, via the display region's camera.
  if (!AtomicAdjust::dec(_attrib_count)) {
    clear_shadow_buffers();
  }
}

/**
 * Returns the Light object upcast to a PandaNode.
 */
PandaNode *LightLensNode::
as_node() {
  return this;
}

/**
 * Cross-casts the node to a Light pointer, if it is one of the four kinds of
 * Light nodes, or returns NULL if it is not.
 */
Light *LightLensNode::
as_light() {
  return this;
}

/**
 *
 */
void LightLensNode::
output(std::ostream &out) const {
  LensNode::output(out);
}

/**
 *
 */
void LightLensNode::
write(std::ostream &out, int indent_level) const {
  LensNode::write(out, indent_level);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void LightLensNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  Camera::write_datagram(manager, dg);
  Light::write_datagram(manager, dg);

  dg.add_bool(_shadow_caster);
  dg.add_int32(_sb_size[0]);
  dg.add_int32(_sb_size[1]);
  dg.add_int32(_sb_sort);
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new LightLensNode.
 */
void LightLensNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  Camera::fillin(scan, manager);
  Light::fillin(scan, manager);

  bool shadow_caster = scan.get_bool();
  int sb_xsize = scan.get_int32();
  int sb_ysize = scan.get_int32();
  int sb_sort = scan.get_int32();
  set_shadow_caster(shadow_caster, sb_xsize, sb_ysize, sb_sort);
}
