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
#include "pointerTo.h"
#include "bitArray.h"

/**
 * This is a subclass of GeomVertexArrayData whose only purpose is to indicate
 * that the object represents an index buffer instead of a vertex buffer.
 *
 * It also stores a few things about the index buffer format locally, such as
 * the index numeric type and stride, so we don't have to ask our pointer to
 * the GeomVertexArrayFormat for the info when we render.
 *
 * It also provides a user-friendly interface to write and read vertex indices
 * to and from the index buffer.
 */
class EXPCL_PANDA_GOBJ GeomIndexData : public GeomVertexArrayData {
protected:
  virtual PT(CopyOnWriteObject) make_cow_copy() override;

PUBLISHED:
  INLINE GeomIndexData(UsageHint usage, NumericType index_type = NT_uint16);
  INLINE GeomIndexData(const GeomIndexData &copy);
  INLINE void operator = (const GeomIndexData &copy);
  virtual ~GeomIndexData() override;

  void set_index_type(NumericType index_type);
  INLINE GeomEnums::NumericType get_index_type() const;
  INLINE int get_index_stride() const;

  void add_vertex(int vertex);
  INLINE void add_vertices(int v1, int v2);
  INLINE void add_vertices(int v1, int v2, int v3);
  INLINE void add_vertices(int v1, int v2, int v3, int v4);
  void add_consecutive_vertices(int start, int num_vertices);
  void add_next_vertices(int num_vertices);
  void reserve_num_vertices(int num_vertices);

  void offset_vertices(int offset);

  INLINE int get_num_vertices() const;
  int get_vertex(int n) const;
  INLINE int get_min_vertex() const;
  INLINE int get_max_vertex() const;

  PT(GeomIndexData) reverse() const;
  void reverse_in_place();

  PT(GeomIndexData) doubleside() const;
  void doubleside_in_place();

  INLINE bool close_primitive() { return true; }

  void check_minmax() const;
  void recompute_minmax();

  void get_referenced_vertices(BitArray &bits) const;

  INLINE static CPT(GeomVertexArrayFormat) make_index_format(NumericType index_type);

private:
  void consider_elevate_index_type(int vertex);

private:
  GeomEnums::NumericType _index_type;

  // The minimum and maximum vertex index referenced by the index buffer.
  // Needed for glDrawRangeElements().
  bool _got_minmax;
  unsigned int _min_vertex;
  unsigned int _max_vertex;

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
