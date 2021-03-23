/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pTexture.cxx
 * @author lachbr
 * @date 2021-01-17
 */

#include "pTexture.h"
#include "virtualFileSystem.h"
#include "string_utils.h"
#include "keyValues.h"

/**
 *
 */
PTexture::
PTexture() {
  clear();
}

/**
 *
 */
PTexture::
PTexture(const PTexture &other) {
  _image_filename = other._image_filename;
  _image_fullpath = other._image_fullpath;

  _alpha_image_filename = other._alpha_image_filename;
  _alpha_image_fullpath = other._alpha_image_fullpath;

  _filename = other._filename;
  _fullpath = other._fullpath;

  _texture_type = other._texture_type;
  _format = other._format;
  _wrap_u = other._wrap_u;
  _wrap_v = other._wrap_v;
  _wrap_w = other._wrap_w;
  _min_filter = other._min_filter;
  _mag_filter = other._mag_filter;
  _border_color = other._border_color;
  _min_lod = other._min_lod;
  _max_lod = other._max_lod;
  _lod_bias = other._lod_bias;
  _anisotropic_degree = other._anisotropic_degree;
  _quality = other._quality;
  _compression = other._compression;
  _num_pages = other._num_pages;

  _flags = other._flags;
}

/**
 * Resets the texture state.
 */
void PTexture::
clear() {
  _texture_type = TT_unspecified;
  _filename = "";
  _fullpath = "";
  _image_filename = "";
  _image_fullpath = "";
  _alpha_image_filename = "";
  _alpha_image_fullpath = "";
  _format = F_unspecified;
  _wrap_u = WM_unspecified;
  _wrap_v = WM_unspecified;
  _wrap_w = WM_unspecified;
  _min_filter = FT_unspecified;
  _mag_filter = FT_unspecified;
  _border_color.set(0, 0, 0, 0);
  _min_lod = 0.0;
  _max_lod = 0.0;
  _lod_bias = 0.0;
  _anisotropic_degree = 0;
  _quality = QL_unspecified;
  _compression = CM_default;
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
  case F_red:
  case F_green:
  case F_blue:
  case F_luminance:
  case F_sluminance:
  case F_rgb:
  case F_rgb12:
  case F_rgb8:
  case F_rgb5:
  case F_rgb332:
  case F_srgb:
    // These formats never use alpha, regardless of the number of components
    // we have.
    return false;

  case F_alpha:
    // This format always uses alpha.
    return true;

  case F_luminance_alpha:
  case F_luminance_alphamask:
  case F_sluminance_alpha:
  case F_rgba:
  case F_rgbm:
  case F_rgba12:
  case F_rgba8:
  case F_rgba4:
  case F_rgba5:
  case F_srgb_alpha:
  case F_unspecified:
    // These formats use alpha if the image had alpha.
    return (num_components == 2 || num_components == 4);
  }

  return false;
}

/**
 *
 */
int PTexture::
compare_to(const PTexture *other) const {

  if (_flags != other->_flags) {
    return (_flags < other->_flags) ? -1 : 1;
  }

  int cmp;

  cmp = _filename.compare_to(other->_filename);
  if (cmp != 0) {
    return cmp;
  }

  cmp = _fullpath.compare_to(other->_fullpath);
  if (cmp != 0) {
    return cmp;
  }

  cmp = _image_filename.compare_to(other->_image_filename);
  if (cmp != 0) {
    return cmp;
  }

  cmp = _image_fullpath.compare_to(other->_image_fullpath);
  if (cmp != 0) {
    return cmp;
  }

  cmp = _alpha_image_filename.compare_to(other->_alpha_image_filename);
  if (cmp != 0) {
    return cmp;
  }

  cmp = _alpha_image_fullpath.compare_to(other->_alpha_image_fullpath);
  if (cmp != 0) {
    return cmp;
  }

  if (_texture_type != other->_texture_type) {
    return (_texture_type < other->_texture_type) ? -1 : 1;
  }

  if (_format != other->_format) {
    return (_format < other->_format) ? -1 : 1;
  }

  if (_wrap_u != other->_wrap_u) {
    return (_wrap_u < other->_wrap_u) ? -1 : 1;
  }

  if (_wrap_v != other->_wrap_v) {
    return (_wrap_v < other->_wrap_v) ? -1 : 1;
  }

  if (_wrap_w != other->_wrap_w) {
    return (_wrap_w < other->_wrap_w) ? -1 : 1;
  }

  if (_min_filter != other->_min_filter) {
    return (_min_filter < other->_min_filter) ? -1 : 1;
  }

  if (_mag_filter != other->_mag_filter) {
    return (_mag_filter < other->_mag_filter) ? -1 : 1;
  }

  cmp = _border_color.compare_to(other->_border_color);
  if (cmp != 0) {
    return cmp;
  }

  if (_min_lod != other->_min_lod) {
    return (_min_lod < other->_min_lod) ? -1 : 1;
  }

  if (_max_lod != other->_max_lod) {
    return (_max_lod < other->_max_lod) ? -1 : 1;
  }

  if (_lod_bias != other->_lod_bias) {
    return (_lod_bias < other->_lod_bias) ? -1 : 1;
  }

  if (_anisotropic_degree != other->_anisotropic_degree) {
    return (_anisotropic_degree < other->_anisotropic_degree) ? -1 : 1;
  }

  if (_quality != other->_quality) {
    return (_quality < other->_quality) ? -1 : 1;
  }

  if (_compression != other->_compression) {
    return (_compression < other->_compression) ? -1 : 1;
  }

  if (_num_pages != other->_num_pages) {
    return (_num_pages < other->_num_pages) ? -1 : 1;
  }

  return 0;
}

/**
 * Resolves the image filenames along the indicated search path.
 * Returns true if all images were successfully resolved, or false if one or
 * more could not be resolved.
 */
bool PTexture::
resolve_filenames(const DSearchPath &search_path) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  bool all_ok = true;

  if (!_image_filename.empty()) {
    _image_fullpath = _image_filename;
    if (!vfs->resolve_filename(_image_fullpath, search_path)) {
      all_ok = false;
    }
  }

  if (!_alpha_image_filename.empty()) {
    _alpha_image_fullpath = _alpha_image_filename;
    if (!vfs->resolve_filename(_alpha_image_fullpath, search_path)) {
      all_ok = false;
    }
  }

  return all_ok;
}

/**
 * Writes the object to the indicated .ptex file.
 */
void PTexture::
write(const Filename &filename) {
  PT(KeyValues) kv = new KeyValues;

  if (!_image_filename.empty()) {
    kv->set_key_value("image", _image_filename.get_fullpath());
  }

  if (!_alpha_image_filename.empty()) {
    kv->set_key_value("alpha_image", _alpha_image_filename.get_fullpath());
  }

  if (get_texture_type() != TT_unspecified) {
    kv->set_key_value("type", texture_type_string(get_texture_type()));
  }

  if (get_format() != F_unspecified) {
    kv->set_key_value("format", format_string(get_format()));
  }

  if (get_wrap_u() != WM_unspecified) {
    kv->set_key_value("wrap_u", wrap_mode_string(get_wrap_u()));
  }

  if (get_wrap_v() != WM_unspecified) {
    kv->set_key_value("wrap_v", wrap_mode_string(get_wrap_v()));
  }

  if (get_wrap_w() != WM_unspecified) {
    kv->set_key_value("wrap_w", wrap_mode_string(get_wrap_w()));
  }

  if (get_minfilter() != FT_unspecified) {
    kv->set_key_value("minfilter", filter_type_string(get_minfilter()));
  }

  if (get_magfilter() != FT_unspecified) {
    kv->set_key_value("magfilter", filter_type_string(get_magfilter()));
  }

  if (has_border_color()) {
    kv->set_key_value("border_color", KeyValues::to_string(_border_color));
  }

  if (has_min_lod()) {
    kv->set_key_value("min_lod", KeyValues::to_string(_min_lod));
  }

  if (has_max_lod()) {
    kv->set_key_value("max_lod", KeyValues::to_string(_max_lod));
  }

  if (has_lod_bias()) {
    kv->set_key_value("lod_bias", KeyValues::to_string(_lod_bias));
  }

  if (get_compression() != CM_default) {
    kv->set_key_value("compression", compression_mode_string(get_compression()));
  }

  if (get_quality() != QL_unspecified) {
    kv->set_key_value("quality", quality_level_string(get_quality()));
  }

  if (has_anisotropic_degree()) {
    kv->set_key_value("anisotropic", KeyValues::to_string(get_anisotropic_degree()));
  }

  if (get_texture_type() == TT_3d_texture) {
    // 3D textures need an explicit page count.
    kv->set_key_value("num_pages", KeyValues::to_string(get_num_pages()));
  }

  kv->write(filename, 2);
}

/**
 * Loads the indicated .ptex file from disk and returns a new PTexture object.
 *
 * Return NULL if the texture could not be loaded or parsed.
 */
PT(PTexture) PTexture::
load(const Filename &filename, const DSearchPath &search_path) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  Filename fullpath = filename;
  if (!vfs->resolve_filename(fullpath, search_path)) {
    util_cat.error()
      << "Could not find ptex file " << filename << " on model-path "
      << search_path << "\n";
    return nullptr;
  }

  std::string data = vfs->read_file(fullpath, true);
  PT(KeyValues) kv = KeyValues::from_string(data);
  if (kv == nullptr) {
    util_cat.error()
      << "Could not parse ptex file " << fullpath << "\n";
    return nullptr;
  }

  PT(PTexture) tex = new PTexture;
  tex->set_filename(filename);
  tex->set_fullpath(fullpath);

  for (size_t i = 0; i < kv->get_num_keys(); i++) {
    const std::string &key = kv->get_key(i);
    const std::string &value = kv->get_value(i);

    if (key == "format") {
      tex->set_format(string_format(value));

    } else if (key == "type") {
      tex->set_texture_type(string_texture_type(value));

    } else if (key == "wrap") {
      WrapMode mode = string_wrap_mode(value);
      tex->set_wrap_u(mode);
      tex->set_wrap_v(mode);
      tex->set_wrap_w(mode);

    } else if (key == "wrap_u") {
      tex->set_wrap_u(string_wrap_mode(value));

    } else if (key == "wrap_v") {
      tex->set_wrap_v(string_wrap_mode(value));

    } else if (key == "wrap_w") {
      tex->set_wrap_w(string_wrap_mode(value));

    } else if (key == "minfilter") {
      tex->set_minfilter(string_filter_type(value));

    } else if (key == "magfilter") {
      tex->set_magfilter(string_filter_type(value));

    } else if (key == "compression") {
      tex->set_compression(string_compression_mode(value));

    } else if (key == "quality") {
      tex->set_quality(string_quality_level(value));

    } else if (key == "anisotropic") {
      tex->set_anisotropic_degree(atoi(value.c_str()));

    } else if (key == "image") {
      tex->set_image_filename(value);

    } else if (key == "alpha_image") {
      tex->set_alpha_image_filename(value);

    } else if (key == "min_lod") {
      tex->set_min_lod(atof(value.c_str()));

    } else if (key == "max_lod") {
      tex->set_max_lod(atof(value.c_str()));

    } else if (key == "lod_bias") {
      tex->set_lod_bias(atof(value.c_str()));

    } else if (key == "border_color") {
      tex->set_border_color(KeyValues::to_4f(value));

    } else if (key == "num_pages") {
      tex->set_num_pages(atoi(value.c_str()));
    }
  }

  // Now resolve the image filenames, using the directory of the ptex file
  // as an additional search path.
  DSearchPath tex_search_path = search_path;
  tex_search_path.append_directory(fullpath.get_dirname());
  tex->resolve_filenames(tex_search_path);

  return tex;
}

/**
 * Returns the Texture_ype value associated with the given string
 * representation, or TT_unspecified if the string does not match any known
 * TextureType value.
 */
PTexture::TextureType PTexture::
string_texture_type(const std::string &string) {
  if (cmp_nocase_uh(string, "1d") == 0 ||
      cmp_nocase_uh(string, "1dtexture") == 0 ||
      cmp_nocase_uh(string, "1d_texture") == 0) {
    return TT_1d_texture;

  } else if (cmp_nocase_uh(string, "2d") == 0 ||
             cmp_nocase_uh(string, "2dtexture") == 0 ||
             cmp_nocase_uh(string, "2d_texture") == 0) {
    return TT_2d_texture;

  } else if (cmp_nocase_uh(string, "3d") == 0 ||
             cmp_nocase_uh(string, "3dtexture") == 0 ||
             cmp_nocase_uh(string, "3d_texture") == 0) {
    return TT_3d_texture;

  } else if (cmp_nocase_uh(string, "cube") == 0 ||
             cmp_nocase_uh(string, "cubemap") == 0 ||
             cmp_nocase_uh(string, "cube_map") == 0) {
    return TT_cube_map;

  } else {
    return TT_unspecified;
  }
}

/**
 * Returns a string representation for the given texture type.
 */
std::string PTexture::
texture_type_string(TextureType type) {
  switch (type) {
  case TT_1d_texture:
    return "1d";
  case TT_2d_texture:
    return "2d";
  case TT_3d_texture:
    return "3d";
  case TT_cube_map:
    return "cube";
  default:
    return "unspecified";
  }
}

/**
 * Returns the Format value associated with the given string representation,
 * or F_unspecified if the string does not match any known Format value.
 */
PTexture::Format PTexture::
string_format(const std::string &string) {
  if (cmp_nocase_uh(string, "rgba") == 0) {
    return F_rgba;
  } else if (cmp_nocase_uh(string, "srgb_alpha") == 0) {
    return F_srgb_alpha;
  } else if (cmp_nocase_uh(string, "rgbm") == 0) {
    return F_rgbm;
  } else if (cmp_nocase_uh(string, "rgba12") == 0) {
    return F_rgba12;
  } else if (cmp_nocase_uh(string, "rgba8") == 0) {
    return F_rgba8;
  } else if (cmp_nocase_uh(string, "rgba4") == 0) {
    return F_rgba4;

  } else if (cmp_nocase_uh(string, "rgb") == 0) {
    return F_rgb;
  } else if (cmp_nocase_uh(string, "srgb") == 0) {
    return F_srgb;
  } else if (cmp_nocase_uh(string, "rgb12") == 0) {
    return F_rgb12;
  } else if (cmp_nocase_uh(string, "rgb8") == 0) {
    return F_rgb8;
  } else if (cmp_nocase_uh(string, "rgb5") == 0) {
    return F_rgb5;
  } else if (cmp_nocase_uh(string, "rgba5") == 0) {
    return F_rgba5;
  } else if (cmp_nocase_uh(string, "rgb332") == 0) {
    return F_rgb332;
  } else if (cmp_nocase_uh(string, "red") == 0) {
    return F_red;
  } else if (cmp_nocase_uh(string, "green") == 0) {
    return F_green;
  } else if (cmp_nocase_uh(string, "blue") == 0) {
    return F_blue;
  } else if (cmp_nocase_uh(string, "alpha") == 0) {
    return F_alpha;
  } else if (cmp_nocase_uh(string, "luminance") == 0) {
    return F_luminance;
  } else if (cmp_nocase_uh(string, "luminance_alpha") == 0) {
    return F_luminance_alpha;
  } else if (cmp_nocase_uh(string, "luminance_alphamask") == 0) {
    return F_luminance_alphamask;
  } else if (cmp_nocase_uh(string, "sluminance") == 0) {
    return F_sluminance;
  } else if (cmp_nocase_uh(string, "sluminance_alpha") == 0) {
    return F_sluminance_alpha;
  } else {
    return F_unspecified;
  }
}

/**
 * Returns a string representation for the given format.
 */
std::string PTexture::
format_string(Format format) {
  switch (format) {
  case F_rgba:
    return "rgba";
  case F_srgb_alpha:
    return "srgb_alpha";
  case F_rgbm:
    return "rgbm";
  case F_rgba12:
    return "rgba12";
  case F_rgba8:
    return "rgba8";
  case F_rgba4:
    return "rgba4";

  case F_rgb:
    return "rgb";
  case F_srgb:
    return "srgb";
  case F_rgb12:
    return "rgb12";
  case F_rgb8:
    return "rgb8";
  case F_rgb5:
    return "rgb5";
  case F_rgba5:
    return "rgba5";
  case F_rgb332:
    return "rgb332";
  case F_red:
    return "red";
  case F_green:
    return "green";
  case F_blue:
    return "blue";
  case F_alpha:
    return "alpha";
  case F_luminance:
    return "luminance";
  case F_luminance_alpha:
    return "luminance_alpha";
  case F_luminance_alphamask:
    return "luminance_alphamask";
  case F_sluminance:
    return "sluminance";
  case F_sluminance_alpha:
    return "sluminance_alpha";

  default:
    return "unspecified";
  }
}

/**
 * Returns the CompressionMode value associated with the given string
 * representation, or CM_default if the string does not match any known
 * CompressionMode value.
 */
PTexture::CompressionMode PTexture::
string_compression_mode(const std::string &string) {
  if (cmp_nocase_uh(string, "off") == 0) {
    return CM_off;
  } else if (cmp_nocase_uh(string, "on") == 0) {
    return CM_on;
  } else if (cmp_nocase_uh(string, "fxt1") == 0) {
    return CM_fxt1;
  } else if (cmp_nocase_uh(string, "dxt1") == 0) {
    return CM_dxt1;
  } else if (cmp_nocase_uh(string, "dxt2") == 0) {
    return CM_dxt2;
  } else if (cmp_nocase_uh(string, "dxt3") == 0) {
    return CM_dxt3;
  } else if (cmp_nocase_uh(string, "dxt4") == 0) {
    return CM_dxt4;
  } else if (cmp_nocase_uh(string, "dxt5") == 0) {
    return CM_dxt5;
  } else {
    return CM_default;
  }
}

/**
 * Returns a string representation for the given compression mode.
 */
std::string PTexture::
compression_mode_string(CompressionMode mode) {
  switch (mode) {
  case CM_off:
    return "off";
  case CM_on:
    return "on";
  case CM_fxt1:
    return "fxt1";
  case CM_dxt1:
    return "dxt1";
  case CM_dxt2:
    return "dxt2";
  case CM_dxt3:
    return "dxt3";
  case CM_dxt4:
    return "dxt4";
  case CM_dxt5:
    return "dxt5";
  default:
    return "unspecified";
  }
}

/**
 * Returns the WrapMode value associated with the given string representation,
 * or WM_unspecified if the string does not match any known WrapMode value.
 */
PTexture::WrapMode PTexture::
string_wrap_mode(const std::string &string) {
  if (cmp_nocase_uh(string, "repeat") == 0) {
    return WM_repeat;
  } else if (cmp_nocase_uh(string, "clamp") == 0) {
    return WM_clamp;
  } else if (cmp_nocase_uh(string, "mirror") == 0) {
    return WM_mirror;
  } else if (cmp_nocase_uh(string, "mirror_once") == 0) {
    return WM_mirror_once;
  } else if (cmp_nocase_uh(string, "border_color") == 0) {
    return WM_border_color;
  } else {
    return WM_unspecified;
  }
}

/**
 * Returns a string representation for the given wrap mode.
 */
std::string PTexture::
wrap_mode_string(WrapMode mode) {
  switch (mode) {
  case WM_repeat:
    return "repeat";
  case WM_clamp:
    return "clamp";
  case WM_mirror:
    return "mirror";
  case WM_mirror_once:
    return "mirror_once";
  case WM_border_color:
    return "border_color";
  default:
    return "unspecified";
  }
}

/**
 * Returns the FilterType value associated with the given string
 * representation, or FT_unspecified if the string does not match any known
 * FilterType value.
 */
PTexture::FilterType PTexture::
string_filter_type(const std::string &string) {
  // Old egg filter types.
  if (cmp_nocase_uh(string, "point") == 0) {
    return FT_nearest;
  } else if (cmp_nocase_uh(string, "linear") == 0) {
    return FT_linear;
  } else if (cmp_nocase_uh(string, "bilinear") == 0) {
    return FT_linear;
  } else if (cmp_nocase_uh(string, "trilinear") == 0) {
    return FT_linear_mipmap_linear;
  } else if (cmp_nocase_uh(string, "mipmap") == 0) {
    return FT_linear_mipmap_linear;
  } else if (cmp_nocase_uh(string, "mipmap_point") == 0) {
    return FT_nearest_mipmap_nearest;
  } else if (cmp_nocase_uh(string, "mipmap_linear") == 0) {
    return FT_nearest_mipmap_linear;
  } else if (cmp_nocase_uh(string, "mipmap_bilinear") == 0) {
    return FT_linear_mipmap_nearest;
  } else if (cmp_nocase_uh(string, "mipmap_trilinear") == 0) {
    return FT_linear_mipmap_linear;

    // Current egg filter types, that match those in Texture.
  } else if (cmp_nocase_uh(string, "nearest") == 0) {
    return FT_nearest;
  } else if (cmp_nocase_uh(string, "linear") == 0) {
    return FT_linear;
  } else if (cmp_nocase_uh(string, "nearest_mipmap_nearest") == 0) {
    return FT_nearest_mipmap_nearest;
  } else if (cmp_nocase_uh(string, "linear_mipmap_nearest") == 0) {
    return FT_linear_mipmap_nearest;
  } else if (cmp_nocase_uh(string, "nearest_mipmap_linear") == 0) {
    return FT_nearest_mipmap_linear;
  } else if (cmp_nocase_uh(string, "linear_mipmap_linear") == 0) {
    return FT_linear_mipmap_linear;

  } else {
    return FT_unspecified;
  }
}

/**
 * Returns a string representation for the given filter type.
 */
std::string PTexture::
filter_type_string(FilterType type) {
  switch (type) {
  case FT_nearest:
    return "nearest";
  case FT_linear:
    return "linear";
  case FT_nearest_mipmap_nearest:
    return "nearest_mipmap_nearest";
  case FT_linear_mipmap_nearest:
    return "linear_mipmap_nearest";
  case FT_nearest_mipmap_linear:
    return "nearest_mipmap_linear";
  case FT_linear_mipmap_linear:
    return "linear_mipmap_linear";
  default:
    return "unspecified";
  }
}

/**
 * Returns the TexGen value associated with the given string representation,
 * or ET_unspecified if the string does not match any known TexGen value.
 */
PTexture::QualityLevel PTexture::
string_quality_level(const std::string &string) {
  if (cmp_nocase_uh(string, "unspecified") == 0) {
    return QL_unspecified;

  } else if (cmp_nocase_uh(string, "default") == 0) {
    return QL_default;

  } else if (cmp_nocase_uh(string, "fastest") == 0) {
    return QL_fastest;

  } else if (cmp_nocase_uh(string, "normal") == 0) {
    return QL_normal;

  } else if (cmp_nocase_uh(string, "best") == 0) {
    return QL_best;

  } else {
    return QL_unspecified;
  }
}

/**
 * Returns a string representation for the given quality level.
 */
std::string PTexture::
quality_level_string(QualityLevel level) {
  switch (level) {
  case QL_default:
    return "default";
  case QL_fastest:
    return "fastest";
  case QL_normal:
    return "normal";
  case QL_best:
    return "best";
  default:
    return "unspecified";
  }
}
