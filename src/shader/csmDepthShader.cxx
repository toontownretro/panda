/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file csmDepthShader.cxx
 * @author lachbr
 * @date 2020-10-30
 */

#include "csmDepthShader.h"
#include "renderState.h"
#include "transparencyAttrib.h"
#include "lightAttrib.h"
#include "textureAttrib.h"
#include "textureStage.h"
#include "cascadeLight.h"

TypeHandle CSMDepthShader::_type_handle;

/**
 * Synthesizes a shader for a given render state.
 */
void CSMDepthShader::
generate_shader(GraphicsStateGuardianBase *gsg,
                const RenderState *state,
                const ParamAttrib *params,
                const GeomVertexAnimationSpec &anim_spec) {

  set_language(Shader::SL_GLSL);

  set_vertex_shader("shaders/csmdepth.vert.glsl");
  set_geometry_shader("shaders/csmdepth.geom.glsl");
  set_pixel_shader("shaders/csmdepth.frag.glsl");

  // Do we have transparency?
  add_transparency(state);
  add_alpha_test(state);

  // Hardware skinning?
  add_hardware_skinning(anim_spec);

  // How about clip planes?
  if (add_clip_planes(state)) {
    set_vertex_shader_define("NEED_WORLD_POSITION");
    set_geometry_shader_define("NEED_WORLD_POSITION");
    set_pixel_shader_define("NEED_WORLD_POSITION");
  }

  // Need textures for alpha-tested shadows.
  const TextureAttrib *ta;
  state->get_attrib_def(ta);
  for (int i = 0; i < ta->get_num_on_stages(); i++) {
    TextureStage *stage = ta->get_on_stage(i);
    if (stage == TextureStage::get_default() ||
        stage->get_name() == "albedo") {
      set_pixel_shader_define("BASETEXTURE");
      set_input(ShaderInput("baseTextureSampler", ta->get_on_texture(stage)));
    }
  }

  // Find the number of cascades
  const LightAttrib *lattr;
  state->get_attrib_def(lattr);

  // Have to call this to sort the lights if they haven't been yet...
  // Why can't check_sorted() be public?
  lattr->get_num_non_ambient_lights();

  // It is set up so that the only light we have is the light we are rendering
  // shadows for, so just expect that.
  CascadeLight *clight = DCAST(CascadeLight, lattr->get_on_light(0).node());
  set_vertex_shader_define("NUM_SPLITS", clight->get_num_cascades());
  set_geometry_shader_define("MAX_VERTICES", clight->get_num_cascades() * 3);

  // Instance the geometry to each cascade.
  set_instance_count(clight->get_num_cascades());
}
