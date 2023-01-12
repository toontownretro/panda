/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialParamTexture.cxx
 * @author brian
 * @date 2021-03-07
 */

#include "materialParamTexture.h"
#include "texturePool.h"
#include "virtualFileSystem.h"
#include "pdxElement.h"
#include "pdxList.h"

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

    if (element->has_attribute("anims")) {
      PDXList *anims_list = element->get_attribute_value("anims").get_list();
      int page = 0;
      for (size_t i = 0; i < anims_list->size(); ++i) {
        PDXElement *anim_data = anims_list->get(i).get_element();
        AnimData adata;
        adata._interp = false;
        adata._loop = false;
        adata._fps = 24;
        if (anim_data->has_attribute("first_page")) {
          page = anim_data->get_attribute_value("first_page").get_int();
        }
        adata._first_frame = page;
        adata._num_frames = anim_data->get_attribute_value("num_pages").get_int();
        if (anim_data->has_attribute("fps")) {
          adata._fps = anim_data->get_attribute_value("fps").get_int();
        }
        if (anim_data->has_attribute("interpolate")) {
          adata._interp = anim_data->get_attribute_value("interpolate").get_bool();
        }
        if (anim_data->has_attribute("loop")) {
          adata._loop = anim_data->get_attribute_value("loop").get_bool();
        }
        _anim_datas.push_back(std::move(adata));
        page += adata._num_frames;
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

  if (!validate_animations()) {
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
bool MaterialParamTexture::
validate_animations() const {
  if (_anim_datas.empty()) {
    return true;
  }

  if (_value == nullptr) {
    return false;
  }

  // We can't have animations if the texture is not a 2-D array.
  if (_value->get_texture_type() != Texture::TT_2d_texture_array) {
    return false;
  }

  // Validate animation ranges against texture page count.
  for (const AnimData &adata : _anim_datas) {
    if ((adata._first_frame < 0) || (adata._first_frame >= _value->get_num_pages())) {
      return false;
    }
    if ((adata._num_frames < 0) || ((adata._first_frame + adata._num_frames) > _value->get_num_pages())) {
      return false;
    }
  }

  return true;
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
  me.add_uint8(_anim_datas.size());
  for (const AnimData &adata : _anim_datas) {
    me.add_uint16(adata._first_frame);
    me.add_uint16(adata._num_frames);
    me.add_uint8(adata._fps);
    me.add_bool(adata._loop);
    me.add_bool(adata._interp);
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
  _anim_datas.resize(scan.get_uint8());
  for (size_t i = 0; i < _anim_datas.size(); ++i) {
    AnimData &adata = _anim_datas[i];
    adata._first_frame = scan.get_uint16();
    adata._num_frames = scan.get_uint16();
    adata._fps = scan.get_uint8();
    adata._loop = scan.get_bool();
    adata._interp = scan.get_bool();
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
