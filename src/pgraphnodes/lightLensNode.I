/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lightLensNode.I
 * @author drose
 * @date 2002-03-26
 */

/**
 * Returns whether this light is configured to cast shadows or not.
 */
INLINE bool LightLensNode::
is_shadow_caster() const {
  return _shadow_caster;
}

/**
 * Returns the sort of the shadow buffer to be created for this light source.
 */
INLINE int LightLensNode::
get_shadow_buffer_sort() const {
  return _sb_sort;
}

/**
 * Returns the size of the shadow buffer to be created for this light source.
 */
INLINE LVecBase2i LightLensNode::
get_shadow_buffer_size() const {
  return _sb_size;
}

/**
 * Sets the size of the shadow buffer to be created for this light source.
 */
INLINE void LightLensNode::
set_shadow_buffer_size(const LVecBase2i &size) {
  if (size != _sb_size) {
    clear_shadow_buffers();
    _sb_size = size;
    setup_shadow_map();
  }
}

/**
 * Returns the offset that will be applied to depth values from the shadow map
 * when doing the depth comparison.
 */
INLINE PN_stdfloat LightLensNode::
get_depth_bias() const {
  return _depth_bias;
}

/**
 * Sets the offset that will be applied to depth values from the shadow map
 * when doing the depth comparison.
 */
INLINE void LightLensNode::
set_depth_bias(PN_stdfloat bias) {
  _depth_bias = bias;
}

/**
 * Returns how much vertex normals will affect the shadow map projection
 * offset.
 */
INLINE PN_stdfloat LightLensNode::
get_normal_offset_scale() const {
  return _normal_offset_scale;
}

/**
 * Sets how much vertex normals will affect the shadow map projection
 * offset.
 */
INLINE void LightLensNode::
set_normal_offset_scale(PN_stdfloat scale) {
  _normal_offset_scale = scale;
}

/**
 * Returns the softness factor of the shadows.
 */
INLINE PN_stdfloat LightLensNode::
get_softness_factor() const {
  return _softness_factor;
}

/**
 * Sets the softness factor of the shadows.
 */
INLINE void LightLensNode::
set_softness_factor(PN_stdfloat factor) {
  _softness_factor = factor;
}

/**
 * Returns whether or not shadow projection normal offsetting will be done in
 * UV space.
 */
INLINE bool LightLensNode::
get_normal_offset_uv_space() const {
  return _normal_offset_uv_space;
}

/**
 * Sets whether or not shadow projection normal offsetting will be done in
 * UV space.
 */
INLINE void LightLensNode::
set_normal_offset_uv_space(bool flag) {
  _normal_offset_uv_space = flag;
}

/**
 * Returns the shadow map texture for this light.
 */
INLINE Texture *LightLensNode::
get_shadow_map() const {
  return _shadow_map;
}

/**
 * Returns the buffer that has been constructed for a given GSG, or NULL if no
 * such buffer has (yet) been constructed.  This should be used for debugging
 * only, you will not need to call this normally.
 */
INLINE GraphicsOutputBase *LightLensNode::
get_shadow_buffer(GraphicsStateGuardianBase *gsg) {
  ShadowBuffers::iterator it = _sbuffers.find(gsg);
  if (it == _sbuffers.end()) {
    return nullptr;
  } else {
    return (*it).second;
  }
}

/**
 * Marks this light as having been used by the auto shader.
 */
INLINE void LightLensNode::
mark_used_by_auto_shader() const {
  _used_by_auto_shader = true;
}
