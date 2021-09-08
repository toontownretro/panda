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
 * @date 2021-09-05
 */

#ifndef GEOM_H
#define GEOM_H

#include "pandabase.h"
#include "geomIndexData.h"
#include "geomVertexData.h"
#include "pointerTo.h"
#include "geomEnums.h"
#include "updateSeq.h"
#include "bitArray.h"

/**
 * A Geom is the smallest atomic piece of renderable geometry that can be sent
 * to the graphics card in one call.  It is simply a vertex buffer and index
 * buffer pairing.  Each Geom has an associated primitive type that is used
 * to interpret the index buffer when rendering the Geom.  Examples of
 * primitive types are triangles, lines, or points.
 *
 * This is not a reference-counted object and is intended to be stored by
 * value.
 */
class EXPCL_PANDA_GOBJ Geom : public GeomEnums {
PUBLISHED:
  Geom(GeomPrimitiveType type, const GeomVertexData *vertex_data,
       const GeomIndexData *index_data);
  Geom();

  void compute_index_range();

  INLINE Geom(const Geom &copy);
  INLINE Geom(Geom &&other);
  INLINE void operator = (const Geom &copy);
  INLINE void operator = (Geom &&other);

  INLINE void set_buffers(GeomVertexData *vertex_data, GeomIndexData *index_data);

  INLINE void set_vertex_data(const GeomVertexData *data);
  INLINE const GeomVertexData *get_vertex_data() const;
  INLINE PT(GeomVertexData) modify_vertex_data();

  INLINE void set_index_data(const GeomIndexData *data);
  INLINE const GeomIndexData *get_index_data() const;
  INLINE bool is_indexed() const;
  INLINE PT(GeomIndexData) modify_index_data();

  INLINE void set_primitive_type(GeomPrimitiveType type);
  INLINE GeomPrimitiveType get_primitive_type() const;

  INLINE PrimitiveType get_primitive_family() const;

  INLINE int get_num_vertices_per_primitive() const;

  INLINE void set_index_range(unsigned int first_index, unsigned int num_indices);
  INLINE unsigned int get_first_index() const;
  INLINE unsigned int get_num_indices() const;

  INLINE bool is_empty() const;

  INLINE bool is_composite() const;

  INLINE int get_first_vertex() const;
  INLINE int get_num_vertices() const;

  INLINE int get_min_vertex() const;
  INLINE int get_max_vertex() const;

  INLINE int get_num_primitives() const;
  int get_primitive_start(int n) const;
  int get_primitive_end(int n) const;
  int get_primitive_num_vertices(int n) const;
  int get_num_used_vertices() const;

  INLINE int get_num_faces() const;
  INLINE int get_primitive_num_faces(int n) const;

  void make_indexed();

  Geom reverse() const;
  void reverse_in_place();

  Geom doubleside() const;
  void doubleside_in_place();

  void offset_vertices(const GeomVertexData *data, int offset);

  void get_referenced_vertices(BitArray &bits) const;

public:
  static UpdateSeq get_next_modified();

private:
  // Pointer to the vertex buffer the Geom should render with.
  COWPT(GeomVertexData) _vertex_data;

  // Pointer to the index buffer the Geom should render with.
  // If this is NULL, the Geom is non-indexed, and the _first_index
  // and _num_indices variables define the range of consecutive
  // vertices from the vertex buffer to render.
  COWPT(GeomIndexData) _index_data;

  // The geometric primitive type of the Geom.  Can be triangles,
  // lines, points, or patches.  No strips or triangle fans.
  GeomPrimitiveType _primitive_type;

  /////////////////////////////////////////////////////////////////////////////
  // For an indexed Geom, this is a range of consective indices
  // into the *index* buffer that should be drawn for the Geom.
  //
  // For a non-indexed Geom, this is the range of consecutive indices
  // into the *vertex* buffer.
  //
  unsigned int _first_index;
  unsigned int _num_indices;
  /////////////////////////////////////////////////////////////////////////////

  // Specific to the patch primitive type.
  int _num_vertices_per_patch;

public:
  static UpdateSeq _next_modified;
};

#include "geom.I"

#endif // GEOM_H
