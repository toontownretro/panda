/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file standardMaterial.I
 * @author lachbr
 * @date 2021-03-06
 */

/**
 *
 */
INLINE void StandardMaterial::
set_rim_light(bool rim_light) {
  _rim_light = rim_light;
}

/**
 *
 */
INLINE bool StandardMaterial::
get_rim_light() const {
  return _rim_light;
}

/**
 *
 */
INLINE void StandardMaterial::
set_rim_light_boost(PN_stdfloat boost) {
  _rim_light_boost = boost;
}

/**
 *
 */
INLINE PN_stdfloat StandardMaterial::
get_rim_light_boost() const {
  return _rim_light_boost;
}

/**
 *
 */
INLINE void StandardMaterial::
set_rim_light_exponent(PN_stdfloat exponent) {
  _rim_light_exponent = exponent;
}

/**
 *
 */
INLINE PN_stdfloat StandardMaterial::
get_rim_light_exponent() const {
  return _rim_light_exponent;
}

/**
 *
 */
INLINE void StandardMaterial::
set_half_lambert(bool flag) {
  _half_lambert = flag;
}

/**
 *
 */
INLINE bool StandardMaterial::
get_half_lambert() const {
  return _half_lambert;
}

/**
 *
 */
INLINE void StandardMaterial::
set_base_texture(Texture *tex) {
  _base_texture = tex;
}

/**
 *
 */
INLINE Texture *StandardMaterial::
get_base_texture() const {
  return _base_texture;
}

/**
 *
 */
INLINE void StandardMaterial::
set_base_color(const LColor &color) {
  _base_color = color;
}

/**
 *
 */
INLINE const LColor &StandardMaterial::
get_base_color() const {
  return _base_color;
}

/**
 *
 */
INLINE void StandardMaterial::
set_normal_texture(Texture *tex) {
  _normal_texture = tex;
}

/**
 *
 */
INLINE Texture *StandardMaterial::
get_normal_texture() const {
  return _normal_texture;
}

/**
 *
 */
INLINE void StandardMaterial::
set_lightwarp_texture(Texture *tex) {
  _lightwarp_texture = tex;
}

/**
 *
 */
INLINE Texture *StandardMaterial::
get_lightwarp_texture() const {
  return _lightwarp_texture;
}

/**
 *
 */
INLINE void StandardMaterial::
set_envmap_texture(Texture *tex) {
  _envmap_texture = tex;
}

/**
 *
 */
INLINE Texture *StandardMaterial::
get_envmap_texture() const {
  return _envmap_texture;
}

/**
 *
 */
INLINE void StandardMaterial::
set_env_cubemap(bool flag) {
  _env_cubemap = flag;
}

/**
 *
 */
INLINE bool StandardMaterial::
get_env_cubemap() const {
  return _env_cubemap;
}

/**
 *
 */
INLINE void StandardMaterial::
set_emission_enabled(bool enabled) {
  _enable_emission = enabled;
}

/**
 *
 */
INLINE bool StandardMaterial::
get_emission_enabled() const {
  return _enable_emission;
}

/**
 *
 */
INLINE void StandardMaterial::
set_arme_texture(Texture *tex) {
  _arme_texture = tex;
}

/**
 *
 */
INLINE Texture *StandardMaterial::
get_arme_texture() const {
  return _arme_texture;
}

/**
 *
 */
INLINE void StandardMaterial::
set_roughness(PN_stdfloat roughness) {
  _roughness = roughness;
}

/**
 *
 */
INLINE PN_stdfloat StandardMaterial::
get_roughness() const {
  return _roughness;
}

/**
 *
 */
INLINE void StandardMaterial::
set_metalness(PN_stdfloat metalness) {
  _metalness = metalness;
}

/**
 *
 */
INLINE PN_stdfloat StandardMaterial::
get_metalness() const {
  return _metalness;
}

/**
 *
 */
INLINE void StandardMaterial::
set_emission(PN_stdfloat emission) {
  _emission = emission;
}

/**
 *
 */
INLINE PN_stdfloat StandardMaterial::
get_emission() const {
  return _emission;
}
