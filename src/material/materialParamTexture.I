/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialParamTexture.I
 * @author lachbr
 * @date 2021-03-07
 */

/**
 *
 */
INLINE MaterialParamTexture::
MaterialParamTexture(const std::string &name, Texture *default_value) :
  MaterialParamBase(name),
  _value(default_value),
  _view(0),
  _has_sampler(false)
{
}

/**
 *
 */
INLINE void MaterialParamTexture::
set_value(Texture *tex, int view) {
  _value = tex;
  _view = view;
}

/**
 *
 */
INLINE Texture *MaterialParamTexture::
get_value() const {
  return _value;
}

/**
 *
 */
INLINE void MaterialParamTexture::
set_view(int view) {
  _view = view;
}

/**
 *
 */
INLINE int MaterialParamTexture::
get_view() const {
  return _view;
}

/**
 *
 */
INLINE void MaterialParamTexture::
set_sampler_state(const SamplerState &sampler) {
  _sampler = sampler;
  _has_sampler = true;
}

/**
 *
 */
INLINE const SamplerState &MaterialParamTexture::
get_sampler_state() const {
  if (_has_sampler) {
    return _sampler;

  } else {
    return _value->get_default_sampler();
  }
}

/**
 *
 */
INLINE void MaterialParamTexture::
clear_sampler_state() {
  _has_sampler = false;
}

/**
 *
 */
INLINE bool MaterialParamTexture::
has_sampler_state() const {
  return _has_sampler;
}

/**
 * Returns the number of animations encoded in the texture.
 */
INLINE int MaterialParamTexture::
get_num_animations() const {
  return (int)_anim_datas.size();
}

/**
 * Returns data for the nth animation encoded in the texture.
 */
INLINE const MaterialParamTexture::AnimData *MaterialParamTexture::
get_animation(int n) const {
  nassertr(n >= 0 && n < (int)_anim_datas.size(), nullptr);
  return &_anim_datas[n];
}
