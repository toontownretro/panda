/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialRegistry.h
 * @author lachbr
 * @date 2021-03-06
 */

#ifndef MATERIALREGISTRY_H
#define MATERIALREGISTRY_H

#include "pandabase.h"
#include "pointerTo.h"
#include "typeHandle.h"
#include "pmap.h"

class Material;

/**
 * This is a global class that knows about all material types in the world and
 * can instantiate one by name or by TypeHandle.  Each material type should
 * register itself with this class.
 */
class EXPCL_PANDA_MATERIAL MaterialRegistry {
private:
  MaterialRegistry();

public:
  typedef Material *(*CreateMaterialFunc)();

  void register_material(const TypeHandle &type, CreateMaterialFunc create_func);

PUBLISHED:
  PT(Material) create_material(const std::string &name);
  PT(Material) create_material(const TypeHandle &type);

  static MaterialRegistry *get_global_ptr();

private:
  typedef pmap<TypeHandle, CreateMaterialFunc> RegisteredMaterials;
  RegisteredMaterials _registered_materials;

  static MaterialRegistry *_global_ptr;
};

#include "materialRegistry.I"

#endif // MATERIALREGISTRY_H
