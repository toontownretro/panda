/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderManager.cxx
 * @author lachbr
 * @date 2020-10-12
 */

#include "shaderManager.h"
#include "config_putil.h"
#include "load_dso.h"
#include "shaderBase.h"
#include "shaderParamAttrib.h"
#include "renderState.h"

typedef void (*ShaderLibInit)();

ShaderManager *ShaderManager::_global_ptr = nullptr;

/**
 * Returns the global ShaderManager object.
 */
ShaderManager *ShaderManager::
get_global_ptr() {
  if (!_global_ptr) {
    _global_ptr = new ShaderManager;
  }
  return _global_ptr;
}

/**
 * Loads the shader plugin libraries specified in PRC file.
 */
void ShaderManager::
load_shader_libraries() {
  const ConfigVariableList &libraries = get_shader_libraries();

  for (size_t i = 0; i < libraries.get_num_unique_values(); i++) {
    std::string lib_name = libraries.get_unique_value(i);
    Filename lib_filename = Filename::dso_filename("lib" + lib_name + ".so");
    lib_filename.to_os_specific();

    shadermgr_cat.info()
      << "Loading shader library " << lib_filename.get_fullpath() << "\n";

    void *handle = load_dso(get_plugin_path().get_value(), lib_filename);
    if (!handle) {
      shadermgr_cat.warning()
        << "Unable to load shader library " << lib_filename.get_fullpath()
        << " on plugin path " << get_plugin_path()
        << "\n";
      continue;

    } else {
      // Look for the function named `init_lib<shader library name>`.
      // This function should already be defined if you are following Panda's
      // convention for library initialization.  We call it to initialize and
      // register the shaders that you define in your library.
      void *symbol = get_dso_symbol(handle, "init_lib" + lib_name);

      if (!symbol) {
        shadermgr_cat.warning()
          << "Shader library " << lib_filename.get_fullpath() << " does not "
          << "define the initialization function: init_lib" << lib_name << "()\n";
        unload_dso(handle);
        continue;

      } else {
        // Call the initialization function
        ShaderLibInit init_func = (ShaderLibInit)symbol;
        (*init_func)();
      }
    }
  }
}

/**
 * Registers the shader.  The shader can now be invoked by RenderStates that
 * want to use it.
 */
void ShaderManager::
register_shader(ShaderBase *shader) {
  _shaders[shader->get_name()] = shader;
}

/**
 * Generates a shader for a given RenderState.  Invokes the shader instance
 * requested by name in the state.
 */
CPT(RenderAttrib) ShaderManager::
generate_shader(const RenderState *state,
                const GeomVertexAnimationSpec &anim_spec) {
  // First figure out what shader the state would like to use.
  const ShaderParamAttrib *spa;
  state->get_attrib_def(spa);

  ShaderBase *shader = get_shader(spa->get_shader_name());
  if (!shader) {
    shadermgr_cat.error()
      << "Shader `" << spa->get_shader_name() << "` not found\n";
    return nullptr;
  }
}
