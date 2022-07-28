/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sourceLightmappedMaterial.cxx
 * @author brian
 * @date 2022-03-17
 */

#include "sourceLightmappedMaterial.h"

#include "pdxValue.h"
#include "pdxElement.h"
#include "materialParamBool.h"
#include "materialParamTexture.h"
#include "materialParamVector.h"
#include "materialParamColor.h"
#include "materialParamFloat.h"

TypeHandle SourceLightmappedMaterial::_type_handle;

/**
 *
 */
SourceLightmappedMaterial::
SourceLightmappedMaterial(const std::string &name) :
  Material(name)
{
}

/**
 *
 */
void SourceLightmappedMaterial::
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
    if (key == "basetexture" || key == "bumpmap" ||
        key == "basetexture2" || key == "bumpmap2" ||
        key == "envmapmask" ||
        key == "albedo") {
      if (key == "albedo" || key == "basetexture") {
        key = "base_color";
      }
      param = new MaterialParamTexture(key);

    // Bool types
    } else if (key == "selfillum" ||
               key == "basealphaenvmapmask" ||
               key == "normalmapalphaenvmapmask" ||
               key == "ssbump" ||
               key == "planarreflection") {
      param = new MaterialParamBool(key);

    // Float types
    } else if (key == "envmapcontrast" ||
               key == "envmapsaturation") {
      param = new MaterialParamFloat(key);

    // Vec3 types
    } else if (key == "selfillumtint" ||
               key == "envmaptint") {
      param = new MaterialParamVector(key);

    // Vec4 types, not necessarily color
    //} else if (key == "selfillumfresnelminmaxexp") {
    //  param = new MaterialParamColor(key);

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
void SourceLightmappedMaterial::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 *
 */
TypedWritable *SourceLightmappedMaterial::
make_from_bam(const FactoryParams &params) {
  SourceLightmappedMaterial *mat = new SourceLightmappedMaterial;
  DatagramIterator scan;
  BamReader *manager;
  parse_params(params, scan, manager);

  mat->fillin(scan, manager);
  return mat;
}

/**
 *
 */
Material *SourceLightmappedMaterial::
create_SourceLightmappedMaterial() {
  return new SourceLightmappedMaterial;
}
