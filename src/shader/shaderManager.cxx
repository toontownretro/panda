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
#include "shaderAttrib.h"
#include "shaderInput.h"
#include "pvector.h"
#include "renderState.h"
#include "pStatCollector.h"
#include "pStatTimer.h"
#include "material.h"
#include "materialAttrib.h"
#include "texturePool.h"

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
 * Returns the default cube map texture.  Loads the texture from the config
 * variable if it hasn't already been loaded.
 */
Texture *ShaderManager::
get_default_cube_map() {
  if (_default_cubemap == nullptr && !default_cube_map.empty()) {
    _default_cubemap = TexturePool::load_texture(default_cube_map);
  }
  return _default_cubemap;
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
 * Registers the indicated shader.
 */
void ShaderManager::
register_shader(ShaderBase *shader) {
  _shaders[InternalName::make(shader->get_name())] = shader;
  for (size_t i = 0; i < shader->get_num_aliases(); i++) {
    _shaders[InternalName::make(shader->get_alias(i))] = shader;
  }
}

/**
 * Registers the indicated shader and associates it with the indicated material
 * type.
 */
void ShaderManager::
register_shader(ShaderBase *shader, TypeHandle material_type) {
  register_shader(shader);
  _material_shaders[material_type] = shader;
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

  // First figure out what shader the state should use.
  const MaterialAttrib *mattr;
  state->get_attrib_def(mattr);

  Material *material = mattr->get_material();

  const ShaderAttrib *shattr;
  state->get_attrib_def(shattr);

  ShaderBase *shader = nullptr;
  if (shattr->get_shader_name() == nullptr) {
    // Use the shader associated with the material type.
    TypeHandle material_type = (!mattr->is_off() && material != nullptr)
      ? material->get_type() : TypeHandle::none();
    MaterialShaders::const_iterator msi = _material_shaders.find(material_type);
    if (msi != _material_shaders.end()) {
      shader = (*msi).second;
    }

  } else {
    // There's a specific shader requested by the render state, regardless of
    // the material type.
    shader = get_shader(shattr->get_shader_name());
  }

  if (shader == nullptr) {
    shadermgr_cat.error()
      << "Could not determine shader to use for state: ";
    state->output(shadermgr_cat.error(false));
    return ShaderAttrib::make();
  }

  if (shadermgr_cat.is_debug()) {
    shadermgr_cat.debug()
      << "Generating shader for state: ";
    state->output(shadermgr_cat.debug(false));
    shadermgr_cat.debug(false) << "\n";

    shadermgr_cat.debug()
      << "Using shader " << shader->get_name() << "\n";
  }

  synthesize_source_collector.start();
  shader->generate_shader(gsg, state, material, anim_spec);
  synthesize_source_collector.stop();

  PT(Shader) shader_obj;
  CPT(RenderAttrib) generated_attr;

  cache_collector.start();
  // See if we need to create a new Shader object for the setup.
  ShaderBase::ObjectSetupCache::const_iterator oit = shader->_obj_cache.find(shader->_obj_setup);
  cache_collector.stop();
  if (oit != shader->_obj_cache.end()) {
    shader_obj = (*oit).second;

  } else {
    make_shader_collector.start();
    shader_obj = Shader::make(
      shader->get_language(),
      shader->get_stage(ShaderBase::S_vertex).get_final_source(),
      shader->get_stage(ShaderBase::S_pixel).get_final_source(),
      shader->get_stage(ShaderBase::S_geometry).get_final_source(),
      shader->get_stage(ShaderBase::S_tess).get_final_source(),
      shader->get_stage(ShaderBase::S_tess_eval).get_final_source());
    make_shader_collector.stop();

    // Throw it in the cache.
    shader->_obj_cache.insert(
      ShaderBase::ObjectSetupCache::value_type(shader->_obj_setup, shader_obj));
  }

  cache_collector.start();
  // Now see if we've already created a shader attrib with the same setup.
  ShaderBase::SetupCache::const_iterator it = shader->_cache.find(shader->_setup);
  cache_collector.stop();

  if (it != shader->_cache.end()) {
    // We have!  Just use that.
    generated_attr = (*it).second;

    // Make sure the attribute uses the correct Shader object.
    CPT(ShaderAttrib) shattr = DCAST(ShaderAttrib, generated_attr);
    if (shattr->get_shader() != shader_obj) {
      generated_attr = shattr->set_shader(shader_obj);
    }

  } else {
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
