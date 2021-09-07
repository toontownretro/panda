/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomIndexData.h
 * @author brian
 * @date 2021-09-06
 */

#ifndef GEOMINDEXDATA_H
#define GEOMINDEXDATA_H

#include "pandabase.h"
#include "geomVertexArrayData.h"

/**
 * This is a subclass of GeomVertexArrayData whose only purpose is to indicate
 * that the object represents an index buffer instead of a vertex buffer.
 *
 * It also stores a few things about the index buffer format locally, such as
 * the index numeric type and stride, so we don't have to ask our pointer to
 * the GeomVertexArrayFormat for the info when we render.
 */
class EXPCL_PANDA_GOBJ GeomIndexData : public GeomVertexArrayData {
PUBLISHED:
  INLINE GeomEnums::NumericType get_index_type() const;
  INLINE int get_index_stride() const;

private:
  GeomEnums::NumericType _index_type;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GeomVertexArrayData::init_type();
    register_type(_type_handle, "GeomIndexData",
                  GeomVertexArrayData::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};
#include "geomIndexData.I"

#endif // GEOMINDEXDATA_H
