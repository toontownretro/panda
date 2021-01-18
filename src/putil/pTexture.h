/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pTexture.h
 * @author lachbr
 * @date 2021-01-17
 */

#ifndef PTEXTURE_H
#define PTEXTURE_H

#include "referenceCount.h"
#include "config_putil.h"
#include "dSearchPath.h"
#include "pointerTo.h"
#include "luse.h"

/**
 * Panda Texture (ptex) file data.  This is a source texture file that contains
 * properties such as the image filename, filtering properties, etc.
 */
class EXPCL_PANDA_PUTIL PTexture : public ReferenceCount {
PUBLISHED:
  enum TextureType {
    TT_unspecified,
    TT_1d_texture,
    TT_2d_texture,
    TT_3d_texture,
    TT_cube_map,
  };

  enum Format {
    F_unspecified,
    F_rgba, F_rgbm, F_rgba12, F_rgba8, F_rgba4, F_rgba5,
    F_rgb, F_rgb12, F_rgb8, F_rgb5, F_rgb332,
    F_red, F_green, F_blue, F_alpha, F_luminance,
    F_luminance_alpha, F_luminance_alphamask,
    F_srgb, F_srgb_alpha,
    F_sluminance, F_sluminance_alpha
  };

  enum WrapMode {
    WM_unspecified, WM_clamp, WM_repeat,
    WM_mirror, WM_mirror_once, WM_border_color
  };

  enum FilterType {
    // Note that these type values match up, name-for-name, with a similar
    // enumerated type in Panda's Texture object.  However, they do *not*
    // match up numerically.  You must convert between them using a switch
    // statement.
    FT_unspecified,

    // Mag Filter and Min Filter
    FT_nearest,
    FT_linear,

    // Min Filter Only
    FT_nearest_mipmap_nearest,   // "mipmap point"
    FT_linear_mipmap_nearest,    // "mipmap linear"
    FT_nearest_mipmap_linear,    // "mipmap bilinear"
    FT_linear_mipmap_linear,     // "mipmap trilinear"
  };

  enum QualityLevel {
    QL_unspecified,
    QL_default,
    QL_fastest,
    QL_normal,
    QL_best,
  };

  enum CompressionMode {
    CM_default, CM_off, CM_on,
    CM_fxt1, CM_dxt1, CM_dxt2, CM_dxt3, CM_dxt4, CM_dxt5,
  };

  enum Flags {
    F_anisotropic_degree = 0x1,
    F_border_color = 0x2,
    F_min_lod = 0x4,
    F_max_lod = 0x8,
    F_lod_bias = 0x10,
  };

  PTexture();
  PTexture(const PTexture &copy);

  void clear();

  int compare_to(const PTexture *other) const;

  bool resolve_filenames(const DSearchPath &search_path = get_model_path());

  void write(const Filename &filename);

  static PT(PTexture) load(const Filename &filename,
                           const DSearchPath &search_path = get_model_path());

  INLINE std::string get_name() const;

  INLINE void set_filename(const Filename &filename);
  INLINE const Filename &get_filename() const;
  INLINE void set_fullpath(const Filename &fullpath);
  INLINE const Filename &get_fullpath() const;

  INLINE void set_image_filename(const Filename &filename);
  INLINE const Filename &get_image_filename() const;
  INLINE void set_image_fullpath(const Filename &fullpath);
  INLINE const Filename &get_image_fullpath() const;

  INLINE void set_alpha_image_filename(const Filename &filename);
  INLINE const Filename &get_alpha_image_filename() const;
  INLINE void set_alpha_image_fullpath(const Filename &fullpath);
  INLINE const Filename &get_alpha_image_fullpath() const;
  INLINE bool has_alpha_image_filename() const;

  INLINE void set_texture_type(TextureType type);
  INLINE TextureType get_texture_type() const;

  INLINE void set_format(Format format);
  INLINE Format get_format() const;

  INLINE void set_wrap_u(WrapMode mode);
  INLINE WrapMode get_wrap_u() const;

  INLINE void set_wrap_v(WrapMode mode);
  INLINE WrapMode get_wrap_v() const;

  INLINE void set_wrap_w(WrapMode mode);
  INLINE WrapMode get_wrap_w() const;

  INLINE void set_minfilter(FilterType type);
  INLINE FilterType get_minfilter() const;

  INLINE void set_magfilter(FilterType type);
  INLINE FilterType get_magfilter() const;

  INLINE void set_border_color(const LColor &color);
  INLINE const LColor &get_border_color() const;
  INLINE bool has_border_color() const;
  INLINE void clear_border_color();

  INLINE void set_min_lod(PN_stdfloat lod);
  INLINE PN_stdfloat get_min_lod() const;
  INLINE bool has_min_lod() const;
  INLINE void clear_min_lod();

  INLINE void set_max_lod(PN_stdfloat lod);
  INLINE PN_stdfloat get_max_lod() const;
  INLINE bool has_max_lod() const;
  INLINE void clear_max_lod();

  INLINE void set_lod_bias(PN_stdfloat bias);
  INLINE PN_stdfloat get_lod_bias() const;
  INLINE bool has_lod_bias() const;
  INLINE void clear_lod_bias();

  INLINE void set_anisotropic_degree(int degree);
  INLINE int get_anisotropic_degree() const;
  INLINE bool has_anisotropic_degree() const;
  INLINE void clear_anisotropic_degree();

  INLINE void set_quality(QualityLevel quality);
  INLINE QualityLevel get_quality() const;

  INLINE void set_compression(CompressionMode mode);
  INLINE CompressionMode get_compression() const;

  static TextureType string_texture_type(const std::string &string);
  static std::string texture_type_string(TextureType type);

  static Format string_format(const std::string &string);
  static std::string format_string(Format format);

  static CompressionMode string_compression_mode(const std::string &string);
  static std::string compression_mode_string(CompressionMode mode);

  static WrapMode string_wrap_mode(const std::string &string);
  static std::string wrap_mode_string(WrapMode mode);

  static FilterType string_filter_type(const std::string &string);
  static std::string filter_type_string(FilterType type);

  static QualityLevel string_quality_level(const std::string &string);
  static std::string quality_level_string(QualityLevel level);

private:
  Filename _image_filename;
  Filename _image_fullpath;

  Filename _alpha_image_filename;
  Filename _alpha_image_fullpath;

  Filename _filename;
  Filename _fullpath;

  TextureType _texture_type;
  Format _format;
  WrapMode _wrap_u;
  WrapMode _wrap_v;
  WrapMode _wrap_w;
  FilterType _min_filter;
  FilterType _mag_filter;
  LColor _border_color;
  PN_stdfloat _min_lod;
  PN_stdfloat _max_lod;
  PN_stdfloat _lod_bias;
  int _anisotropic_degree;
  QualityLevel _quality;
  CompressionMode _compression;

  unsigned int _flags;
};

#include "pTexture.I"

#endif // PTEXTURE_H
