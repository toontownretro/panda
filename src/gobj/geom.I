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
 * @date 2021-09-05
 */

/**
 *
 */
INLINE Geom::
Geom(GeomPrimitiveType type, const GeomVertexData *data) :
  _vertex_data(data),
  _index_data(nullptr),
  _primitive_type(type),
  _first_index(0),
  _num_indices(0)
{
}

/**
 *
 */
INLINE Geom::
Geom(const Geom &copy) :
  _vertex_data(copy._vertex_data),
  _index_data(copy._index_data),
  _primitive_type(copy._primitive_type),
  _first_index(copy._first_index),
  _num_indices(copy._num_indices)
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
  _first_index(std::move(other._first_index)),
  _num_indices(std::move(other._num_indices))
{
}

/**
 *
 */
INLINE void Geom::
operator = (const Geom &copy) {
  _vertex_data = copy._vertex_data;
  _index_data = copy._index_data;
  _primitive_type = copy._primitive_type;
  _first_index = copy._first_index;
  _num_indices = copy._num_indices;
}

/**
 *
 */
INLINE void Geom::
operator = (Geom &&other) {
  _vertex_data = std::move(other._vertex_data);
  _index_data = std::move(other._index_data);
  _primitive_type = std::move(other._primitive_type);
  _first_index = std::move(other._first_index);
  _num_indices = std::move(other._num_indices);
}

/**
 * Sets the table of vertices that should be used to draw this Geom.
 */
INLINE void Geom::
set_vertex_data(const GeomVertexData *data) {
  _vertex_data = data;
}

/**
 * Returns the table of vertices that should be used to draw this Geom.
 */
INLINE const GeomVertexData *Geom::
get_vertex_data() const {
  return _vertex_data;
}

/**
 * Sets the index buffer of the Geom.
 */
INLINE void Geom::
set_index_data(const GeomIndexData *data) {
  _index_data = data;
}

/**
 * Returns the index buffer of the Geom.
 */
INLINE const GeomIndexData *Geom::
get_index_data() const {
  return _index_data;
}

/**
 * Returns true if this is an indexed Geom.  That is, if the Geom contains an
 * index buffer.
 */
INLINE bool Geom::
is_indexed() const {
  return _index_data != nullptr;
}

/**
 * Sets the geometric primitive type of the Geom.  This dictates how
 * the Geom is drawn and how the indices in the Geom's index buffer are
 * interpreted.
 */
INLINE void Geom::
set_primitive_type(GeomPrimitiveType type) {
  _primitive_type = type;
}

/**
 * Returns the geometric primitive type of the Geom.  This dictates how the
 * Geom is drawn and how the indices in the Geom's index buffer are
 * interpreted.
 */
INLINE Geom::GeomPrimitiveType Geom::
get_primitive_type() const {
  return _primitive_type;
}

/**
 * Returns the primitive family that the primitive type of the Geom belongs to.
 * For instance, triangles, tristrips, and trifans are all in the polygons
 * primitive family.  This is used to determine the correct antialiasing mode
 * to use when rendering the Geom.
 */
INLINE Geom::PrimitiveType Geom::
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
 * Sets the range of indices that the Geom should render from its index
 * buffer.
 */
INLINE void Geom::
set_index_range(unsigned int first_index, unsigned int num_indices) {
  _first_index = first_index;
  _num_indices = num_indices;
}

/**
 * Returns the index into the Geom's index buffer that the Geom should start
 * drawing from.  This method and get_num_indices() specifies the subset of
 * the Geom's index buffer that the Geom should draw.
 */
INLINE unsigned int Geom::
get_first_index() const {
  return _first_index;
}

/**
 * Returns the number of indices from the Geom's index buffer that the Geom
 * should draw.  This method and get_first_index() specifies the subset of
 * the Geom's index buffer that the Geom should draw.
 */
INLINE unsigned int Geom::
get_num_indices() const {
  return _num_indices;
}
