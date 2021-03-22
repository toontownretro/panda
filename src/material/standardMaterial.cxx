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
#include "materialParamTexture.h"
#include "materialParamBool.h"
#include "materialParamFloat.h"
#include "materialParamColor.h"
#include "materialParamVector.h"
#include "bamReader.h"

TypeHandle StandardMaterial::_type_handle;

/**
 *
 */
StandardMaterial::
StandardMaterial(const std::string &name) :
  Material(name)
{
}

/**
 *
 */
void StandardMaterial::
read_keyvalues(KeyValues *kv, const DSearchPath &search_path) {
  for (size_t i = 0; i < kv->get_num_keys(); i++) {
    std::string key = downcase(kv->get_key(i));
    const std::string &value = kv->get_value(i);

    if (key == "$basetexture" || key == "$basemap") {
      PT(MaterialParamTexture) tex = new MaterialParamTexture("$basecolor");
      tex->from_string(value, search_path);
      set_param(tex);

    } else if (key == "$basecolor") {
      PT(MaterialParamColor) color = new MaterialParamColor("$basecolor");
      color->from_string(value, search_path);
      set_param(color);

    } else if (key == "$normal" || key == "$bumpmap" ||
               key == "$normalmap" || key == "$normaltexture") {
      PT(MaterialParamTexture) tex = new MaterialParamTexture("$normalmap");
      tex->from_string(value, search_path);
      set_param(tex);

    } else if (key == "$arme" || key == "$armemap" || key == "$armetexture") {
      PT(MaterialParamTexture) tex = new MaterialParamTexture("$armetexture");
      tex->from_string(value, search_path);
      set_param(tex);

    } else if (key == "$halflambert") {
      PT(MaterialParamBool) hl = new MaterialParamBool("$halflambert");
      hl->from_string(value, search_path);
      set_param(hl);

    } else if (key == "$rimlight") {
      PT(MaterialParamBool) rl = new MaterialParamBool("$rimlight");
      rl->from_string(value, search_path);
      set_param(rl);

    } else if (key == "$rimlightboost") {
      PT(MaterialParamFloat) rlb = new MaterialParamFloat("$rimlightboost");
      rlb->from_string(value, search_path);
      set_param(rlb);

    } else if (key == "$rimlightexponent") {
      PT(MaterialParamFloat) rle = new MaterialParamFloat("$rimlightexponent");
      rle->from_string(value, search_path);
      set_param(rle);

    } else if (key == "$selfillum") {
      PT(MaterialParamBool) si = new MaterialParamBool("$selfillum");
      si->from_string(value, search_path);
      set_param(si);

    } else if (key == "$selfillumtint") {
      PT(MaterialParamVector) sit = new MaterialParamVector("$selfillumtint");
      sit->from_string(value, search_path);
      set_param(sit);

    } else if (key == "$envmap") {
      if (value == "env_cubemap") {
        PT(MaterialParamBool) ecm = new MaterialParamBool("$envmap", true);
        set_param(ecm);

      } else {
        PT(MaterialParamTexture) tex = new MaterialParamTexture("$envmap");
        tex->from_string(value, search_path);
        set_param(tex);
      }

    } else if (key == "$roughness") {
      PT(MaterialParamFloat) rough = new MaterialParamFloat("$roughness");
      rough->from_string(value, search_path);
      set_param(rough);

    } else if (key == "$metalness") {
      PT(MaterialParamFloat) metal = new MaterialParamFloat("$metalness");
      metal->from_string(value, search_path);
      set_param(metal);

    } else if (key == "$emission") {
      PT(MaterialParamFloat) emit = new MaterialParamFloat("$emission");
      emit->from_string(value, search_path);
      set_param(emit);

    } else if (key == "$lightwarp" || key == "$lightwarptexture") {
      PT(MaterialParamTexture) tex = new MaterialParamTexture("$lightwarp");
      tex->from_string(value, search_path);
      set_param(tex);
    }
  }
}

/**
 *
 */
void StandardMaterial::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 *
 */
TypedWritable *StandardMaterial::
make_from_bam(const FactoryParams &params) {
  StandardMaterial *mat = new StandardMaterial;
  DatagramIterator scan;
  BamReader *manager;
  parse_params(params, scan, manager);

  mat->fillin(scan, manager);
  return mat;
}

/**
 *
 */
Material *StandardMaterial::
create_StandardMaterial() {
  return new StandardMaterial;
}


/**
 *
 */
void StandardMaterial::
set_rim_light(bool rim_light) {
  set_param(new MaterialParamBool("$rimlight", rim_light));
}

/**
 *
 */
bool StandardMaterial::
get_rim_light() const {
  MaterialParamBase *param = get_param("$rimlight");
  if (param == nullptr) {
    return false;
  }

  return DCAST(MaterialParamBool, param)->get_value();
}

/**
 *
 */
void StandardMaterial::
set_rim_light_boost(PN_stdfloat boost) {
  set_param(new MaterialParamFloat("$rimlightboost", boost));
}

/**
 *
 */
PN_stdfloat StandardMaterial::
get_rim_light_boost() const {
  MaterialParamBase *param = get_param("$rimlightboost");
  if (param == nullptr) {
    return 2.0f;
  }

  return DCAST(MaterialParamFloat, param)->get_value();
}

/**
 *
 */
void StandardMaterial::
set_rim_light_exponent(PN_stdfloat exponent) {
  set_param(new MaterialParamFloat("$rimlightexponent", exponent));
}

/**
 *
 */
PN_stdfloat StandardMaterial::
get_rim_light_exponent() const {
  MaterialParamBase *param = get_param("$rimlightexponent");
  if (param == nullptr) {
    return 4.0f;
  }

  return DCAST(MaterialParamFloat, param)->get_value();
}

/**
 *
 */
void StandardMaterial::
set_half_lambert(bool flag) {
  set_param(new MaterialParamBool("$halflambert", flag));
}

/**
 *
 */
bool StandardMaterial::
get_half_lambert() const {
  MaterialParamBase *param = get_param("$halflambert");
  if (param == nullptr) {
    return false;
  }

  return DCAST(MaterialParamBool, param)->get_value();
}

/**
 *
 */
void StandardMaterial::
set_base_texture(Texture *tex) {
  set_param(new MaterialParamTexture("$basecolor", tex));
}

/**
 *
 */
Texture *StandardMaterial::
get_base_texture() const {
  MaterialParamBase *param = get_param("$basecolor");
  if (param == nullptr || !param->is_of_type(MaterialParamTexture::get_class_type())) {
    return nullptr;
  }

  return DCAST(MaterialParamTexture, param)->get_value();
}

/**
 *
 */
void StandardMaterial::
set_base_color(const LColor &color) {
  set_param(new MaterialParamColor("$basecolor", color));
}

/**
 *
 */
LColor StandardMaterial::
get_base_color() const {
  MaterialParamBase *param = get_param("$basecolor");
  if (param == nullptr || !param->is_of_type(MaterialParamColor::get_class_type())) {
    return LColor(1, 1, 1, 1);
  }

  return DCAST(MaterialParamColor, param)->get_value();
}

/**
 *
 */
void StandardMaterial::
set_normal_texture(Texture *tex) {
  set_param(new MaterialParamTexture("$normalmap", tex));
}

/**
 *
 */
Texture *StandardMaterial::
get_normal_texture() const {
  MaterialParamBase *param = get_param("$normalmap");
  if (param == nullptr) {
    return nullptr;
  }

  return DCAST(MaterialParamTexture, param)->get_value();
}

/**
 *
 */
void StandardMaterial::
set_lightwarp_texture(Texture *tex) {
  set_param(new MaterialParamTexture("$lightwarp", tex));
}

/**
 *
 */
Texture *StandardMaterial::
get_lightwarp_texture() const {
  MaterialParamBase *param = get_param("$lightwarp");
  if (param == nullptr) {
    return nullptr;
  }

  return DCAST(MaterialParamTexture, param)->get_value();
}

/**
 *
 */
void StandardMaterial::
set_envmap_texture(Texture *tex) {
  set_param(new MaterialParamTexture("$envmap", tex));
}

/**
 *
 */
Texture *StandardMaterial::
get_envmap_texture() const {
  MaterialParamBase *param = get_param("$envmap");
  if (param == nullptr || !param->is_of_type(MaterialParamTexture::get_class_type())) {
    return nullptr;
  }

  return DCAST(MaterialParamTexture, param)->get_value();
}

/**
 *
 */
void StandardMaterial::
set_env_cubemap(bool flag) {
  set_param(new MaterialParamBool("$envmap", flag));
}

/**
 *
 */
bool StandardMaterial::
get_env_cubemap() const {
  MaterialParamBase *param = get_param("$envmap");
  if (param == nullptr || !param->is_of_type(MaterialParamBool::get_class_type())) {
    return false;
  }

  return DCAST(MaterialParamBool, param)->get_value();
}

/**
 *
 */
void StandardMaterial::
set_emission_enabled(bool enabled) {
  set_param(new MaterialParamBool("$selfillum", enabled));
}

/**
 *
 */
bool StandardMaterial::
get_emission_enabled() const {
  MaterialParamBase *param = get_param("$selfillum");
  if (param == nullptr) {
    return false;
  }

  return DCAST(MaterialParamBool, param)->get_value();
}

/**
 *
 */
void StandardMaterial::
set_arme_texture(Texture *tex) {
  set_param(new MaterialParamTexture("$armetexture", tex));
}

/**
 *
 */
Texture *StandardMaterial::
get_arme_texture() const {
  MaterialParamBase *param = get_param("$armetexture");
  if (param == nullptr) {
    return nullptr;
  }

  return DCAST(MaterialParamTexture, param)->get_value();
}

/**
 *
 */
void StandardMaterial::
set_roughness(PN_stdfloat roughness) {
  set_param(new MaterialParamFloat("$roughness", roughness));
}

/**
 *
 */
PN_stdfloat StandardMaterial::
get_roughness() const {
  MaterialParamBase *param = get_param("$roughness");
  if (param == nullptr) {
    return 1.0f;
  }

  return DCAST(MaterialParamFloat, param)->get_value();
}

/**
 *
 */
void StandardMaterial::
set_metalness(PN_stdfloat metalness) {
  set_param(new MaterialParamFloat("$metalness", metalness));
}

/**
 *
 */
PN_stdfloat StandardMaterial::
get_metalness() const {
  MaterialParamBase *param = get_param("$metalness");
  if (param == nullptr) {
    return 0.0f;
  }

  return DCAST(MaterialParamFloat, param)->get_value();
}

/**
 *
 */
void StandardMaterial::
set_emission(PN_stdfloat emission) {
  set_param(new MaterialParamFloat("$emission", emission));
}

/**
 *
 */
PN_stdfloat StandardMaterial::
get_emission() const {
  MaterialParamBase *param = get_param("$emission");
  if (param == nullptr) {
    return 0.0f;
  }

  return DCAST(MaterialParamFloat, param)->get_value();
}

/**
 *
 */
void StandardMaterial::
set_emission_tint(const LVecBase3 &tint) {
  set_param(new MaterialParamVector("$selfillumtint", tint));
}

/**
 *
 */
LVecBase3 StandardMaterial::
get_emission_tint() const {
  MaterialParamBase *param = get_param("$selfillumtint");
  if (param == nullptr) {
    return LVecBase3(1);
  }

  return DCAST(MaterialParamVector, param)->get_value();
}
