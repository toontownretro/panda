/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geom.I
 * @author brian
 * @date 2021-11-25
 */

/**
 *
 */
INLINE Geom::
Geom() :
  _primitive_type(GPT_invalid),
  _first_vertex(0),
  _num_vertices(0)
{
}

/**
 *
 */
INLINE Geom::
Geom(GeomPrimitiveType prim_type, GeomVertexData *vertex_data, GeomIndexData *index_data) :
  _vertex_data(vertex_data),
  _index_data(index_data),
  _primitive_type(prim_type)
{
  calc_vertex_range();
}

/**
 *
 */
INLINE Geom::
Geom(const Geom &copy) :
  _vertex_data(copy._vertex_data),
  _index_data(copy._index_data),
  _primitive_type(copy._primitive_type),
  _first_vertex(copy._first_vertex),
  _num_vertices(copy._num_vertices)
{
}

/**
 *
 */
INLINE Geom::
Geom(Geom &&other) :
  _vertex_data(std::move(other._vertex_data)),
  _index_data(std::move(other._index_data)),
  _primitive_type(std::move(other._primitive_type)),
  _first_vertex(std::move(other._first_vertex)),
  _num_vertices(std::move(other._num_vertices))
{
}

/**
 * Returns true if the Geom is valid and can be rendered, false otherwise.
 */
INLINE bool Geom::
is_valid() const {
  if (_primitive_type == GPT_invalid || _vertex_data == nullptr ||
      _first_vertex < 0 || _num_vertices <= 0) {
    return false;
  }

  if (is_indexed()) {
    // The vertex range must be within the size of the index buffer.
    return (_first_vertex < _index_data->get_num_indices() &&
            (_first_vertex + _num_vertices) < _index_data->get_num_indices());

  } else {
    // Non-indexed case: the vertex range must be within the size of the
    // vertex buffer.
    return (_first_vertex < _vertex_data->get_num_rows() &&
            (_first_vertex + _num_vertices) < _vertex_data->get_num_rows());
  }
}

/**
 * Returns true if the Geom is indexed.  That is, if the Geom contains
 * an index buffer.
 */
INLINE bool Geom::
is_indexed() const {
  return _index_data != nullptr;
}

/**
 * Sets the primitive topology of the Geom.
 */
INLINE void Geom::
set_primitive_type(GeomPrimitiveType type) {
  _primitive_type = type;
}

/**
 * Returns the primitive topology of the Geom.
 */
INLINE GeomEnums::GeomPrimitiveType Geom::
get_primitive_type() const {
  return _primitive_type;
}

/**
 * Returns the fundamental primitive family of the Geom, such as polygons or
 * lines.  This is used to determine the antialiasing mode to use when
 * rendering the Geom.
 */
INLINE GeomEnums::PrimitiveType Geom::
get_primitive_family() const {
  switch (_primitive_type) {
  case GPT_triangles:
  case GPT_triangles_adj:
  case GPT_triangle_strips:
  case GPT_triangle_strips_adj:
  case GPT_triangle_fans:
    return PT_polygons;

  case GPT_lines:
  case GPT_lines_adj:
  case GPT_line_strips:
  case GPT_line_strips_adj:
    return PT_lines;

  case GPT_points:
    return PT_points;

  case GPT_patches:
    return PT_patches;

  default:
    return PT_none;
  }
}

/**
 * Returns a monotonically increasing sequence.  Each time this is called, a
 * new sequence number is returned, higher than the previous value.
 *
 * This is used to ensure that GeomVertexArrayData::get_modified() and
 * GeomIndexData::get_modified() update from the same space, so that
 * Geom::get_modified() returns a meaningful value.
 */
UpdateSeq Geom::
get_next_modified() {
  ++_next_modified;
  return _next_modified;
}
