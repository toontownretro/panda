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
#include "materialParamColor.h"
#include "shaderManager.h"
#include "texturePool.h"
#include "shaderAttrib.h"
#include "textureAttrib.h"
#include "geomVertexAnimationSpec.h"
#include "fogAttrib.h"
#include "fog.h"

TypeHandle EyeRefractShader::_type_handle;

/**
 * Synthesizes a shader for a given render state.
 */
void EyeRefractShader::
generate_shader(GraphicsStateGuardianBase *gsg,
                const RenderState *state,
                Material *material,
                const GeomVertexAnimationSpec &anim_spec) {

  static const CPT_InternalName IN_SKINNING("SKINNING");
  static const CPT_InternalName IN_DIRECT_LIGHT("DIRECT_LIGHT");
  static const CPT_InternalName IN_AMBIENT_LIGHT("AMBIENT_LIGHT");
  static const CPT_InternalName IN_RAYTRACESPHERE("RAYTRACESPHERE");
  static const CPT_InternalName IN_RAYTRACEDISCARD("RAYTRACEDISCARD");
  static const CPT_InternalName IN_FOG("FOG");

  static const CPT_InternalName IN_NUM_LIGHTS("NUM_LIGHTS");
  static const CPT_InternalName IN_FOG_MODE("FOG_MODE");

  set_language(Shader::SL_GLSL);

  set_vertex_shader("shadersnew/eyes.vert.sho.pz");
  set_pixel_shader("shadersnew/eyes.frag.sho.pz");

  if (anim_spec.get_animation_type() == GeomEnums::AT_hardware &&
      anim_spec.get_num_transforms() > 0) {
    // Doing skinning in the vertex shader.
    set_vertex_shader_combo(IN_SKINNING, 1);
  }

  // Check for fog.
  const FogAttrib *fa;
  if (state->get_attrib(fa)) {
    Fog *fog = fa->get_fog();
    if (fog != nullptr) {
      set_pixel_shader_combo(IN_FOG, 1);
      set_spec_constant(IN_FOG_MODE, (int)fog->get_mode());
    }
  }

  EyeRefractMaterial *eye_mat = DCAST(EyeRefractMaterial, material);

  // Break out the lights by type.
  const LightAttrib *la;
  state->get_attrib_def(la);
  size_t num_lights = 0;
  size_t num_ambient_lights = 0;
  if (!la->has_all_off()) {
    num_lights = la->get_num_non_ambient_lights();
    num_ambient_lights = la->get_num_on_lights() - num_lights;

    const ShaderAttrib *sa;
    state->get_attrib_def(sa);
    if (sa->has_shader_input("ambientProbe")) {
      // SH ambient probe.
      set_pixel_shader_combo(IN_AMBIENT_LIGHT, 2);

    } else if (num_ambient_lights != 0) {
      // Flat ambient.
      set_pixel_shader_combo(IN_AMBIENT_LIGHT, 1);
    }
  }

  if (num_lights > 0) {
    // We have one or more direct local light sources.
    set_pixel_shader_combo(IN_DIRECT_LIGHT, 1);
    set_spec_constant(IN_NUM_LIGHTS, (int)num_lights);
  }

  MaterialParamBase *param = eye_mat->get_param("iris_texture");
  if (param && param->is_of_type(MaterialParamTexture::get_class_type())) {
    set_input(ShaderInput("irisSampler", DCAST(MaterialParamTexture, param)->get_value()));
  }

  param = eye_mat->get_param("cornea_texture");
  if (param && param->is_of_type(MaterialParamTexture::get_class_type())) {
    set_input(ShaderInput("corneaSampler", DCAST(MaterialParamTexture, param)->get_value()));
  }

  param = eye_mat->get_param("ambient_occl_texture");
  if (param && param->is_of_type(MaterialParamTexture::get_class_type())) {
    set_input(ShaderInput("eyeAmbientOcclSampler", DCAST(MaterialParamTexture, param)->get_value()));
  }

  param = eye_mat->get_param("env_map");
  if (param && param->is_of_type(MaterialParamTexture::get_class_type())) {
    set_input(ShaderInput("eyeReflectionCubemapSampler", DCAST(MaterialParamTexture, param)->get_value()));
  } else {
    set_input(ShaderInput("eyeReflectionCubemapSampler", ShaderManager::get_global_ptr()->get_default_cube_map()));
  }

  //Texture *envmap_tex = nullptr;
  //const TextureAttrib *ta;
  //state->get_attrib_def(ta);
  //for (int i = 0; i < ta->get_num_on_stages(); ++i) {
  //  TextureStage *stage = ta->get_on_stage(i);
  //  if (stage->get_name() == "envmap") {
  //    envmap_tex = ta->get_on_texture(stage);
  //    break;
  //  }
  //}
  //if (envmap_tex == nullptr) {
  //  envmap_tex = ShaderManager::get_global_ptr()->get_default_cube_map();
  //}

  //set_input(ShaderInput("eyeReflectionCubemapSampler", envmap_tex));

  param = eye_mat->get_param("lightwarp_texture");
  if (param && param->is_of_type(MaterialParamTexture::get_class_type())) {
    set_input(ShaderInput("lightwarpSampler", DCAST(MaterialParamTexture, param)->get_value()));
  }

  PN_stdfloat dilation = 0.5;
  PN_stdfloat glossiness = 1;
  PN_stdfloat average_ambient = 1;
  PN_stdfloat cornea_bump_strength = 1;
  param = eye_mat->get_param("dilation");
  if (param) {
    dilation = DCAST(MaterialParamFloat, param)->get_value();
  }
  param = eye_mat->get_param("glossiness");
  if (param) {
    glossiness = DCAST(MaterialParamFloat, param)->get_value();
  }
  param = eye_mat->get_param("cornea_bump_strength");
  if (param) {
    cornea_bump_strength = DCAST(MaterialParamFloat, param)->get_value();
  }
  LVector4 packed_const_0(dilation, glossiness, average_ambient, cornea_bump_strength);
  set_input(ShaderInput("packedConst0", packed_const_0));

  PN_stdfloat eyeball_radius = 0;
  PN_stdfloat parallax_strength = 1;
  param = eye_mat->get_param("eyeball_radius");
  if (param) {
    eyeball_radius = DCAST(MaterialParamFloat, param)->get_value();
  }
  param = eye_mat->get_param("parallax_strength");
  if (param) {
    parallax_strength = DCAST(MaterialParamFloat, param)->get_value();
  }
  LVector4 packed_const_1(0, eyeball_radius, 0, parallax_strength);
  set_input(ShaderInput("packedConst1", packed_const_1));

  LVector3 ambient_occl_color(1);
  param = eye_mat->get_param("ambient_occl_color");
  if (param) {
    ambient_occl_color = DCAST(MaterialParamColor, param)->get_value().get_xyz();
  }
  set_input(ShaderInput("ambientOcclColor", ambient_occl_color));

  // Eye origin and iris U/V projection inputs get calculated and applied from
  // the eye node.

}
