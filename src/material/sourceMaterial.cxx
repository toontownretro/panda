/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sourceMaterial.cxx
 * @author brian
 * @date 2021-10-25
 */

#include "sourceMaterial.h"
#include "pdxValue.h"
#include "pdxElement.h"
#include "materialParamBool.h"
#include "materialParamTexture.h"
#include "materialParamVector.h"
#include "materialParamColor.h"
#include "materialParamFloat.h"
#include "materialParamMatrix.h"
#include "materialParamInt.h"

TypeHandle SourceMaterial::_type_handle;

/**
 *
 */
SourceMaterial::
SourceMaterial(const std::string &name) :
  Material(name)
{
}

/**
 *
 */
void SourceMaterial::
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
        key == "lightwarptexture" ||
        key == "envmapmask" ||
        key == "phongwarptexture" ||
        key == "selfillummask" ||
        key == "phongexponenttexture" ||
        key == "albedo"||
        key == "detail") {
      if (key == "albedo" || key == "basetexture") {
        key = "base_color";
      }
      param = new MaterialParamTexture(key);

    // Bool types
    } else if (key == "selfillumfresnel" ||
               key == "phong" ||
               key == "phongalbedotint" ||
               key == "rimlight" ||
               key == "rimmask" ||
               key == "basemapalphaphongmask" ||
               key == "invertphongmask" ||
               key == "halflambert" ||
               key == "selfillum" ||
               key == "basealphaenvmapmask" ||
               key == "normalmapalphaenvmapmask") {
      param = new MaterialParamBool(key);

    // Float types
    } else if (key == "envmapcontrast" ||
               key == "envmapsaturation" ||
               key == "phongexponent" ||
               key == "phongboost" ||
               key == "envmapfresnel" ||
               key == "rimlightexponent" ||
               key == "rimlightboost" ||
               key == "phongexponentfactor" ||
               key == "detailblendfactor" ||
               key == "detailscale") {
      param = new MaterialParamFloat(key);

    // Vec3 types
    } else if (key == "selfillumtint" ||
               key == "envmaptint" ||
               key == "phongtint" ||
               key == "phongfresnelranges" ||
               key == "detailtint") {
      param = new MaterialParamVector(key);

    // Vec4 types, not necessarily color
    } else if (key == "selfillumfresnelminmaxexp") {
      param = new MaterialParamColor(key);

    } else if (key == "envmap") {
      // Special handler for envmap.  A string means it's a texture path to an
      // explicit cubemap, bool is whether or not to use closest cube map
      // (or none at all).
      if (val.get_value_type() == PDXValue::VT_string) {
        param = new MaterialParamTexture(key);

      } else {
        param = new MaterialParamBool(key);
      }

    } else if (key == "basetexturetransform") {
      param = new MaterialParamMatrix(key);

    // Int types
    } else if (key == "detailblendmode") {
      param = new MaterialParamInt(key);
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
void SourceMaterial::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 *
 */
TypedWritable *SourceMaterial::
make_from_bam(const FactoryParams &params) {
  SourceMaterial *mat = new SourceMaterial;
  DatagramIterator scan;
  BamReader *manager;
  parse_params(params, scan, manager);

  mat->fillin(scan, manager);
  return mat;
}

/**
 *
 */
Material *SourceMaterial::
create_SourceMaterial() {
  return new SourceMaterial;
}

