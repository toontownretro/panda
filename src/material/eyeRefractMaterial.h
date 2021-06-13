/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eyeRefractMaterial.h
 * @author lachbr
 * @date 2021-03-24
 */

#ifndef EYEREFRACTMATERIAL_H
#define EYEREFRACTMATERIAL_H

#include "material.h"
#include "materialRegistry.h"

class EXPCL_PANDA_MATERIAL EyeRefractMaterial : public Material {
PUBLISHED:
  EyeRefractMaterial(const std::string &name = "");

public:
  virtual void read_pdx(PDXElement *data, const DSearchPath &search_path) override;

  // Bam factory methods.
  static void register_with_read_factory();
  static TypedWritable *make_from_bam(const FactoryParams &params);

  // Material registry methods.
  static Material *create_EyeRefractMaterial();

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    Material::init_type();
    register_type(_type_handle, "EyeRefractMaterial",
                  Material::get_class_type());
    MaterialRegistry::get_global_ptr()
      ->register_material(_type_handle, create_EyeRefractMaterial);
  }

private:
  static TypeHandle _type_handle;
};

#include "eyeRefractMaterial.I"

#endif // EYEREFRACTMATERIAL_H
