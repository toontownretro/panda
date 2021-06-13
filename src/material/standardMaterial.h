/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file standardMaterial.h
 * @author lachbr
 * @date 2021-03-06
 */

#ifndef STANDARDMATERIAL_H
#define STANDARDMATERIAL_H

#include "material.h"
#include "materialRegistry.h"
#include "pointerTo.h"
#include "texture.h"

/**
 * Standard material with all the fundamental parameters (base texture, normal
 * map, etc), with a few common fancy parameters (rim light, lightwarp, etc).
 */
class EXPCL_PANDA_MATERIAL StandardMaterial : public Material {
PUBLISHED:
  StandardMaterial(const std::string &name = "");

  void set_rim_light(bool rim_light);
  bool get_rim_light() const;

  void set_rim_light_boost(PN_stdfloat boost);
  PN_stdfloat get_rim_light_boost() const;

  void set_rim_light_exponent(PN_stdfloat exponent);
  PN_stdfloat get_rim_light_exponent() const;

  void set_half_lambert(bool half_lambert);
  bool get_half_lambert() const;

  void set_base_texture(Texture *texture);
  Texture *get_base_texture() const;

  void set_base_color(const LColor &color);
  LColor get_base_color() const;

  void set_normal_texture(Texture *texture);
  Texture *get_normal_texture() const;

  void set_lightwarp_texture(Texture *texture);
  Texture *get_lightwarp_texture() const;

  void set_envmap_texture(Texture *texture);
  Texture *get_envmap_texture() const;

  void set_env_cubemap(bool flag);
  bool get_env_cubemap() const;

  void set_emission_enabled(bool enable);
  bool get_emission_enabled() const;

  void set_arme_texture(Texture *texture);
  Texture *get_arme_texture() const;

  void set_roughness(PN_stdfloat roughness);
  PN_stdfloat get_roughness() const;

  void set_metalness(PN_stdfloat metalness);
  PN_stdfloat get_metalness() const;

  void set_emission(PN_stdfloat emission);
  PN_stdfloat get_emission() const;

  void set_emission_tint(const LVecBase3 &tint);
  LVecBase3 get_emission_tint() const;

  void set_specular_texture(Texture *texture);
  Texture *get_specular_texture() const;

public:
  virtual void read_pdx(PDXElement *data, const DSearchPath &search_path) override;

  // Bam factory methods.
  static void register_with_read_factory();
  static TypedWritable *make_from_bam(const FactoryParams &params);

  // Material registry methods.
  static Material *create_StandardMaterial();

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
    register_type(_type_handle, "StandardMaterial",
                  Material::get_class_type());
    MaterialRegistry::get_global_ptr()
      ->register_material(_type_handle, create_StandardMaterial);
  }

private:
  static TypeHandle _type_handle;
};

#include "standardMaterial.I"

#endif // STANDARDMATERIAL_H
