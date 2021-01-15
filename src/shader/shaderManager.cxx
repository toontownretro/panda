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
#include "shader.h"
#include "shaderBase.h"
#include "paramAttrib.h"
#include "shaderAttrib.h"
#include "shaderInput.h"
#include "pvector.h"
#include "renderState.h"
#include "pStatCollector.h"
#include "pStatTimer.h"

static PStatCollector generate_collector("*:Munge:GenerateShader");
static PStatCollector find_shader_collector("*:Munge:GenerateShader:FindShader");
static PStatCollector synthesize_source_collector("*:Munge:GenerateShader:SetupShader");
static PStatCollector make_shader_collector("*:Munge:GenerateShader:MakeShaderObject");
static PStatCollector make_attrib_collector("*:Munge:GenerateShader:MakeShaderAttrib");
static PStatCollector reset_collector("*:Munge:GenerateShader:ResetShader");
static PStatCollector cache_collector("*:Munge:GenerateShader:CacheLookup");

typedef void (*ShaderLibInit)();

ShaderManager *ShaderManager::_global_ptr = nullptr;

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
generate_shader(GraphicsStateGuardianBase *gsg,
                const RenderState *state,
                const GeomVertexAnimationSpec &anim_spec) {
  PStatTimer timer(generate_collector);

  // First figure out what shader the state would like to use.
  const ParamAttrib *pa;
  state->get_attrib_def(pa);

  const ShaderAttrib *shattr;
  state->get_attrib_def(shattr);

  if (shadermgr_cat.is_debug()) {
    shadermgr_cat.debug()
      << "Generating shader for state: ";
    state->output(shadermgr_cat.debug(false));
    shadermgr_cat.debug(false) << "\n";
  }

  find_shader_collector.start();
  ShaderBase *shader = get_shader(shattr->get_shader_name());
  find_shader_collector.stop();
  if (!shader) {
    shadermgr_cat.error()
      << "Shader `" << shattr->get_shader_name() << "` not found\n";
    return ShaderAttrib::make();
  }

  if (shadermgr_cat.is_debug()) {
    shadermgr_cat.debug()
      << "Using shader " << shader->get_name() << "\n";
  }

  synthesize_source_collector.start();
  shader->generate_shader(gsg, state, pa, anim_spec);
  synthesize_source_collector.stop();

  CPT(RenderAttrib) generated_attr;

  cache_collector.start();
  // Now see if we've already created a shader with the same setup.
  ShaderBase::SetupCache::const_iterator it = shader->_cache.find(shader->_setup);
  cache_collector.stop();
  if (it != shader->_cache.end()) {
    // We have!  Just use that.
    generated_attr = (*it).second;

  } else {
    make_shader_collector.start();
    PT(Shader) shader_obj = Shader::make(
      shader->get_language(),
      shader->get_stage(ShaderBase::S_vertex).get_final_source(),
      shader->get_stage(ShaderBase::S_pixel).get_final_source(),
      shader->get_stage(ShaderBase::S_geometry).get_final_source(),
      shader->get_stage(ShaderBase::S_tess).get_final_source(),
      shader->get_stage(ShaderBase::S_tess_eval).get_final_source());

    make_shader_collector.stop();

    make_attrib_collector.start();

    generated_attr = ShaderAttrib::make(shader_obj);

    if (shader->get_num_inputs() > (size_t)0) {
      if (shadermgr_cat.is_debug()) {
        shadermgr_cat.debug()
          << "Applying shader inputs\n";
      }
      generated_attr = DCAST(ShaderAttrib, generated_attr)
        ->set_shader_inputs(shader->get_inputs());
    }

    if (shader->get_flags() != 0) {
      if (shadermgr_cat.is_debug()) {
        shadermgr_cat.debug()
          << "Setting shader flags\n";
      }
      generated_attr = DCAST(ShaderAttrib, generated_attr)
        ->set_flag(shader->get_flags(), true);
    }

    if (shader->get_instance_count() > 0) {
      if (shadermgr_cat.is_debug()) {
        shadermgr_cat.debug()
          << "Setting shader instance count to "
          << shader->get_instance_count() << "\n";
      }
      generated_attr = DCAST(ShaderAttrib, generated_attr)
        ->set_instance_count(shader->get_instance_count());
    }

    make_attrib_collector.stop();

    // Throw it in the cache.
    shader->_cache.insert(
      ShaderBase::SetupCache::value_type(shader->_setup, generated_attr));
  }

  make_attrib_collector.start();

  // Apply inputs from the ShaderAttrib stored directly on the state to our
  // generated ShaderAttrib.
  if (shattr->get_num_shader_inputs() > (size_t)0) {
    if (shadermgr_cat.is_debug()) {
      shadermgr_cat.debug()
        << "Copying inputs shader inputs from target state\n";
    }
    generated_attr = DCAST(ShaderAttrib, generated_attr)->copy_shader_inputs_from(shattr);
  }

  make_attrib_collector.stop();

  if (shadermgr_cat.is_debug()) {
    shadermgr_cat.debug()
      << "Generated shader: ";
    generated_attr->output(shadermgr_cat.debug(false));
    shadermgr_cat.debug(false) << "\n";
  }

  reset_collector.start();
  // Reset the shader for next time.
  shader->reset();
  reset_collector.stop();

  return generated_attr;
}
