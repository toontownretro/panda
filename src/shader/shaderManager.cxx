/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderManager.cxx
 * @author brian
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
#include "graphicsStateGuardianBase.h"
//#include "shaderSource.h"
#include "shaderStage.h"
#include "textureAttrib.h"
#include "textureStagePool.h"
#include "lightMutexHolder.h"

#include "sourceShader.h"
#include "sourceMaterial.h"
#include "sourceLightmappedMaterial.h"
#include "sourceLightmappedShader.h"

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
 * Forces all shaders to be reloaded and regenerated.
 */
void ShaderManager::
reload_shaders(bool clear_file_cache) {
  GraphicsStateGuardianBase::mark_rehash_generated_shaders();
  for (auto it = _shaders.begin(); it != _shaders.end(); ++it) {
    (*it).second->clear_cache();
  }
  if (clear_file_cache) {
    ShaderStage::clear_sho_cache();
  }
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
#if 0
    shadermgr_cat.error()
      << "Could not determine shader to use for state: ";
    state->output(shadermgr_cat.error(false));
    shadermgr_cat.error(false) << "\n";
    if (material == nullptr) {
      shadermgr_cat.error()
        << "No material\n";
    } else {
      shadermgr_cat.error()
        << material->get_type() << "\n";
    }
    shadermgr_cat.error()
      << "Currently registered shaders:\n";
    for (auto it = _material_shaders.begin(); it != _material_shaders.end(); ++it) {
      shadermgr_cat.error(false)
        << "\t" << (*it).first << " : " << (*it).second->get_name() << "\n";
    }
#endif
    // Use the fallback shader.
    MaterialShaders::const_iterator msi = _material_shaders.find(TypeHandle::none());
    nassertr(msi != _material_shaders.end(), ShaderAttrib::make());
    shader = (*msi).second;
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
  ShaderSetup setup;
  shader->generate_shader(gsg, state, material, anim_spec, setup);
  setup._obj_setup.calc_variation_indices();
  synthesize_source_collector.stop();

  PT(Shader) shader_obj;
  CPT(RenderAttrib) generated_attr;

  {
    LightMutexHolder holder(shader->_lock);

    cache_collector.start();
    // See if we need to create a new Shader object for the setup.
    ShaderBase::ObjectSetupCache::const_iterator oit = shader->_obj_cache.find(setup._obj_setup);
    cache_collector.stop();
    if (oit != shader->_obj_cache.end()) {
      if (shadermgr_cat.is_debug()) {
        shadermgr_cat.debug()
          << "Object cache hit\n";
      }
      shader_obj = (*oit).second;

    } else {
      make_shader_collector.start();

      COWPT(ShaderModule) v_mod = setup.get_stage(ShaderSetup::S_vertex).get_module();
      COWPT(ShaderModule) p_mod = setup.get_stage(ShaderSetup::S_pixel).get_module();
      COWPT(ShaderModule) g_mod = setup.get_stage(ShaderSetup::S_geometry).get_module();
      COWPT(ShaderModule) t_mod = setup.get_stage(ShaderSetup::S_tess).get_module();
      COWPT(ShaderModule) te_mod = setup.get_stage(ShaderSetup::S_tess_eval).get_module();
      shader_obj = Shader::make(
        setup.get_language(),
        (ShaderModule *)v_mod.get_read_pointer(),
        (ShaderModule *)p_mod.get_read_pointer(),
        (ShaderModule *)g_mod.get_read_pointer(),
        (ShaderModule *)t_mod.get_read_pointer(),
        (ShaderModule *)te_mod.get_read_pointer());
      nassertr(shader_obj != nullptr, nullptr);
      if (shader_obj != nullptr) {
        // Supply the specialization constants.
        for (auto cit = setup._obj_setup._spec_constants.begin(); cit != setup._obj_setup._spec_constants.end(); ++cit) {
          shader_obj->set_constant((*cit).first, (*cit).second);
          if (shadermgr_cat.is_debug()) {
            std::cout << "spec constant: " << (*cit).first->get_name() << " -> " << (*cit).second << "\n";
          }
        }
      }
      make_shader_collector.stop();

      // Throw it in the cache.
      shader->_obj_cache.insert(
        ShaderBase::ObjectSetupCache::value_type(setup._obj_setup, shader_obj));
    }

#if 0
    cache_collector.start();
    // Now see if we've already created a shader attrib with the same setup.
    ShaderBase::SetupCache::const_iterator it = shader->_cache.find(setup._setup);
    cache_collector.stop();

    if (it != shader->_cache.end()) {
      if (shadermgr_cat.is_debug()) {
        shadermgr_cat.debug()
          << "ShaderAttrib cache hit\n";
      }

      // We have!  Just use that.
      generated_attr = (*it).second;

      // Make sure the attribute uses the correct Shader object.
      CPT(ShaderAttrib) shattr = DCAST(ShaderAttrib, generated_attr);
      if (shattr->get_shader() != shader_obj) {
        generated_attr = shattr->set_shader(shader_obj);
      }

    } else
#endif
    {
      make_attrib_collector.start();

      generated_attr = ShaderAttrib::make(shader_obj, std::move(setup.move_inputs()),
                                          setup.get_flags(), setup.get_instance_count());

      make_attrib_collector.stop();

      // Throw it in the cache.
      //shader->_cache.insert(
      //  ShaderBase::SetupCache::value_type(setup._setup, generated_attr));
    }
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

  return generated_attr;
}
