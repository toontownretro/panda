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

#include "materialBase.h"
#include "materialRegistry.h"
#include "pointerTo.h"
#include "texture.h"

/**
 * Standard material with all the fundamental parameters (base texture, normal
 * map, etc), with a few common fancy parameters (rim light, lightwarp, etc).
 */
class EXPCL_PANDA_GOBJ StandardMaterial : public MaterialBase {
PUBLISHED:
  StandardMaterial(const std::string &name = "");

  INLINE void set_rim_light(bool rim_light);
  INLINE bool get_rim_light() const;

  INLINE void set_rim_light_boost(PN_stdfloat boost);
  INLINE PN_stdfloat get_rim_light_boost() const;

  INLINE void set_rim_light_exponent(PN_stdfloat exponent);
  INLINE PN_stdfloat get_rim_light_exponent() const;

  INLINE void set_half_lambert(bool half_lambert);
  INLINE bool get_half_lambert() const;

  INLINE void set_base_texture(Texture *texture);
  INLINE Texture *get_base_texture() const;

  INLINE void set_base_color(const LColor &color);
  INLINE LColor get_base_color() const;

  INLINE void set_normal_texture(Texture *texture);
  INLINE Texture *get_normal_texture() const;

  INLINE void set_lightwarp_texture(Texture *texture);
  INLINE Texture *get_lightwarp_texture() const;

  INLINE void set_envmap_texture(Texture *texture);
  INLINE Texture *get_envmap_texture() const;

  INLINE void set_env_cubemap(bool flag);
  INLINE bool get_env_cubemap() const;

  INLINE void set_emission_enabled(bool enable);
  INLINE bool get_emission_enabled() const;

  INLINE void set_arme_texture(Texture *texture);
  INLINE Texture *get_arme_texture() const;

  INLINE void set_roughness(PN_stdfloat roughness);
  INLINE PN_stdfloat get_roughness() const;

  INLINE void set_metalness(PN_stdfloat metalness);
  INLINE PN_stdfloat get_metalness() const;

  INLINE void set_emission(PN_stdfloat emission);
  INLINE PN_stdfloat get_emission() const;

public:
  virtual void read_keyvalues(KeyValues *kv, const DSearchPath &search_path) override;

public:
  static MaterialBase *create_StandardMaterial();

protected:
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MaterialBase::init_type();
    register_type(_type_handle, "StandardMaterial",
                  MaterialBase::get_class_type());
    MaterialRegistry::get_global_ptr()
      ->register_material(_type_handle, create_StandardMaterial);
  }

private:
  static TypeHandle _type_handle;
};

#include "standardMaterial.I"

#endif // STANDARDMATERIAL_H
