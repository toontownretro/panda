/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialRegistry.cxx
 * @author lachbr
 * @date 2021-03-06
 */

#include "materialRegistry.h"
#include "material.h"
#include "typeRegistry.h"

MaterialRegistry *MaterialRegistry::_global_ptr = nullptr;

/**
 *
 */
MaterialRegistry::
MaterialRegistry() {
}

/**
 * Registers a new material type that can be instantiated by name or by
 * TypeHandle.
 */
void MaterialRegistry::
register_material(const TypeHandle &type, CreateMaterialFunc create_func) {
  nassertv(_registered_materials.find(type) == _registered_materials.end());
  nassertv(type.is_derived_from(Material::get_class_type()));

  _registered_materials[type] = create_func;
}

/**
 * Creates and returns a new material of the type with the given name.
 *
 * Returns NULL if there is no registered material type with the given name.
 */
PT(Material) MaterialRegistry::
create_material(const std::string &name) {
  TypeHandle type = TypeRegistry::ptr()->find_type(name);
  if (type == TypeHandle::none()) {
    return nullptr;
  }

  return create_material(type);
}

/**
 * Creates and returns new material of the indicated type.
 *
 * Returns NULL if there is no registered material type with the given name.
 */
PT(Material) MaterialRegistry::
create_material(const TypeHandle &type) {
  RegisteredMaterials::const_iterator it = _registered_materials.find(type);
  if (it == _registered_materials.end()) {
    return nullptr;
  }

  CreateMaterialFunc func = (*it).second;

  return (*func)();
}

/**
 *
 */
MaterialRegistry *MaterialRegistry::
get_global_ptr() {
  if (_global_ptr == nullptr) {
    _global_ptr = new MaterialRegistry;
  }

  return _global_ptr;
}
