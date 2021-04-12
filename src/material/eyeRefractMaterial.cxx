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
#include "materialParamVector.h"
#include "materialParamFloat.h"

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
read_keyvalues(KeyValues *kv, const DSearchPath &search_path) {
  for (size_t i = 0; i < kv->get_num_keys(); i++) {
    std::string key = downcase(kv->get_key(i));
    const std::string &value = kv->get_value(i);

    if (key == "$iris") {
      PT(MaterialParamTexture) tex = new MaterialParamTexture("$iris");
      tex->from_string(value, search_path);
      set_param(tex);

    } else if (key == "$corneatexture") {
      PT(MaterialParamTexture) tex = new MaterialParamTexture("$corneatexture");
      tex->from_string(value, search_path);
      set_param(tex);

    } else if (key == "$ambientoccltexture") {
      PT(MaterialParamTexture) tex = new MaterialParamTexture("$ambientoccltexture");
      tex->from_string(value, search_path);
      set_param(tex);

    } else if (key == "$envmap") {
      PT(MaterialParamTexture) tex = new MaterialParamTexture("$envmap");
      tex->from_string(value, search_path);
      set_param(tex);

    } else if (key == "$lightwarptexture") {
      PT(MaterialParamTexture) tex = new MaterialParamTexture("$lightwarptexture");
      tex->from_string(value, search_path);
      set_param(tex);

    } else if (key == "$glossiness") {
      PT(MaterialParamFloat) gloss = new MaterialParamFloat("$glossiness");
      gloss->from_string(value, search_path);
      set_param(gloss);

    } else if (key == "$spheretexkillcombo") {
      PT(MaterialParamBool) param = new MaterialParamBool("$spheretexkillcombo");
      param->from_string(value, search_path);
      set_param(param);

    } else if (key == "$raytracesphere") {
      PT(MaterialParamBool) param = new MaterialParamBool("$raytracesphere");
      param->from_string(value, search_path);
      set_param(param);

    } else if (key == "$parallaxstrength") {
      PT(MaterialParamFloat) str = new MaterialParamFloat("$parallaxstrength");
      str->from_string(value, search_path);
      set_param(str);

    } else if (key == "$corneabumpstrength") {
      PT(MaterialParamFloat) str = new MaterialParamFloat("$corneabumpstrength");
      str->from_string(value, search_path);
      set_param(str);

    } else if (key == "$ambientocclcolor") {
      PT(MaterialParamVector) col = new MaterialParamVector("$ambientocclcolor");
      col->from_string(value, search_path);
      set_param(col);

    } else if (key == "$eyeballradius") {
      PT(MaterialParamFloat) rad = new MaterialParamFloat("$eyeballradius");
      rad->from_string(value, search_path);
      set_param(rad);

    } else if (key == "$dilation") {
      PT(MaterialParamFloat) dil = new MaterialParamFloat("$dilation");
      dil->from_string(value, search_path);
      set_param(dil);
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
