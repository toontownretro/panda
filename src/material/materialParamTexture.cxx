/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialParamTexture.cxx
 * @author lachbr
 * @date 2021-03-07
 */

#include "materialParamTexture.h"
#include "texturePool.h"
#include "virtualFileSystem.h"
#include "pdxElement.h"

TypeHandle MaterialParamTexture::_type_handle;

/**
 *
 */
bool MaterialParamTexture::
from_pdx(const PDXValue &val, const DSearchPath &search_path) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  Filename filename;

  if (val.get_value_type() == PDXValue::VT_string) {
    // Single texture pathname.  Defaults to view 0 and the Texture's
    // default SamplerState.
    filename = val.get_string();

  } else if (val.get_value_type() == PDXValue::VT_element) {
    // Texture pathname with additional optional parameters.

    PDXElement *element = val.get_element();

    nassertr(element->has_attribute("texture"), false);
    filename = element->get_attribute_value("texture").get_string();

    if (element->has_attribute("view")) {
      _view = element->get_attribute_value("view").get_int();
    }

    if (element->has_attribute("sampler")) {
      _has_sampler = true;

      PDXElement *samplere = element->get_attribute_value("sampler").get_element();
      nassertr(samplere != nullptr, false);

      if (samplere->has_attribute("wrap")) {
        SamplerState::WrapMode wm = SamplerState::string_wrap_mode(
          samplere->get_attribute_value("wrap").get_string());
        _sampler.set_wrap_u(wm);
        _sampler.set_wrap_v(wm);
        _sampler.set_wrap_w(wm);

      } else {
        if (samplere->has_attribute("wrap_u")) {
          SamplerState::WrapMode wm = SamplerState::string_wrap_mode(
            samplere->get_attribute_value("wrap_u").get_string());
          _sampler.set_wrap_u(wm);
        }
        if (samplere->has_attribute("wrap_v")) {
          SamplerState::WrapMode wm = SamplerState::string_wrap_mode(
            samplere->get_attribute_value("wrap_v").get_string());
          _sampler.set_wrap_v(wm);
        }
        if (samplere->has_attribute("wrap_w")) {
          SamplerState::WrapMode wm = SamplerState::string_wrap_mode(
            samplere->get_attribute_value("wrap_w").get_string());
          _sampler.set_wrap_w(wm);
        }
      }

      if (samplere->has_attribute("minfilter")) {
        SamplerState::FilterType ft = SamplerState::string_filter_type(
          samplere->get_attribute_value("minfilter").get_string());
        _sampler.set_minfilter(ft);
      }
      if (samplere->has_attribute("magfilter")) {
        SamplerState::FilterType ft = SamplerState::string_filter_type(
          samplere->get_attribute_value("magfilter").get_string());
        _sampler.set_magfilter(ft);
      }

      if (samplere->has_attribute("anisotropic")) {
        _sampler.set_anisotropic_degree(samplere->get_attribute_value("anisotropic").get_int());
      }

      if (samplere->has_attribute("border_color")) {
        LColorf color;
        samplere->get_attribute_value("border_color").to_vec4(color);
        color /= 255.0f;
        _sampler.set_border_color(color);
      }

      if (samplere->has_attribute("min_lod")) {
        _sampler.set_min_lod(samplere->get_attribute_value("min_lod").get_float());
      }
      if (samplere->has_attribute("max_lod")) {
        _sampler.set_max_lod(samplere->get_attribute_value("max_lod").get_float());
      }

      if (samplere->has_attribute("lod_bias")) {
        _sampler.set_lod_bias(samplere->get_attribute_value("lod_bias").get_float());
      }
    }

  } else {
    // Invalid PDXValue type for a texture parameter.
    return false;
  }

  if (!vfs->resolve_filename(filename, search_path)) {
    return false;
  }
  _value = TexturePool::load_texture(filename);

  if (_value == nullptr) {
    return false;
  }

  _view = std::min(_view, _value->get_num_views() - 1);

  return true;
}

/**
 *
 */
void MaterialParamTexture::
to_pdx(PDXValue &val, const Filename &filename) {
  if (_value == nullptr) {
    val.set_string("__null__");

  } else {
    Filename out_filename = filename;
    out_filename.make_canonical();
    Filename tex_filename = _value->get_fullpath();
    tex_filename.make_canonical();
    tex_filename.make_relative_to(out_filename.get_dirname());
    val.set_string(tex_filename.get_fullpath());
  }
}

/**
 *
 */
void MaterialParamTexture::
write_datagram(BamWriter *manager, Datagram &me) {
  MaterialParamBase::write_datagram(manager, me);

  manager->write_pointer(me, _value);

  me.add_uint8((uint8_t)_view);
  me.add_bool(_has_sampler);
  if (_has_sampler) {
    _sampler.write_datagram(me);
  }
}

/**
 *
 */
int MaterialParamTexture::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = MaterialParamBase::complete_pointers(p_list, manager);

  Texture *tex;
  DCAST_INTO_R(tex, p_list[pi++], pi);
  _value = tex;

  return pi;
}

/**
 *
 */
void MaterialParamTexture::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 *
 */
void MaterialParamTexture::
fillin(DatagramIterator &scan, BamReader *manager) {
  MaterialParamBase::fillin(scan, manager);

  manager->read_pointer(scan);

  _view = (int)scan.get_uint8();
  _has_sampler = scan.get_bool();
  if (_has_sampler) {
    _sampler.read_datagram(scan, manager);
  }
}

/**
 *
 */
TypedWritable *MaterialParamTexture::
make_from_bam(const FactoryParams &params) {
  MaterialParamTexture *param = new MaterialParamTexture("");
  DatagramIterator scan;
  BamReader *manager;
  parse_params(params, scan, manager);

  param->fillin(scan, manager);
  return param;
}
