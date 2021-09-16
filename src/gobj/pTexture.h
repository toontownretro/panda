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

#include "pandabase.h"
#include "referenceCount.h"
#include "config_putil.h"
#include "dSearchPath.h"
#include "pointerTo.h"
#include "luse.h"
#include "texture.h"
#include "assetBase.h"

class PDXElement;

/**
 * Texture description as read in from a .ptex file.
 */
class EXPCL_PANDA_GOBJ PTexture : public AssetBase {
PUBLISHED:
  enum Flags {
    F_anisotropic_degree = 0x1,
    F_border_color = 0x2,
    F_min_lod = 0x4,
    F_max_lod = 0x8,
    F_lod_bias = 0x10,
  };

  PTexture();

  bool has_alpha_channel(int num_components) const;

  void clear();

  bool resolve_filenames(const DSearchPath &search_path = get_model_path());

  bool load(PDXElement *data,
            const DSearchPath &search_path = get_model_path());

  INLINE void set_image_filename(const Filename &filename);
  INLINE const Filename &get_image_filename() const;
  INLINE void set_image_fullpath(const Filename &fullpath);
  INLINE const Filename &get_image_fullpath() const;

  INLINE void set_alpha_image_filename(const Filename &filename);
  INLINE const Filename &get_alpha_image_filename() const;
  INLINE void set_alpha_image_fullpath(const Filename &fullpath);
  INLINE const Filename &get_alpha_image_fullpath() const;
  INLINE bool has_alpha_image_filename() const;

  INLINE void set_texture_type(Texture::TextureType type);
  INLINE Texture::TextureType get_texture_type() const;

  INLINE void set_format(Texture::Format format);
  INLINE Texture::Format get_format() const;

  INLINE void set_wrap_u(Texture::WrapMode mode);
  INLINE Texture::WrapMode get_wrap_u() const;

  INLINE void set_wrap_v(Texture::WrapMode mode);
  INLINE Texture::WrapMode get_wrap_v() const;

  INLINE void set_wrap_w(Texture::WrapMode mode);
  INLINE Texture::WrapMode get_wrap_w() const;

  INLINE void set_minfilter(Texture::FilterType type);
  INLINE Texture::FilterType get_minfilter() const;

  INLINE void set_magfilter(Texture::FilterType type);
  INLINE Texture::FilterType get_magfilter() const;

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

  INLINE void set_quality(Texture::QualityLevel quality);
  INLINE Texture::QualityLevel get_quality() const;

  INLINE void set_compression(Texture::CompressionMode mode);
  INLINE Texture::CompressionMode get_compression() const;

  INLINE void set_num_pages(int pages);
  INLINE int get_num_pages() const;

  // AssetBase interface.
  virtual std::string get_name() override;
  virtual std::string get_source_extension() override;
  virtual std::string get_built_extension() override;
  virtual bool load(const Filename &filename, const DSearchPath &search_path) override;

public:
  // AssetBase interface.
  virtual void get_dependencies(vector_string &filenames) override;
  virtual PT(AssetBase) make_new() const override;

private:
  Filename _image_filename;
  Filename _image_fullpath;

  Filename _alpha_image_filename;
  Filename _alpha_image_fullpath;

  Texture::TextureType _texture_type;
  Texture::Format _format;
  Texture::WrapMode _wrap_u;
  Texture::WrapMode _wrap_v;
  Texture::WrapMode _wrap_w;
  Texture::FilterType _min_filter;
  Texture::FilterType _mag_filter;
  LColor _border_color;
  PN_stdfloat _min_lod;
  PN_stdfloat _max_lod;
  PN_stdfloat _lod_bias;
  int _anisotropic_degree;
  Texture::QualityLevel _quality;
  Texture::CompressionMode _compression;
  int _num_pages;

  unsigned int _flags;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AssetBase::init_type();
    register_type(_type_handle, "PTexture",
                  AssetBase::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

EXPCL_PANDA_GOBJ std::ostream &operator << (std::ostream &out, const PTexture &ptex);

#include "pTexture.I"

#endif // PTEXTURE_H
