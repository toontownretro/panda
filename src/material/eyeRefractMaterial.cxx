/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eyeRefractMaterial.cxx
 * @author lachbr
 * @date 2021-03-24
 */

#include "eyeRefractMaterial.h"
#include "string_utils.h"
#include "materialParamTexture.h"
#include "materialParamBool.h"
#include "materialParamColor.h"
#include "materialParamFloat.h"
#include "pdxElement.h"

TypeHandle EyeRefractMaterial::_type_handle;

/**
 *
 */
EyeRefractMaterial::
EyeRefractMaterial(const std::string &name) :
  Material(name)
{
}

/**
 *
 */
void EyeRefractMaterial::
read_pdx(PDXElement *data, const DSearchPath &search_path) {
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

    if (key == "iris_texture") {
      param = new MaterialParamTexture(key);

    } else if (key == "cornea_texture") {
      param = new MaterialParamTexture(key);

    } else if (key == "ambient_occl_texture") {
      param = new MaterialParamTexture(key);

    } else if (key == "env_map") {
      param = new MaterialParamTexture(key);

    } else if (key == "lightwarp_texture") {
      param = new MaterialParamTexture(key);

    } else if (key == "glossiness") {
      param = new MaterialParamFloat(key);

    } else if (key == "sphere_texkill_combo") {
      param = new MaterialParamBool(key);

    } else if (key == "ray_trace_sphere") {
      param = new MaterialParamBool(key);

    } else if (key == "parallax_strength") {
      param = new MaterialParamFloat(key);

    } else if (key == "cornea_bump_strength") {
      param = new MaterialParamFloat(key);

    } else if (key == "ambient_occl_color") {
      param = new MaterialParamColor(key);

    } else if (key == "eyeball_radius") {
      param = new MaterialParamFloat(key);

    } else if (key == "dilation") {
      param = new MaterialParamFloat(key);
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
void EyeRefractMaterial::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 *
 */
TypedWritable *EyeRefractMaterial::
make_from_bam(const FactoryParams &params) {
  EyeRefractMaterial *mat = new EyeRefractMaterial;
  DatagramIterator scan;
  BamReader *manager;
  parse_params(params, scan, manager);

  mat->fillin(scan, manager);
  return mat;
}

/**
 *
 */
Material *EyeRefractMaterial::
create_EyeRefractMaterial() {
  return new EyeRefractMaterial;
}
