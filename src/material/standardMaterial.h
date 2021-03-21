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
 *
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
  INLINE const LColor &get_base_color() const;

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
  virtual void read_keyvalues(KeyValues *kv) override;
  virtual void write_keyvalues(KeyValues *kv) override;

private:
  bool _rim_light; // Enable a dedicated rim lighting term?
  PN_stdfloat _rim_light_boost;
  PN_stdfloat _rim_light_exponent;

  bool _half_lambert; // Enable half-lambertian diffuse?

  // Can have a base color texture...
  PT(Texture) _base_texture;
  // ...or a single base color value.
  LColor _base_color;

  PT(Texture) _normal_texture;
  PT(Texture) _lightwarp_texture;

  // We might have a dedicated environment map...
  PT(Texture) _envmap_texture;
  // ...or use the closest environment map to the node.
  bool _env_cubemap;

  // Emission has to be turned on explicitly.
  bool _enable_emission;

  // Can have a packed occlusion-roughness-metalness-emission texture...
  PT(Texture) _arme_texture;
  // ...or dedicated values for roughness, metalness, and emission.
  PN_stdfloat _roughness;
  PN_stdfloat _metalness;
  PN_stdfloat _emission;

public:
  virtual void write_datagram(BamWriter *manager, Datagram &dg) override;

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
