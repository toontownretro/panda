/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialAttrib.I
 * @author brian
 * @date 2021-03-21
 */

/**
 *
 */
INLINE MaterialAttrib::
MaterialAttrib() :
  _material(nullptr),
  _modifier_state(nullptr),
  _is_off(false)
{
}

/**
 * Returns the material referenced by this material attribute.
 */
INLINE Material *MaterialAttrib::
get_material() const {
  return _material;
}

/**
 * Returns true if this is an "off" material attribute, false otherwise.
 */
INLINE bool MaterialAttrib::
is_off() const {
  return _is_off;
}

/**
 * Returns the RenderState object containing the attributes modified by the
 * material itself.
 */
INLINE const RenderState *MaterialAttrib::
get_modifier_state() const {
  if (_material == nullptr) {
    return nullptr;
  }

  if (_modifier_state == nullptr) {
    ((MaterialAttrib *)this)->create_modifier_state();
  }

  return _modifier_state;
}
