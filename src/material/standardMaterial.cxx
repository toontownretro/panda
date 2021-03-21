/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file standardMaterial.cxx
 * @author lachbr
 * @date 2021-03-06
 */

#include "standardMaterial.h"
#include "keyValues.h"
#include "string_utils.h"
#include "texturePool.h"

/**
 *
 */
StandardMaterial::
StandardMaterial(const std::string &name) :
  MaterialBase(name)
{
  _rim_light = false;
  _rim_light_boost = 2;
  _rim_light_exponent = 4;

  _half_lambert = false;

  _base_color.set(0.75, 0.75, 0.75, 1.0);

  _env_cubemap = false;

  _enable_emission = false;

  _roughness = 1.0f;
  _metalness = 0.0f;
  _emission = 0.0f;
}

/**
 *
 */
void StandardMaterial::
read_keyvalues(KeyValues *kv) {
  for (size_t i = 0; i < kv->get_num_keys(); i++) {
    std::string key = downcase(kv->get_key(i));
    const std::string &value = kv->get_value(i);

    if (key == "$basetexture" || key == "$basemap") {
      set_base_texture(TexturePool::load_texture(value));

    } else if (key == "$basecolor") {
      set_base_color(KeyValues::to_4f(value));

    } else if (key == "$normal" || key == "$bumpmap" ||
               key == "$normalmap" || key == "$normaltexture") {
      set_normal_texture(TexturePool::load_texture(value));

    } else if (key == "$arme" || key == "$armemap" || key == "$armetexture") {
      set_arme_texture(TexturePool::load_texture(value));

    } else if (key == "$halflambert") {
      int flag;
      string_to_int(value, flag);
      set_half_lambert((bool)flag);

    } else if (key == "$rimlight") {
      int flag;
      string_to_int(value, flag);
      set_rim_light((bool)flag);

    } else if (key == "$rimlight_boost") {
      PN_stdfloat boost;
      string_to_stdfloat(value, boost);
      set_rim_light_boost(boost);

    } else if (key == "$rimlight_exponent") {
      PN_stdfloat exp;
      string_to_stdfloat(value, exp);
      set_rim_light_exponent(exp);

    } else if (key == "$selfillum") {
      int flag;
      string_to_int(value, flag);
      set_emission_enabled((bool)flag);

    } else if (key == "$envmap") {
      if (value == "env_cubemap") {
        set_env_cubemap(true);

      } else {
        set_envmap_texture(TexturePool::load_texture(value));
      }

    } else if (key == "$roughness") {
      PN_stdfloat rough;
      string_to_stdfloat(value, rough);
      set_roughness(rough);

    } else if (key == "$metalness") {
      PN_stdfloat metal;
      string_to_stdfloat(value, metal);
      set_metalness(metal);

    } else if (key == "$emission") {
      PN_stdfloat emission;
      string_to_stdfloat(value, emission);
      set_emission(emission);

    } else if (key == "$lightwarp" || key == "$lightwarptexture") {
      set_lightwarp_texture(TexturePool::load_texture(value));
    }
  }
}

/**
 *
 */
MaterialBase *StandardMaterial::
create_StandardMaterial() {
  return new StandardMaterial;
}
