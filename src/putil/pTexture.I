/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pTexture.I
 * @author lachbr
 * @date 2021-01-17
 */

/**
 * Returns the name of the texture.  This is the basename of the filename the
 * texture originated from.
 */
INLINE std::string PTexture::
get_name() const {
  return _filename.get_basename_wo_extension();
}

/**
 * Sets the filename of the texture.
 */
INLINE void PTexture::
set_filename(const Filename &filename) {
  _filename = filename;
}

/**
 * Returns the filename of the texture.
 */
INLINE const Filename &PTexture::
get_filename() const {
  return _filename;
}

/**
 * Sets the fullpath of the texture.
 */
INLINE void PTexture::
set_fullpath(const Filename &fullpath) {
  _fullpath = fullpath;
}

/**
 * Returns the fullpath of the texture.
 */
INLINE const Filename &PTexture::
get_fullpath() const {
  return _fullpath;
}

/**
 * Sets the filename of the texture image.
 */
INLINE void PTexture::
set_image_filename(const Filename &filename) {
  _image_filename = filename;
  if (_texture_type == TT_3d_texture || _texture_type == TT_cube_map) {
    _image_filename.set_pattern(true);
  } else {
    _image_filename.set_pattern(false);
  }
}

/**
 * Returns the filename of the texture image.
 */
INLINE const Filename &PTexture::
get_image_filename() const {
  return _image_filename;
}

/**
 * Sets the fullpath of the texture image.
 */
INLINE void PTexture::
set_image_fullpath(const Filename &fullpath) {
  _image_fullpath = fullpath;
  if (_texture_type == TT_3d_texture || _texture_type == TT_cube_map) {
    _image_fullpath.set_pattern(true);
  } else {
    _image_fullpath.set_pattern(false);
  }
}

/**
 * Returns the fullpath of the texture image.
 */
INLINE const Filename &PTexture::
get_image_fullpath() const {
  return _image_fullpath;
}

/**
 * Sets the filename of the texture alpha image.
 */
INLINE void PTexture::
set_alpha_image_filename(const Filename &filename) {
  _alpha_image_filename = filename;
}

/**
 * Returns the filename of the texture alpha image.
 */
INLINE const Filename &PTexture::
get_alpha_image_filename() const {
  return _alpha_image_filename;
}

/**
 * Sets the fullpath of the texture alpha image.
 */
INLINE void PTexture::
set_alpha_image_fullpath(const Filename &fullpath) {
  _alpha_image_fullpath = fullpath;
}

/**
 * Returns the fullpath of the texture alpha image.
 */
INLINE const Filename &PTexture::
get_alpha_image_fullpath() const {
  return _alpha_image_fullpath;
}

/**
 * Returns true if the texture has a non-empty alpha image filename, false
 * otherwise.
 */
INLINE bool PTexture::
has_alpha_image_filename() const {
  return !_alpha_image_filename.empty();
}

/**
 * Sets the texture type.
 */
INLINE void PTexture::
set_texture_type(TextureType type) {
  _texture_type = type;
  if (_texture_type == TT_cube_map) {
    set_num_pages(6);
  }

  if (_texture_type == TT_cube_map || _texture_type == TT_3d_texture) {
    _image_filename.set_pattern(true);
    _image_fullpath.set_pattern(true);
  } else {
    _image_filename.set_pattern(false);
    _image_fullpath.set_pattern(false);
  }
}

/**
 * Returns the texture type.
 */
INLINE PTexture::TextureType PTexture::
get_texture_type() const {
  return _texture_type;
}

/**
 * Sets the image format of the texture.
 */
INLINE void PTexture::
set_format(Format format) {
  _format = format;
}

/**
 * Returns the image format of the texture.
 */
INLINE PTexture::Format PTexture::
get_format() const {
  return _format;
}

/**
 * Sets the U-axis wrapping mode of the texture.
 */
INLINE void PTexture::
set_wrap_u(WrapMode mode) {
  _wrap_u = mode;
}

/**
 * Returns the U-axis wrapping mode of the texture.
 */
INLINE PTexture::WrapMode PTexture::
get_wrap_u() const {
  return _wrap_u;
}

/**
 * Sets the V-axis wrapping mode of the texture.
 */
INLINE void PTexture::
set_wrap_v(WrapMode mode) {
  _wrap_v = mode;
}

/**
 * Returns the V-axis wrapping mode of the texture.
 */
INLINE PTexture::WrapMode PTexture::
get_wrap_v() const {
  return _wrap_v;
}

/**
 * Sets the W-axis wrapping mode of the texture.
 */
INLINE void PTexture::
set_wrap_w(WrapMode mode) {
  _wrap_w = mode;
}

/**
 * Returns the W-axis wrapping mode of the texture.
 */
INLINE PTexture::WrapMode PTexture::
get_wrap_w() const {
  return _wrap_w;
}

/**
 * Sets the minfilter of the texture.
 */
INLINE void PTexture::
set_minfilter(FilterType type) {
  _min_filter = type;
}

/**
 * Returns the minfilter of the texture.
 */
INLINE PTexture::FilterType PTexture::
get_minfilter() const {
  return _min_filter;
}

/**
 * Sets the magfilter of the texture.
 */
INLINE void PTexture::
set_magfilter(FilterType type) {
  _mag_filter = type;
}

/**
 * Returns the magfilter of the texture.
 */
INLINE PTexture::FilterType PTexture::
get_magfilter() const {
  return _mag_filter;
}

/**
 * Sets the border color when using the WM_border_color wrapping mode.
 */
INLINE void PTexture::
set_border_color(const LColor &color) {
  _border_color = color;
  _flags |= F_border_color;
}

/**
 * Returns the border color.
 */
INLINE const LColor &PTexture::
get_border_color() const {
  return _border_color;
}

/**
 * Returns true if a border color has been specified, false otherwise.
 */
INLINE bool PTexture::
has_border_color() const {
  return (_flags & F_border_color) != 0;
}

/**
 * Clears the border color.
 */
INLINE void PTexture::
clear_border_color() {
  _border_color.set(0, 0, 0, 0);
  _flags &= ~F_border_color;
}

/**
 * Sets the minimum LOD value.
 */
INLINE void PTexture::
set_min_lod(PN_stdfloat lod) {
  _min_lod = lod;
  _flags |= F_min_lod;
}

/**
 * Returns the minimum LOD value.
 */
INLINE PN_stdfloat PTexture::
get_min_lod() const {
  return _min_lod;
}

/**
 * Returns true if a minimum LOD value has been specified, false otherwise.
 */
INLINE bool PTexture::
has_min_lod() const {
  return (_flags & F_min_lod) != 0;
}

/**
 * Clears the minimum LOD value.
 */
INLINE void PTexture::
clear_min_lod() {
  _min_lod = 0.0;
  _flags &= ~F_min_lod;
}

/**
 * Sets the maximum LOD value.
 */
INLINE void PTexture::
set_max_lod(PN_stdfloat lod) {
  _max_lod = lod;
  _flags |= F_max_lod;
}

/**
 * Returns the maximum LOD value.
 */
INLINE PN_stdfloat PTexture::
get_max_lod() const {
  return _max_lod;
}

/**
 * Returns true if a maximum LOD value has been specified, false otherwise.
 */
INLINE bool PTexture::
has_max_lod() const {
  return (_flags & F_max_lod) != 0;
}

/**
 * Clears the maximum LOD value.
 */
INLINE void PTexture::
clear_max_lod() {
  _max_lod = 0.0;
  _flags &= ~F_max_lod;
}

/**
 * Sets the LOD bias value.
 */
INLINE void PTexture::
set_lod_bias(PN_stdfloat lod) {
  _lod_bias = lod;
  _flags |= F_lod_bias;
}

/**
 * Returns the LOD bias value.
 */
INLINE PN_stdfloat PTexture::
get_lod_bias() const {
  return _lod_bias;
}

/**
 * Returns true if a LOD bias value has been specified, false otherwise.
 */
INLINE bool PTexture::
has_lod_bias() const {
  return (_flags & F_lod_bias) != 0;
}

/**
 * Clears the LOD bias value.
 */
INLINE void PTexture::
clear_lod_bias() {
  _lod_bias = 0.0;
  _flags &= ~F_lod_bias;
}

/**
 * Sets the anisotropic degree of the texture.
 */
INLINE void PTexture::
set_anisotropic_degree(int degree) {
  _anisotropic_degree = degree;
  _flags |= F_anisotropic_degree;
}

/**
 * Returns the anisotropic degree of the texture.
 */
INLINE int PTexture::
get_anisotropic_degree() const {
  return _anisotropic_degree;
}

/**
 * Returns true if an anisotropic degree has been specified, false otherwise.
 */
INLINE bool PTexture::
has_anisotropic_degree() const {
  return (_flags & F_anisotropic_degree) != 0;
}

/**
 * Clears the anisotropic degree.
 */
INLINE void PTexture::
clear_anisotropic_degree() {
  _anisotropic_degree = 0;
  _flags &= ~F_anisotropic_degree;
}

/**
 * Sets the quality level of the texture.
 */
INLINE void PTexture::
set_quality(QualityLevel quality) {
  _quality = quality;
}

/**
 * Returns the quality level of the texture.
 */
INLINE PTexture::QualityLevel PTexture::
get_quality() const {
  return _quality;
}

/**
 * Sets the compression mode of the texture.
 */
INLINE void PTexture::
set_compression(CompressionMode mode) {
  _compression = mode;
}

/**
 * Returns the compression mode of the texture.
 */
INLINE PTexture::CompressionMode PTexture::
get_compression() const {
  return _compression;
}

/**
 * Sets the number of pages/z-slices in the texture.  It is only necessary to
 * set an explicit page count for 3D textures, cubemaps always have 6, and 2D
 * and 1D textures always have 1.
 */
INLINE void PTexture::
set_num_pages(int pages) {
  _num_pages = pages;
}

/**
 * Returns the number of pages/z-slices in the texture.
 */
INLINE int PTexture::
get_num_pages() const {
  return _num_pages;
}
