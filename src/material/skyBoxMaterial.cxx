/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file skyBoxMaterial.cxx
 * @author brian
 * @date 2021-07-13
 */

#include "skyBoxMaterial.h"
#include "pdxElement.h"
#include "materialParamTexture.h"

TypeHandle SkyBoxMaterial::_type_handle;

/**
 *
 */
SkyBoxMaterial::
SkyBoxMaterial(const std::string &name) :
  Material(name)
{
}

/**
 * Returns the skybox cube map texture.
 */
Texture *SkyBoxMaterial::
get_sky_cube_map() const {
  MaterialParamBase *param = get_param("sky_cube_map");
  if (param == nullptr ||
      !param->is_exact_type(MaterialParamTexture::get_class_type())) {
    return nullptr;
  }
  return DCAST(MaterialParamTexture, param)->get_value();
}

/**
 *
 */
void SkyBoxMaterial::
read_pdx(PDXElement *data, const DSearchPath &search_path) {
  Material::read_pdx(data, search_path);

  if (!data->has_attribute("parameters")) {
    return;
  }

  PDXElement *params = data->get_attribute_value("parameters").get_element();
  if (params == nullptr) {
    return;
  }

  if (params->has_attribute("sky_cube_map")) {
    PT(MaterialParamTexture) tex_p = new MaterialParamTexture("sky_cube_map");
    tex_p->from_pdx(params->get_attribute_value("sky_cube_map"), search_path);
    set_param(tex_p);
  }
}

/**
 *
 */
void SkyBoxMaterial::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 *
 */
TypedWritable *SkyBoxMaterial::
make_from_bam(const FactoryParams &params) {
  SkyBoxMaterial *mat = new SkyBoxMaterial;
  DatagramIterator scan;
  BamReader *manager;
  parse_params(params, scan, manager);

  mat->fillin(scan, manager);
  return mat;
}

/**
 *
 */
Material *SkyBoxMaterial::
create_SkyBoxMaterial() {
  return new SkyBoxMaterial;
}
