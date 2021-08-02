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
#include "pdxElement.h"
#include "string_utils.h"
#include "texturePool.h"
#include "materialParamTexture.h"
#include "materialParamBool.h"
#include "materialParamFloat.h"
#include "materialParamColor.h"
#include "materialParamVector.h"
#include "bamReader.h"
#include "string_utils.h"

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
read_pdx(PDXElement *data, const DSearchPath &search_path) {
  Material::read_pdx(data, search_path);

  if (!data->has_attribute("parameters")) {
    return;
  }

  PDXElement *params = data->get_attribute_value("parameters").get_element();
  if (params == nullptr) {
    return;
  }

  for (size_t i = 0; i < params->get_num_attributes(); i++) {
    std::string key = downcase(params->get_attribute_name(i));
    const PDXValue &value = params->get_attribute_value(i);

    PT(MaterialParamBase) param;

    if (key == "base_texture" || key == "base_map" || key == "base_color") {

      if (value.get_value_type() == PDXValue::VT_string) {
        param = new MaterialParamTexture("base_color");
      } else {
        param = new MaterialParamColor("base_color");
      }

    } else if (key == "normal" || key == "bump_map" ||
               key == "normal_map" || key == "normal_texture") {
      param = new MaterialParamTexture("normal_map");

    } else if (key == "specular_map" || key == "specular_texture") {
      param = new MaterialParamTexture("specular_texture");

    } else if (key == "ao_texture") {
      param = new MaterialParamTexture(key);

    } else if (key == "roughness_texture") {
      param = new MaterialParamTexture(key);

    } else if (key == "gloss_texture") {
      param = new MaterialParamTexture(key);

    } else if (key == "metalness_texture") {
      param = new MaterialParamTexture(key);

    } else if (key == "emission_texture") {
      param = new MaterialParamTexture(key);

    } else if (key == "half_lambert") {
      param = new MaterialParamBool(key);

    } else if (key == "rim_lighting") {
      param = new MaterialParamBool(key);

    } else if (key == "rim_lighting_boost") {
      param = new MaterialParamFloat(key);

    } else if (key == "rim_lighting_exponent") {
      param = new MaterialParamFloat(key);

    } else if (key == "self_illum") {
      param = new MaterialParamBool(key);

    } else if (key == "self_illum_tint") {
      param = new MaterialParamColor(key);

    } else if (key == "env_map") {
      if (value.get_value_type() == PDXValue::VT_string) {
        param = new MaterialParamTexture(key);

      } else {
        param = new MaterialParamBool(key);
      }

    } else if (key == "roughness") {
      param = new MaterialParamFloat(key);

    } else if (key == "metalness") {
      param = new MaterialParamFloat(key);

    } else if (key == "emission") {
      param = new MaterialParamFloat(key);

    } else if (key == "lightwarp" || key == "lightwarp_texture") {
      param = new MaterialParamTexture("lightwarp");
    }

    if (param != nullptr) {
      param->from_pdx(value, search_path);
      set_param(param);
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
  set_param(new MaterialParamBool("rim_lighting", rim_light));
}

/**
 *
 */
bool StandardMaterial::
get_rim_light() const {
  MaterialParamBase *param = get_param("rim_lighting");
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
  set_param(new MaterialParamFloat("rim_lighting_boost", boost));
}

/**
 *
 */
PN_stdfloat StandardMaterial::
get_rim_light_boost() const {
  MaterialParamBase *param = get_param("rim_lighting_boost");
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
  set_param(new MaterialParamFloat("rim_lighting_exponent", exponent));
}

/**
 *
 */
PN_stdfloat StandardMaterial::
get_rim_light_exponent() const {
  MaterialParamBase *param = get_param("rim_lighting_exponent");
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
  set_param(new MaterialParamBool("half_lambert", flag));
}

/**
 *
 */
bool StandardMaterial::
get_half_lambert() const {
  MaterialParamBase *param = get_param("half_lambert");
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
  set_param(new MaterialParamTexture("base_color", tex));
}

/**
 *
 */
Texture *StandardMaterial::
get_base_texture() const {
  MaterialParamBase *param = get_param("base_color");
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
  set_param(new MaterialParamColor("base_color", color));
}

/**
 *
 */
LColor StandardMaterial::
get_base_color() const {
  MaterialParamBase *param = get_param("base_color");
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
  set_param(new MaterialParamTexture("normal_map", tex));
}

/**
 *
 */
Texture *StandardMaterial::
get_normal_texture() const {
  MaterialParamBase *param = get_param("normal_map");
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
  set_param(new MaterialParamTexture("lightwarp", tex));
}

/**
 *
 */
Texture *StandardMaterial::
get_lightwarp_texture() const {
  MaterialParamBase *param = get_param("lightwarp");
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
  set_param(new MaterialParamTexture("env_map", tex));
}

/**
 *
 */
Texture *StandardMaterial::
get_envmap_texture() const {
  MaterialParamBase *param = get_param("env_map");
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
  set_param(new MaterialParamBool("env_map", flag));
}

/**
 *
 */
bool StandardMaterial::
get_env_cubemap() const {
  MaterialParamBase *param = get_param("env_map");
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
  set_param(new MaterialParamBool("self_illum", enabled));
}

/**
 *
 */
bool StandardMaterial::
get_emission_enabled() const {
  MaterialParamBase *param = get_param("self_illum");
  if (param == nullptr) {
    return false;
  }

  return DCAST(MaterialParamBool, param)->get_value();
}

/**
 *
 */
void StandardMaterial::
set_ambient_occlusion(Texture *tex) {
  set_param(new MaterialParamTexture("ao_texture", tex));
}

/**
 *
 */
Texture *StandardMaterial::
get_ambient_occlusion() const {
  MaterialParamBase *param = get_param("ao_texture");
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
  set_param(new MaterialParamFloat("roughness", roughness));
}

/**
 *
 */
void StandardMaterial::
set_roughness(Texture *tex) {
  set_param(new MaterialParamTexture("roughness_texture", tex));
}

/**
 *
 */
Texture *StandardMaterial::
get_roughness_texture() const {
  MaterialParamBase *param = get_param("roughness_texture");
  if (param == nullptr) {
    return nullptr;
  }

  return DCAST(MaterialParamTexture, param)->get_value();
}

/**
 *
 */
PN_stdfloat StandardMaterial::
get_roughness() const {
  MaterialParamBase *param = get_param("roughness");
  if (param == nullptr) {
    return 1.0f;
  }

  return DCAST(MaterialParamFloat, param)->get_value();
}

/**
 *
 */
void StandardMaterial::
set_glossiness(Texture *tex) {
  set_param(new MaterialParamTexture("gloss_texture", tex));
}

/**
 *
 */
Texture *StandardMaterial::
get_glossiness() const {
  MaterialParamBase *param = get_param("gloss_texture");
  if (param == nullptr) {
    return nullptr;
  }

  return DCAST(MaterialParamTexture, param)->get_value();
}

/**
 *
 */
void StandardMaterial::
set_metalness(PN_stdfloat metalness) {
  set_param(new MaterialParamFloat("metalness", metalness));
}

/**
 *
 */
void StandardMaterial::
set_metalness(Texture *tex) {
  set_param(new MaterialParamTexture("metalness_texture", tex));
}

/**
 *
 */
Texture *StandardMaterial::
get_metalness_texture() const {
  MaterialParamBase *param = get_param("metalness_texture");
  if (param == nullptr) {
    return nullptr;
  }

  return DCAST(MaterialParamTexture, param)->get_value();
}

/**
 *
 */
PN_stdfloat StandardMaterial::
get_metalness() const {
  MaterialParamBase *param = get_param("metalness");
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
  set_param(new MaterialParamFloat("emission", emission));
}

/**
 *
 */
void StandardMaterial::
set_emission(Texture *tex) {
  set_param(new MaterialParamTexture("emission_texture", tex));
}

/**
 *
 */
Texture *StandardMaterial::
get_emission_texture() const {
  MaterialParamBase *param = get_param("emission_texture");
  if (param == nullptr) {
    return nullptr;
  }

  return DCAST(MaterialParamTexture, param)->get_value();
}

/**
 *
 */
PN_stdfloat StandardMaterial::
get_emission() const {
  MaterialParamBase *param = get_param("emission");
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
  set_param(new MaterialParamVector("self_illum_tint", tint));
}

/**
 *
 */
LVecBase3 StandardMaterial::
get_emission_tint() const {
  MaterialParamBase *param = get_param("self_illum_tint");
  if (param == nullptr) {
    return LVecBase3(1);
  }

  return DCAST(MaterialParamVector, param)->get_value();
}

/**
 *
 */
void StandardMaterial::
set_specular_texture(Texture *texture) {
  set_param(new MaterialParamTexture("specular_texture", texture));
}

/**
 *
 */
Texture *StandardMaterial::
get_specular_texture() const {
  MaterialParamBase *param = get_param("specular_texture");
  if (param == nullptr) {
    return nullptr;
  }

  return DCAST(MaterialParamTexture, param)->get_value();
}
