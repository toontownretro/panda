/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file depthPrepassAttrib.cxx
 * @author brian
 * @date 2022-02-22
 */

#include "depthPrepassAttrib.h"

TypeHandle DepthPrepassAttrib::_type_handle;
int DepthPrepassAttrib::_attrib_slot;

/**
 *
 */
CPT(RenderAttrib) DepthPrepassAttrib::
make(BitMask32 camera_mask) {
  DepthPrepassAttrib *attr = new DepthPrepassAttrib(camera_mask);
  return return_new(attr);
}

/**
 *
 */
size_t DepthPrepassAttrib::
get_hash_impl() const {
  return integer_hash<uint32_t>::add_hash(0, _camera_mask.get_word());
}

/**
 *
 */
int DepthPrepassAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const DepthPrepassAttrib *that = (const DepthPrepassAttrib *)other;

  if (this->_camera_mask != that->_camera_mask) {
    return this->_camera_mask < that->_camera_mask ? -1 : 1;
  }

  return 0;
}

/**
 * Intended to be overridden by derived RenderAttrib types to specify how two
 * consecutive RenderAttrib objects of the same type interact.
 *
 * This should return the result of applying the other RenderAttrib to a node
 * in the scene graph below this RenderAttrib, which was already applied.  In
 * most cases, the result is the same as the other RenderAttrib (that is, a
 * subsequent RenderAttrib completely replaces the preceding one).  On the
 * other hand, some kinds of RenderAttrib (for instance, ColorTransformAttrib)
 * might combine in meaningful ways.
 */
CPT(RenderAttrib) DepthPrepassAttrib::
compose_impl(const RenderAttrib *other) const {
  const DepthPrepassAttrib *ta = (const DepthPrepassAttrib *)other;

  BitMask32 new_mask = _camera_mask | ta->_camera_mask;

  DepthPrepassAttrib *attrib = new DepthPrepassAttrib(new_mask);
  return return_new(attrib);
}
