/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geom.h
 * @author brian
 * @date 2021-11-25
 */

#ifndef GEOM_H
#define GEOM_H

#include "pandabase.h"
#include "geomEnums.h"
#include "geomVertexData.h"
#include "geomIndexData.h"
#include "pointerTo.h"
#include "updateSeq.h"
#include "boundingVolume.h"

/**
 * This is the smallest piece of renderable geometry, corresponding to a
 * single draw call in the graphics pipe.  It is synonymous to a "mesh".
 *
 * A Geom is simply a vertex buffer and index buffer pairing, where the
 * vertex buffer defines all of the vertex positions, normals, etc,
 * and the index buffer forms the individual primitives, determined by
 * the Geom's primitive type, by indexing into the vertex buffer.
 *
 * A Geom can optionally specify a subset of the index buffer to render
 * from, rather than rendering the entire thing.  This is particularly
 * useful when sharing index buffers between Geoms.
 */
class EXPCL_PANDA_GOBJ Geom : public GeomEnums {
PUBLISHED:
  INLINE Geom(GeomPrimitiveType prim_type, GeomVertexData *data, GeomIndexData *indices = nullptr);
  INLINE Geom(const Geom &copy);
  INLINE Geom(Geom &&other);

  INLINE bool is_valid() const;
  INLINE bool is_indexed() const;

  INLINE void set_primitive_type(GeomPrimitiveType type);
  INLINE GeomPrimitiveType get_primitive_type() const;
  INLINE PrimitiveType get_primitive_family() const;

  INLINE void set_index_data(GeomIndexData *data);
  INLINE const GeomIndexData *get_index_data() const;
  INLINE PT(GeomIndexData) modify_index_data();

  INLINE void set_vertex_data(GeomVertexData *data);
  INLINE const GeomVertexData *get_vertex_data() const;
  INLINE PT(GeomVertexData) modify_vertex_data();

  INLINE void set_vertex_range(int first_vertex, int num_vertices = -1);
  INLINE int get_first_vertex() const;
  INLINE int get_num_vertices() const;

  PT(BoundingVolume) make_bounds(BoundingVolume::BoundsType type = BoundingVolume::BT_default) const;

  void calc_vertex_range();

  void write_datagram(BamWriter *manager, Datagram &me);
  void fillin(DatagramIterator &scan, BamReader *manager);

  INLINE static UpdateSeq get_next_modified();

public:
  INLINE Geom();
  int complete_pointers(int pi, TypedWritable **p_list, BamReader *manager);

private:
  // The vertex buffer the Geom should render from.
  CPT(GeomVertexData) _vertex_data;

  // The index buffer the Geom should render from.
  CPT(GeomIndexData) _index_data;

  // Triangles, tristrips, lines, etc.
  GeomPrimitiveType _primitive_type;

  // Vertex range for rendering only a subset of the vertex buffer or index
  // buffer.  In the non-indexed case, this is a subset of the vertex buffer,
  // while in the indexed case, it is a subset of the index buffer.
  int _first_vertex;
  int _num_vertices;

  static UpdateSeq _next_modified;
};

#include "geom.I"

#endif // GEOM_H
