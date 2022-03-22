/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file twoTextureMaterial.cxx
 * @author brian
 * @date 2022-03-22
 */

#include "twoTextureMaterial.h"
#include "pdxElement.h"
#include "materialParamMatrix.h"
#include "materialParamFloat.h"
#include "materialParamTexture.h"
#include "materialParamVector.h"
#include "materialParamColor.h"

TypeHandle TwoTextureMaterial::_type_handle;

/**
 *
 */
TwoTextureMaterial::
TwoTextureMaterial(const std::string &name) :
  Material(name)
{
}

/**
 *
 */
void TwoTextureMaterial::
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

    if (key == "basetexture" || key == "texture2") {
      if (key == "basetexture") {
        key = "base_color";
      }
      param = new MaterialParamTexture(key);

    } else if (key == "texture2transform" ||
               key == "basetexturetransform") {
      param = new MaterialParamMatrix(key);

    } else if (key == "basetexturescroll" ||
               key == "texture2scroll" ||
               key == "basetexturesinex" ||
               key == "basetexturesiney") {
      param = new MaterialParamVector(key);
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
void TwoTextureMaterial::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 *
 */
TypedWritable *TwoTextureMaterial::
make_from_bam(const FactoryParams &params) {
  TwoTextureMaterial *mat = new TwoTextureMaterial;
  DatagramIterator scan;
  BamReader *manager;
  parse_params(params, scan, manager);

  mat->fillin(scan, manager);
  return mat;
}

/**
 *
 */
Material *TwoTextureMaterial::
create_TwoTextureMaterial() {
  return new TwoTextureMaterial;
}

