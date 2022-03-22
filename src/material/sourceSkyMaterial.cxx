/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sourceSkyMaterial.cxx
 * @author brian
 * @date 2022-02-15
 */

#include "sourceSkyMaterial.h"

#include "pdxValue.h"
#include "pdxElement.h"
#include "materialParamBool.h"
#include "materialParamTexture.h"
#include "materialParamVector.h"
#include "materialParamMatrix.h"

TypeHandle SourceSkyMaterial::_type_handle;

/**
 *
 */
SourceSkyMaterial::
SourceSkyMaterial(const std::string &name) :
  Material(name)
{
}

/**
 *
 */
void SourceSkyMaterial::
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
    if (key == "sky_texture") {
      param = new MaterialParamTexture(key);

    } else if (key == "compressed_hdr") {
      param = new MaterialParamBool(key);

    } else if (key == "texcoord_transform") {
      param = new MaterialParamMatrix(key);
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
void SourceSkyMaterial::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 *
 */
TypedWritable *SourceSkyMaterial::
make_from_bam(const FactoryParams &params) {
  SourceSkyMaterial *mat = new SourceSkyMaterial;
  DatagramIterator scan;
  BamReader *manager;
  parse_params(params, scan, manager);

  mat->fillin(scan, manager);
  return mat;
}

/**
 *
 */
Material *SourceSkyMaterial::
create_SourceSkyMaterial() {
  return new SourceSkyMaterial;
}

