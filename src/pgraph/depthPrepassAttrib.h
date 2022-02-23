/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file depthPrepassAttrib.h
 * @author brian
 * @date 2022-02-22
 */

#ifndef DEPTHPREPASSATTRIB_H
#define DEPTHPREPASSATTRIB_H

#include "pandabase.h"
#include "renderAttrib.h"
#include "bitMask.h"

class FactoryParams;
class BamReader;
class BamWriter;

/**
 *
 */
class EXPCL_PANDA_PGRAPH DepthPrepassAttrib : public RenderAttrib {
private:
  INLINE DepthPrepassAttrib(BitMask32 camera_mask);

PUBLISHED:
  static CPT(RenderAttrib) make(BitMask32 camera_mask = BitMask32::all_on());

  INLINE BitMask32 get_camera_mask() const;

PUBLISHED:
  MAKE_PROPERTY(camera_mask, get_camera_mask);

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual size_t get_hash_impl() const;
  virtual CPT(RenderAttrib) compose_impl(const RenderAttrib *other) const;

private:
  // The depth-prepass will only run on cameras that have bits in
  // common with this mask.
  BitMask32 _camera_mask;

PUBLISHED:
  static int get_class_slot() {
    return _attrib_slot;
  }
  virtual int get_slot() const {
    return get_class_slot();
  }
  MAKE_PROPERTY(class_slot, get_class_slot);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    RenderAttrib::init_type();
    register_type(_type_handle, "DepthPrepassAttrib",
                  RenderAttrib::get_class_type());
    _attrib_slot = register_slot(_type_handle, 100,
                                 new DepthPrepassAttrib(BitMask32::all_off()));
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
  static int _attrib_slot;
};

#include "depthPrepassAttrib.I"

#endif // DEPTHPREPASSATTRIB_H
