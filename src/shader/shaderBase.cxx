/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderBase.cxx
 * @author lachbr
 * @date 2020-10-16
 */

#include "shaderBase.h"
#include "shaderManager.h"
#include "renderState.h"
#include "fogAttrib.h"
#include "clipPlaneAttrib.h"
#include "alphaTestAttrib.h"
#include "shaderAttrib.h"
#include "lightAttrib.h"
#include "cascadeLight.h"
#include "transparencyAttrib.h"
#include "geomVertexAnimationSpec.h"

TypeHandle ShaderBase::_type_handle;

/**
 * Registers a shader instance with the shader manager.
 */
void ShaderBase::
register_shader(ShaderBase *shader) {
  ShaderManager *mgr = ShaderManager::get_global_ptr();
  mgr->register_shader(shader);
}

/**
 * Sets up #defines for hardware skinning.
 */
bool ShaderBase::
add_hardware_skinning(const GeomVertexAnimationSpec &anim_spec) {
  // Hardware skinning?
  if (anim_spec.get_animation_type() == GeomEnums::AT_hardware &&
      anim_spec.get_num_transforms() > 0) {
	  set_vertex_shader_define("HARDWARE_SKINNING");
    int num_transforms;
    if (anim_spec.get_indexed_transforms()) {
      num_transforms = 120;
    } else {
      num_transforms = anim_spec.get_num_transforms();
    }
    set_vertex_shader_define("NUM_TRANSFORMS", num_transforms);

    if (anim_spec.get_indexed_transforms()) {
			set_vertex_shader_define("INDEXED_TRANSFORMS");
    }

    return true;
  }

  return false;
}

/**
 * Sets up appropriate #defines to enable fogging.
 */
bool ShaderBase::
add_fog(const RenderState *state) {
  const FogAttrib *fa;
	state->get_attrib_def(fa);

  // Check for fog.
  if (!fa->is_off()) {
    set_pixel_shader_define("FOG", (int)fa->get_fog()->get_mode());
    return true;
  }

	return false;
}

/**
 * Sets up appropriate #defines to enable clip planes.
 */
bool ShaderBase::
add_clip_planes(const RenderState *state) {
  // Check for clip planes.
  const ClipPlaneAttrib *clip_plane;
  state->get_attrib_def(clip_plane);

  set_pixel_shader_define("NUM_CLIP_PLANES", clip_plane->get_num_on_planes());

  return clip_plane->get_num_on_planes() > 0;
}

/**
 * Sets up appropriate #defines to enable alpha testing.
 */
bool ShaderBase::
add_alpha_test(const RenderState *state) {
  const AlphaTestAttrib *alpha_test;
	state->get_attrib_def(alpha_test);

	if (alpha_test->get_mode() != RenderAttrib::M_none &&
		  alpha_test->get_mode() != RenderAttrib::M_always) {
		// Subsume the alpha test in our shader.
		set_pixel_shader_define("ALPHA_TEST", alpha_test->get_mode());
		set_pixel_shader_define("ALPHA_TEST_REF", alpha_test->get_reference_alpha());

		set_flags(ShaderAttrib::F_subsume_alpha_test);

		return true;
	}

	return false;
}

/**
 * Sets up appropriate #defines to enable cascaded shadow mapping.
 */
bool ShaderBase::
add_csm(const RenderState *state) {
  const LightAttrib *lattr;
  state->get_attrib_def(lattr);

  size_t num_lights = lattr->get_num_non_ambient_lights();
  for (size_t i = 0; i < num_lights; i++) {
    NodePath np = lattr->get_on_light(i);
    if (np.is_empty()) {
      continue;
    }
    if (!np.node()->is_of_type(CascadeLight::get_class_type())) {
      continue;
    }

    // Go with the first one in there.  There really shouldn't be more than
    // one cascaded light in the same scene, that's just dumb.

    CascadeLight *clight = DCAST(CascadeLight, np.node());

    if (!clight->is_shadow_caster()) {
      // Not casting shadows, though.
      continue;
    }

    PN_stdfloat texel_size = 1.0 / clight->get_shadow_buffer_size()[0];

    set_vertex_shader_define("HAS_SHADOW_SUNLIGHT");
    set_vertex_shader_define("PSSM_SPLITS", clight->get_num_cascades());
    set_vertex_shader_define("SHADOW_TEXEL_SIZE", texel_size);
    set_vertex_shader_define("NORMAL_OFFSET_SCALE", clight->get_normal_offset_scale());
    if (clight->get_normal_offset_uv_space()) {
      set_vertex_shader_define("NORMAL_OFFSET_UV_SPACE");
    }
    // The vertex shader needs to know the index of the cascaded light.
    set_vertex_shader_define("PSSM_LIGHT_ID", i);

    set_pixel_shader_define("HAS_SHADOW_SUNLIGHT");
    set_pixel_shader_define("PSSM_SPLITS", clight->get_num_cascades());
    set_pixel_shader_define("DEPTH_BIAS", clight->get_depth_bias());
    set_pixel_shader_define("SHADOW_TEXEL_SIZE", texel_size);
    set_pixel_shader_define("SHADOW_BLUR", texel_size * clight->get_softness_factor());

    return true;
  }

  return false;
}

/**
 * Sets up appropriate #defines to enable transparency.
 */
bool ShaderBase::
add_transparency(const RenderState *state) {
  const TransparencyAttrib *ta;
  state->get_attrib_def(ta);

  if (ta->get_mode() != TransparencyAttrib::M_none) {
    set_pixel_shader_define("TRANSPARENT");
    return true;
  }

  return false;
}
