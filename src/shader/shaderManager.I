/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderManager.I
 * @author brian
 * @date 2020-10-16
 */

/**
 *
 */
INLINE ShaderManager::
ShaderManager() {
  _quality = config_get_shader_quality();

  build_default_textures();
}

/**
 * Sets the quality level for all shaders.  This is only a suggestion, it may
 * have no meaning to certain shaders.  It is up to the shader implementation
 * to respect the chosen quality level.
 *
 * Changing this value forces all shaders to be regenerated.
 */
INLINE void ShaderManager::
set_shader_quality(ShaderManager::ShaderQuality quality) {
  if (quality != _quality) {
    // The quality level changed.  Force all shaders to regenerate.
    _quality = quality;
    GraphicsStateGuardianBase::mark_rehash_generated_shaders();
  }
}

/**
 * Returns the quality level for all shaders.  This is only a suggestion, it
 * may have no meaning to certain shaders.  It is up to the shader
 * implementation to respect the chosen quality level.
 */
INLINE ShaderManager::ShaderQuality ShaderManager::
get_shader_quality() const {
  return _quality;
}

/**
 * Sets the default cube map texture.
 *
 * Changing this value forces all shaders to be regenerated.
 */
INLINE void ShaderManager::
set_default_cube_map(Texture *texture) {
  if (texture != _default_cubemap) {
    _default_cubemap = texture;
    GraphicsStateGuardianBase::mark_rehash_generated_shaders();
  }
}

/**
 * Returns the global ShaderManager object.
 */
INLINE ShaderManager *ShaderManager::
get_global_ptr() {
  if (!_global_ptr) {
    _global_ptr = new ShaderManager;
    // So GraphicsStateGuardian can pick us up, who relies on the interface
    // class.  Aargh.
    ShaderManagerBase::set_global_shader_manager(_global_ptr);
  }
  return _global_ptr;
}

/**
 * Returns the shader instance by name, or nullptr if no shader exists with
 * that name.
 */
INLINE ShaderBase *ShaderManager::
get_shader(CPT_InternalName name) const {
  ShaderRegistry::const_iterator it = _shaders.find(name);
  if (it != _shaders.end()) {
    return (*it).second;
  }

  return nullptr;
}

/**
 *
 */
INLINE Texture *ShaderManager::
get_white_texture() const {
  return _white_texture;
}

/**
 *
 */
INLINE Texture *ShaderManager::
get_black_texture() const {
  return _black_texture;
}

/**
 *
 */
INLINE Texture *ShaderManager::
get_flat_normal_map() const {
  return _flat_normal_map;
}
