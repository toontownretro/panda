/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pbrMaterial.cxx
 * @author brian
 * @date 2024-08-30
 */

#include "pbrMaterial.h"
#include "pdxValue.h"
#include "pdxElement.h"
#include "materialParamTexture.h"
#include "materialParamFloat.h"
#include "materialParamColor.h"
#include "materialParamBool.h"

TypeHandle PBRMaterial::_type_handle;

/**
 *
 */
PBRMaterial::
PBRMaterial(const std::string &name) :
  Material(name)
{
}

/**
 *
 */
void PBRMaterial::
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
    const PDXValue &val = params->get_attribute_value(i);

    PT(MaterialParamBase) param;

    // Texture types
    if (key == "albedo") {
      param = new MaterialParamTexture("base_color");

    } else if (key == "normal" ||
               key == "roughness" ||
               key == "metalness" ||
               key == "ao" ||
               key == "height" ||
               key == "emission") {
      param = new MaterialParamTexture(key);

    } else if (key == "roughness_scale" ||
               key == "normal_scale" ||
               key == "ao_scale" ||
               key == "emission_scale") {
      param = new MaterialParamFloat(key);

    } else if (key == "albedo_val") {
      param = new MaterialParamColor("base_color");

    } else if (key == "envmap") {
      // Special handler for envmap.  A string means it's a texture path to an
      // explicit cubemap, bool is whether or not to use closest cube map
      // (or none at all).
      if (val.get_value_type() == PDXValue::VT_string) {
        param = new MaterialParamTexture(key);

      } else {
        param = new MaterialParamBool(key);
      }
    }

    if (param != nullptr) {
      param->from_pdx(val, search_path);
      set_param(param);
    }
  }
}

/**
 *
 */
void PBRMaterial::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 *
 */
TypedWritable *PBRMaterial::
make_from_bam(const FactoryParams &params) {
  PBRMaterial *mat = new PBRMaterial;
  DatagramIterator scan;
  BamReader *manager;
  parse_params(params, scan, manager);

  mat->fillin(scan, manager);
  return mat;
}

/**
 *
 */
Material *PBRMaterial::
create_PBRMaterial() {
  return new PBRMaterial;
}
