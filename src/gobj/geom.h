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

/**
 * A Geom is the smallest atomic piece of renderable geometry that can be sent
 * to the graphics card in one call.  It is simply a vertex buffer and index
 * buffer pairing.  Each Geom has an associated primitive type that is used
 * to interpret the index buffer when rendering the Geom.  Examples of
 * primitive types are triangles, tristrips, trifans, lines, etc.
 *
 * This is not a reference-counted object and is intended to be stored by
 * value.
 */
class EXPCL_PANDA_GOBJ Geom : public GeomEnums {
PUBLISHED:
  Geom(GeomPrimitiveType type, const GeomVertexData *data);

  void compute_index_range();

  INLINE Geom(const Geom &copy);
  INLINE Geom(Geom &&other);
  INLINE void operator = (const Geom &copy);
  INLINE void operator = (Geom &&other);

  INLINE void set_buffers(const GeomVertexData *vertex_data, const GeomIndexData *index_data);

  INLINE void set_vertex_data(const GeomVertexData *data);
  INLINE const GeomVertexData *get_vertex_data() const;

  INLINE void set_index_data(const GeomIndexData *data);
  INLINE const GeomIndexData *get_index_data() const;
  INLINE bool is_indexed() const;

  INLINE void set_primitive_type(GeomPrimitiveType type);
  INLINE GeomPrimitiveType get_primitive_type() const;

  INLINE PrimitiveType get_primitive_family() const;

  INLINE int get_num_vertices_per_primitive() const;

  INLINE void set_index_range(unsigned int first_index, unsigned int num_indices);
  INLINE unsigned int get_first_index() const;
  INLINE unsigned int get_num_indices() const;

public:
  static UpdateSeq get_next_modified();

private:
  CPT(GeomVertexData) _vertex_data;
  CPT(GeomIndexData) _index_data;
  GeomPrimitiveType _primitive_type;
  unsigned int _first_index;
  unsigned int _num_indices;

  // Specific to the patch primitive type.
  int _num_vertices_per_patch;

public:
  static UpdateSeq _next_modified;
};

#include "geom.I"

#endif // GEOM_H
