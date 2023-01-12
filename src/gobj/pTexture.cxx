/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pTexture.cxx
 * @author brian
 * @date 2021-01-17
 */

#include "pTexture.h"
#include "virtualFileSystem.h"
#include "string_utils.h"
#include "pdxElement.h"
#include "pdxValue.h"
#include "executionEnvironment.h"

TypeHandle PTexture::_type_handle;

/**
 *
 */
PTexture::
PTexture() {
  clear();
}

/**
 * Resets the texture state.
 */
void PTexture::
clear() {
  _texture_type = Texture::TT_2d_texture;
  _image_filename = "";
  _image_fullpath = "";
  _alpha_image_filename = "";
  _alpha_image_fullpath = "";
  _format = Texture::F_rgb;
  _wrap_u = SamplerState::WM_repeat;
  _wrap_v = SamplerState::WM_repeat;
  _wrap_w = SamplerState::WM_repeat;
  _min_filter = SamplerState::FT_default;
  _mag_filter = SamplerState::FT_default;
  _border_color.set(0, 0, 0, 0);
  _min_lod = 0.0;
  _max_lod = 0.0;
  _lod_bias = 0.0;
  _anisotropic_degree = 0;
  _quality = Texture::QL_default;
  _compression = Texture::CM_default;
  _num_pages = 1;
  _flags = 0;
}

/**
 * Given the number of color components (channels) in the image file as
 * actually read from the disk, return true if this texture seems to have an
 * alpha channel or not.  This depends on the EggTexture's format as well as
 * the number of channels.
 */
bool PTexture::
has_alpha_channel(int num_components) const {
  switch (_format) {
  case Texture::F_red:
  case Texture::F_green:
  case Texture::F_blue:
  case Texture::F_luminance:
  case Texture::F_sluminance:
  case Texture::F_rgb:
  case Texture::F_rgb12:
  case Texture::F_rgb8:
  case Texture::F_rgb5:
  case Texture::F_rgb332:
  case Texture::F_srgb:
    // These formats never use alpha, regardless of the number of components
    // we have.
    return false;

  case Texture::F_alpha:
    // This format always uses alpha.
    return true;

  case Texture::F_luminance_alpha:
  case Texture::F_luminance_alphamask:
  case Texture::F_sluminance_alpha:
  case Texture::F_rgba:
  case Texture::F_rgbm:
  case Texture::F_rgba12:
  case Texture::F_rgba8:
  case Texture::F_rgba4:
  case Texture::F_rgba5:
  case Texture::F_srgb_alpha:
    // These formats use alpha if the image had alpha.
    return (num_components == 2 || num_components == 4);
  }

  return false;
}

/**
 * Resolves the image filenames along the indicated search path.
 * Returns true if all images were successfully resolved, or false if one or
 * more could not be resolved.
 */
bool PTexture::
resolve_filenames(const DSearchPath &search_path) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  DSearchPath path = search_path;
  path.append_directory(ExecutionEnvironment::get_cwd());

  bool all_ok = true;

  if (!_image_filename.empty()) {
    _image_fullpath = _image_filename;
    if (!vfs->resolve_filename(_image_fullpath, path)) {
      all_ok = false;
    }
  }

  if (!_alpha_image_filename.empty()) {
    _alpha_image_fullpath = _alpha_image_filename;
    if (!vfs->resolve_filename(_alpha_image_fullpath, path)) {
      all_ok = false;
    }
  }

  return all_ok;
}

/**
 * Loads the indicated .ptex file from disk and returns a new PTexture object.
 *
 * Return NULL if the texture could not be loaded or parsed.
 */
bool PTexture::
load(PDXElement *data, const DSearchPath &search_path) {

  for (size_t i = 0; i < data->get_num_attributes(); i++) {
    const std::string &key = data->get_attribute_name(i);
    const PDXValue &val = data->get_attribute_value(i);

    if (key == "format") {
      set_format(Texture::string_format(val.get_string()));

    } else if (key == "type") {
      set_texture_type(Texture::string_texture_type(val.get_string()));

    } else if (key == "wrap") {
      SamplerState::WrapMode mode = SamplerState::string_wrap_mode(val.get_string());
      set_wrap_u(mode);
      set_wrap_v(mode);
      set_wrap_w(mode);

    } else if (key == "wrap_u") {
      set_wrap_u(SamplerState::string_wrap_mode(val.get_string()));

    } else if (key == "wrap_v") {
      set_wrap_v(SamplerState::string_wrap_mode(val.get_string()));

    } else if (key == "wrap_w") {
      set_wrap_w(SamplerState::string_wrap_mode(val.get_string()));

    } else if (key == "minfilter") {
      set_minfilter(SamplerState::string_filter_type(val.get_string()));

    } else if (key == "magfilter") {
      set_magfilter(SamplerState::string_filter_type(val.get_string()));

    } else if (key == "compression") {
      set_compression(Texture::string_compression_mode(val.get_string()));

    } else if (key == "quality") {
      set_quality(Texture::string_quality_level(val.get_string()));

    } else if (key == "anisotropic") {
      set_anisotropic_degree(val.get_int());

    } else if (key == "image") {
      set_image_filename(val.get_string());

    } else if (key == "alpha_image") {
      set_alpha_image_filename(val.get_string());

    } else if (key == "min_lod") {
      set_min_lod(val.get_float());

    } else if (key == "max_lod") {
      set_max_lod(val.get_float());

    } else if (key == "lod_bias") {
      set_lod_bias(val.get_float());

    } else if (key == "border_color") {
      LVecBase4 border_color(255);
      if (!val.to_vec4(border_color)) {
        gobj_cat.error()
          << "Invalid border color specified\n";
        return false;
      }
      border_color /= 255;
      set_border_color(border_color);

    } else if (key == "num_pages") {
      set_num_pages(val.get_int());
    }
  }

  if (!resolve_filenames(search_path)) {
    return false;
  }

  return true;
}

/**
 *
 */
std::ostream &
operator << (std::ostream &out, const PTexture &ptex) {
  out << "format " << ptex.get_format() << "\n";
  out << "type " << ptex.get_texture_type() << "\n";
  out << "image " << ptex.get_image_fullpath() << "\n";
  out << "alpha " << ptex.get_alpha_image_fullpath() << "\n";
  out << "wrap u " << ptex.get_wrap_u() << "\n";
  out << "wrap v " << ptex.get_wrap_v() << "\n";
  out << "wrap w " << ptex.get_wrap_w() << "\n";
  out << "minfilter " << ptex.get_minfilter() << "\n";
  out << "magfilter " << ptex.get_magfilter() << "\n";
  out << "aniso " << ptex.get_anisotropic_degree() << "\n";
  out << "compression " << ptex.get_compression() << "\n";
  out << "quality " << ptex.get_quality() << "\n";
  out << "border color " << ptex.get_border_color() << "\n";
  out << "min lod " << ptex.get_min_lod() << "\n";
  out << "max lod " << ptex.get_max_lod() << "\n";
  out << "lod bias " << ptex.get_lod_bias() << "\n";
  out << "num pages " << ptex.get_num_pages() << "\n";
  return out;
}

/**
 *
 */
void PTexture::
get_dependencies(vector_string &filenames) {
  // We depend on the image filename(s).  If they change, the ptex
  // needs to be rebuilt.

  Texture::TextureType type = get_texture_type();
  if (type == Texture::TT_1d_texture ||
      type == Texture::TT_2d_texture) {
    // Single-faced texture.  We depend on the image file and the alpha file.
    if (!_image_fullpath.empty()) {
      filenames.push_back(_image_fullpath.get_fullpath());
    }
    if (!_alpha_image_fullpath.empty()) {
      filenames.push_back(_alpha_image_fullpath.get_fullpath());
    }

  } else {
    // A texture with multiple slices.  We depend on all the slice files.
    Filename pattern = _image_fullpath;
    pattern.set_pattern(true);

    Filename alpha_pattern = _alpha_image_fullpath;
    alpha_pattern.set_pattern(true);

    for (int i = 0; i < _num_pages; i++) {
      Filename page_filename = pattern.get_filename_index(i);
      if (!page_filename.empty()) {
        filenames.push_back(page_filename);
      }
      page_filename = alpha_pattern.get_filename_index(i);
      if (!page_filename.empty()) {
        filenames.push_back(page_filename);
      }
    }
  }
}

/**
 *
 */
std::string PTexture::
get_name() {
  return "texture";
}

/**
 *
 */
std::string PTexture::
get_source_extension() {
  return "ptex";
}

/**
 *
 */
std::string PTexture::
get_built_extension() {
  return "txo";
}

/**
 *
 */
bool PTexture::
load(const Filename &filename, const DSearchPath &search_path) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  Filename fullpath = filename;
  if (!vfs->resolve_filename(fullpath, search_path)) {
    gobj_cat.error()
      << "Failed to find ptex file " << filename << " on search path "
      << search_path << "\n";
    return false;
  }

  DSearchPath path = search_path;
  path.append_directory(fullpath.get_dirname());

  PDXValue data;
  if (!data.read(fullpath, path)) {
    return false;
  }

  PDXElement *elem = data.get_element();
  if (elem == nullptr) {
    return false;
  }

  return load(elem, path);
}

PT(AssetBase) PTexture::
make_new() const {
  return new PTexture;
}
