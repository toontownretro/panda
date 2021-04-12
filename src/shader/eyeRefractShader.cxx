/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eyeRefractShader.cxx
 * @author lachbr
 * @date 2021-03-24
 */

#include "eyeRefractShader.h"
#include "material.h"
#include "renderState.h"
#include "lightAttrib.h"
#include "materialParamTexture.h"
#include "materialParamFloat.h"
#include "materialParamBool.h"
#include "materialParamVector.h"

TypeHandle EyeRefractShader::_type_handle;

/**
 * Synthesizes a shader for a given render state.
 */
void EyeRefractShader::
generate_shader(GraphicsStateGuardianBase *gsg,
                const RenderState *state,
                Material *material,
                const GeomVertexAnimationSpec &anim_spec) {

  set_language(Shader::SL_GLSL);

  set_vertex_shader("shaders/eyes.vert.glsl");
  set_pixel_shader("shaders/eyes.frag.glsl");

  // Hardware skinning?
  add_hardware_skinning(anim_spec);

  add_hdr(state);

  add_aux_attachments(state);

  EyeRefractMaterial *eye_mat = DCAST(EyeRefractMaterial, material);

  // Break out the lights by type.
  const LightAttrib *la;
  state->get_attrib_def(la);
  size_t num_lights = la->get_num_non_ambient_lights();
  size_t num_ambient_lights = la->get_num_on_lights() - num_lights;
  if (num_ambient_lights != 0) {
    set_pixel_shader_define("AMBIENT_LIGHT");
  }
  if (num_lights > 0) {
    set_pixel_shader_define("LIGHTING");
    set_pixel_shader_define("NUM_LIGHTS", num_lights);
    set_vertex_shader_define("NUM_LIGHTS", num_lights);
  }

  add_csm(state);
  add_fog(state);

  MaterialParamBase *param = eye_mat->get_param("$iris");
  if (param && param->is_of_type(MaterialParamTexture::get_class_type())) {
    set_input(ShaderInput("irisSampler", DCAST(MaterialParamTexture, param)->get_value()));
  }

  param = eye_mat->get_param("$corneatexture");
  if (param && param->is_of_type(MaterialParamTexture::get_class_type())) {
    set_input(ShaderInput("corneaSampler", DCAST(MaterialParamTexture, param)->get_value()));
  }

  param = eye_mat->get_param("$ambientoccltexture");
  if (param && param->is_of_type(MaterialParamTexture::get_class_type())) {
    set_input(ShaderInput("eyeAmbientOcclSampler", DCAST(MaterialParamTexture, param)->get_value()));
  }

  param = eye_mat->get_param("$envmap");
  if (param && param->is_of_type(MaterialParamTexture::get_class_type())) {
    set_input(ShaderInput("eyeReflectionCubemapSampler", DCAST(MaterialParamTexture, param)->get_value()));
  }

  param = eye_mat->get_param("$lightwarptexture");
  if (param && param->is_of_type(MaterialParamTexture::get_class_type())) {
    set_input(ShaderInput("lightwarpSampler", DCAST(MaterialParamTexture, param)->get_value()));
  }

  PN_stdfloat dilation = 0.5;
  PN_stdfloat glossiness = 1;
  PN_stdfloat average_ambient = 1;
  PN_stdfloat cornea_bump_strength = 1;
  param = eye_mat->get_param("$dilation");
  if (param) {
    dilation = DCAST(MaterialParamFloat, param)->get_value();
  }
  param = eye_mat->get_param("$glossiness");
  if (param) {
    glossiness = DCAST(MaterialParamFloat, param)->get_value();
  }
  param = eye_mat->get_param("$corneabumpstrength");
  if (param) {
    cornea_bump_strength = DCAST(MaterialParamFloat, param)->get_value();
  }
  LVector4 packed_const_0(dilation, glossiness, average_ambient, cornea_bump_strength);
  set_input(ShaderInput("packedConst0", packed_const_0));

  PN_stdfloat eyeball_radius = 0;
  PN_stdfloat parallax_strength = 1;
  param = eye_mat->get_param("$eyeballradius");
  if (param) {
    eyeball_radius = DCAST(MaterialParamFloat, param)->get_value();
  }
  param = eye_mat->get_param("$parallaxstrength");
  if (param) {
    parallax_strength = DCAST(MaterialParamFloat, param)->get_value();
  }
  LVector4 packed_const_1(0, eyeball_radius, 0, parallax_strength);
  set_input(ShaderInput("packedConst1", packed_const_1));

  LVector3 ambient_occl_color(1);
  param = eye_mat->get_param("$ambientocclcolor");
  if (param) {
    ambient_occl_color = DCAST(MaterialParamVector, param)->get_value();
  }
  set_input(ShaderInput("ambientOcclColor", ambient_occl_color));

  // Eye origin and iris U/V projection inputs get calculated and applied from
  // the eye node.

}
