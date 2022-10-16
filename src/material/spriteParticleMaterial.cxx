/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spriteParticleMaterial.cxx
 * @author brian
 * @date 2021-09-01
 */

#include "spriteParticleMaterial.h"
#include "bamReader.h"
#include "pdxValue.h"
#include "pdxElement.h"
#include "materialParamTexture.h"
#include "materialParamFloat.h"
#include "materialParamBool.h"

TypeHandle SpriteParticleMaterial::_type_handle;

/**
 *
 */
SpriteParticleMaterial::
SpriteParticleMaterial(const std::string &name) :
  Material(name)
{
}

/**
 *
 */
void SpriteParticleMaterial::
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
    if (key == "base_texture") {
      param = new MaterialParamTexture(key);
    } else if (key == "x_size" || key == "y_size" ||
               key == "num_frames_per_anim") {
      param = new MaterialParamFloat(key);
    } else if (key == "animated" || "anim_interp") {
      param = new MaterialParamBool(key);
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
void SpriteParticleMaterial::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 *
 */
TypedWritable *SpriteParticleMaterial::
make_from_bam(const FactoryParams &params) {
  SpriteParticleMaterial *mat = new SpriteParticleMaterial;
  DatagramIterator scan;
  BamReader *manager;
  parse_params(params, scan, manager);

  mat->fillin(scan, manager);
  return mat;
}

/**
 *
 */
Material *SpriteParticleMaterial::
create_SpriteParticleMaterial() {
  return new SpriteParticleMaterial;
}
