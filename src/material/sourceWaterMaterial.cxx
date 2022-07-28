/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sourceWaterMaterial.cxx
 * @author brian
 * @date 2022-07-10
 */

#include "sourceWaterMaterial.h"

#include "pdxValue.h"
#include "pdxElement.h"
#include "materialParamBool.h"
#include "materialParamTexture.h"
#include "materialParamVector.h"
#include "materialParamMatrix.h"

TypeHandle SourceWaterMaterial::_type_handle;

/**
 *
 */
SourceWaterMaterial::
SourceWaterMaterial(const std::string &name) :
  Material(name)
{
}

/**
 *
 */
void SourceWaterMaterial::
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

    if (param != nullptr) {
      param->from_pdx(val, search_path);
      set_param(param);
    }
  }
}

/**
 *
 */
void SourceWaterMaterial::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 *
 */
TypedWritable *SourceWaterMaterial::
make_from_bam(const FactoryParams &params) {
  SourceWaterMaterial *mat = new SourceWaterMaterial;
  DatagramIterator scan;
  BamReader *manager;
  parse_params(params, scan, manager);

  mat->fillin(scan, manager);
  return mat;
}

/**
 *
 */
Material *SourceWaterMaterial::
create_SourceWaterMaterial() {
  return new SourceWaterMaterial;
}

