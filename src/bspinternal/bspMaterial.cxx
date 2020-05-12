/**
 * PANDA3D BSP LIBRARY
 *
 * Copyright (c) Brian Lach <brianlach72@gmail.com>
 * All rights reserved.
 *
 * @file bsp_material.cpp
 * @author Brian Lach
 * @date November 02, 2018
 */

#include "bspMaterial.h"

#include <lightReMutex.h>
#include <lightReMutexHolder.h>

static LightReMutex g_matmutex("MaterialMutex");

//====================================================================//

#include "keyvalues.h"
#include <virtualFileSystem.h>

NotifyCategoryDef(bspmaterial, "");

TypeHandle BSPMaterial::_type_handle;

BSPMaterial::materialcache_t BSPMaterial::_material_cache;

const BSPMaterial *BSPMaterial::get_from_file(const Filename &file) {
  LightReMutexHolder holder(g_matmutex);

  int idx = _material_cache.find(file);
  if (idx != -1) {
    // We've already loaded this material file.
    return _material_cache.get_data(idx);
  }

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  if (!vfs->exists(file)) {
    bspmaterial_cat.error()
      << "Could not find material file " << file.get_fullpath() << "\n";
    return nullptr;
  }

  bspmaterial_cat.info()
    << "Loading material " << file.get_fullpath() << "\n";

  PT(BSPMaterial) mat = new BSPMaterial;
  mat->_file = file;

  PT(CKeyValues) kv = CKeyValues::load(file);
  if (!kv) {
    bspmaterial_cat.error()
      << "Problem loading " << file.get_fullpath() << "\n";
    return nullptr;
  }
  CKeyValues *mat_kv = kv->get_child(0);
  if (mat_kv->get_name() == "patch") {
    int iinclude = mat_kv->find_key("$include");
    if (iinclude != -1) {
      std::string include_file = mat_kv->get_value(iinclude);
      const BSPMaterial *include_mat = get_from_file(include_file);
      if (!include_mat) {
        bspmaterial_cat.error()
          << "Could not load $include material `" << include_file
          << "` referenced by patch material `" << file << "`\n";
        return nullptr;
      }

      // Use the shader from the included material
      mat->set_shader(include_mat->get_shader());

      // Put the included material's properties in front of the patch.
      // This way, the patch material's properties will be iterated over last
      // and be able to override the include material.
      for (size_t i = 0; i < include_mat->get_num_keyvalues(); i++) {
        mat->set_keyvalue(include_mat->get_key(i), include_mat->get_value(i));
      }
    } else {
      bspmaterial_cat.error()
        << "Patch material " << file << " didn't provide an $include\n";
      return nullptr;
    }
  } else {
    mat->set_shader(mat_kv->get_name()); // ->VertexLitGeneric<- {...}
  }

  for (size_t i = 0; i < mat_kv->get_num_keys(); i++) {
    mat->set_keyvalue(mat_kv->get_key(i), mat_kv->get_value(i)); // "$basetexture"   "phase_3/maps/desat_shirt_1.jpg"
  }

  // Figure out these values and store
  // for fast and easy access elsewhere.
  mat->_has_env_cubemap = (mat->has_keyvalue("$envmap") && mat->get_keyvalue("$envmap") == "env_cubemap");
  if (mat->has_keyvalue("$surfaceprop"))
    mat->_surfaceprop = mat->get_keyvalue("$surfaceprop");
  if (mat->has_keyvalue("$contents"))
    mat->_contents = mat->get_keyvalue("$contents");
  mat->_has_transparency = (mat->has_keyvalue("$translucent") && atoi(mat->get_keyvalue("$translucent").c_str()) == 1) ||
    (mat->has_keyvalue("$alpha") && atof(mat->get_keyvalue("$alpha").c_str()) < 1.0);
  mat->_has_bumpmap = mat->has_keyvalue("$bumpmap");
  // UNDONE: This is hardcoded, maybe define a global list of lightmapped shaders?
  mat->_lightmapped = mat->get_shader() == "LightmappedGeneric";
  mat->_skybox = mat->get_shader() == "SkyBox";

  _material_cache[file] = mat;

  return mat;
}

//====================================================================//
