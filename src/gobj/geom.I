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
Geom(const Geom &copy) :
  _vertex_data(copy._vertex_data),
  _index_data(copy._index_data),
  _primitive_type(copy._primitive_type),
  _first_index(copy._first_index),
  _num_indices(copy._num_indices),
  _num_vertices_per_patch(copy._num_vertices_per_patch),
  _nested_vertices(copy._nested_vertices),
  _bounds_type(copy._bounds_type),
  _internal_bounds(copy._internal_bounds),
  _internal_bounds_stale(copy._internal_bounds_stale),
  _user_bounds(copy._user_bounds)
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
  _num_indices(std::move(other._num_indices)),
  _num_vertices_per_patch(std::move(other._num_vertices_per_patch)),
  _nested_vertices(std::move(other._nested_vertices)),
  _bounds_type(std::move(other._bounds_type)),
  _internal_bounds(std::move(other._internal_bounds)),
  _internal_bounds_stale(std::move(other._internal_bounds_stale)),
  _user_bounds(std::move(other._user_bounds))
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
  _num_vertices_per_patch = copy._num_vertices_per_patch;
  _nested_vertices = copy._nested_vertices;
  _bounds_type = copy._bounds_type;
  _internal_bounds = copy._internal_bounds;
  _internal_bounds_stale = copy._internal_bounds_stale;
  _user_bounds = copy._user_bounds;
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
  _num_vertices_per_patch = std::move(other._num_vertices_per_patch);
  _nested_vertices = std::move(other._nested_vertices);
  _bounds_type = std::move(other._bounds_type);
  _internal_bounds = std::move(other._internal_bounds);
  _internal_bounds_stale = std::move(other._internal_bounds_stale);
  _user_bounds = std::move(other._user_bounds);
}

/**
 * Simultaneously sets both the vertex buffer and index buffer of the Geom.
 *
 * This causes the index range of the Geom to be recomputed automatically.
 */
INLINE void Geom::
set_buffers(GeomVertexData *vertex_data, GeomIndexData *index_data) {
  _vertex_data = vertex_data;
  _index_data = index_data;
  compute_index_range();
  mark_internal_bounds_stale();
}

/**
 * Sets the table of vertices that should be used to draw this Geom.
 *
 * This causes the index range of the Geom to be recomputed automatically.
 */
INLINE void Geom::
set_vertex_data(const GeomVertexData *data) {
  _vertex_data = (GeomVertexData *)data;
  compute_index_range();
  mark_internal_bounds_stale();
}

/**
 * Returns the table of vertices that should be used to draw this Geom.
 */
INLINE const GeomVertexData *Geom::
get_vertex_data() const {
  return _vertex_data.get_read_pointer();
}

/**
 * Returns a writable pointer to the Geom's vertex data.
 */
INLINE PT(GeomVertexData) Geom::
modify_vertex_data() {
  mark_internal_bounds_stale();
  return _vertex_data.get_write_pointer();
}

/**
 * Sets the index buffer of the Geom.
 *
 * This causes the index range of the Geom to be recomputed automatically.
 */
INLINE void Geom::
set_index_data(const GeomIndexData *data) {
  _index_data = (GeomIndexData *)data;
  compute_index_range();
  mark_internal_bounds_stale();
}

/**
 * Returns the index buffer of the Geom.
 */
INLINE const GeomIndexData *Geom::
get_index_data() const {
  return _index_data.get_read_pointer();
}

/**
 * Returns a writable pointer to the Geom's index data.
 */
INLINE PT(GeomIndexData) Geom::
modify_index_data() {
  mark_internal_bounds_stale();
  return _index_data.get_write_pointer();
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
    return PT_polygons;

  case GPT_lines:
  case GPT_lines_adj:
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
 * Returns the number of vertices that make up a single primitive of the Geom.
 */
INLINE int Geom::
get_num_vertices_per_primitive() const {
  switch (_primitive_type) {
  case GPT_triangles_adj:
    return 6;
  case GPT_lines_adj:
    return 4;
  case GPT_triangles:
    return 3;
  case GPT_lines:
    return 2;
  case GPT_points:
    return 1;
  case GPT_patches:
    return _num_vertices_per_patch;
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
  mark_internal_bounds_stale();
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

/**
 * Returns true if the Geom is empty.  This is, if the Geom has no vertex
 * buffer.
 */
INLINE bool Geom::
is_empty() const {
  return _vertex_data.is_null();
}

/**
 * Returns true if the primitive is a composite primitive such as a tristrip
 * or trifan, or false if it is a fundamental primitive such as a collection
 * of triangles.
 *
 * Note: This always returns false as composite primitives are no longer
 * supported.
 */
INLINE bool Geom::
is_composite() const {
  return false;
}

/**
 * Returns the first vertex number referenced by the primitive.  This is
 * particularly important in the case of a nonindexed primitive, in which case
 * get_first_vertex() and get_num_vertices() completely define the extent of
 * the vertex range.
 */
INLINE int Geom::
get_first_vertex() const {
  return get_first_index();
}

/**
 * Returns the number of indices used by all the primitives in this object.
 */
INLINE int Geom::
get_num_vertices() const {
  return get_num_indices();
}

/**
 * Returns the minimum vertex index number used by this Geom.
 */
INLINE int Geom::
get_min_vertex() const {
  if (is_indexed()) {
    const GeomIndexData *index_data = get_index_data();
    index_data->check_minmax();
    return index_data->get_min_vertex();

  } else {
    return _first_index;
  }
}

/**
 * Returns the maximum vertex index number referenced by the Geom.
 */
INLINE int Geom::
get_max_vertex() const {
  if (is_indexed()) {
    const GeomIndexData *index_data = get_index_data();
    index_data->check_minmax();
    return index_data->get_max_vertex();

  } else {
    return _first_index + _num_indices - 1;
  }
}

/**
 * Returns the number of individual primitives stored within this object.  All
 * primitives are the same type.
 */
INLINE int Geom::
get_num_primitives() const {
  int num_vertices_per_primitive = get_num_vertices_per_primitive();

  // Should not be a composite primitive, not supported anymore.
  nassertr(num_vertices_per_primitive > 0, 0);

  // This is a simple primitive type like a triangle: each primitive uses
  // the same number of vertices.
  return (get_num_vertices() / num_vertices_per_primitive);
}

/**
 *
 */
INLINE int Geom::
get_num_faces() const {
  return get_num_primitives();
}

/**
 *
 */
INLINE int Geom::
get_primitive_num_faces(int n) const {
  return 1;
}

/**
 * Expands min_point and max_point to include all of the vertices in the Geom,
 * if any.  found_any is set true if any points are found.  It is the caller's
 * responsibility to initialize min_point, max_point, and found_any before
 * calling this function.
 *
 * This version of the method allows the Geom to specify an alternate vertex
 * data table (for instance, if the vertex data has already been munged), and
 * also allows the result to be computed in any coordinate space by specifying
 * a transform matrix.
 */
INLINE void Geom::
calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point,
                  bool &found_any,
                  const GeomVertexData *vertex_data,
                  bool got_mat, const LMatrix4 &mat) const {

  PN_stdfloat sq_radius;
  do_calc_tight_bounds(min_point, max_point, sq_radius, found_any,
                       vertex_data, got_mat, mat,
                       InternalName::get_vertex());
}

/**
 * Expands min_point and max_point to include all of the vertices in the Geom,
 * if any.  found_any is set true if any points are found.  It is the caller's
 * responsibility to initialize min_point, max_point, and found_any before
 * calling this function.
 *
 * This version of the method assumes the Geom will use its own vertex data,
 * and the results are computed in the Geom's own coordinate space.
 */
INLINE void Geom::
calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point,
                  bool &found_any) const {

  calc_tight_bounds(min_point, max_point, found_any,
                    get_vertex_data(), false,
                    LMatrix4::ident_mat());
}

/**
 * Similar to calc_tight_bounds(), for UV coordinates or other named columns.
 */
INLINE void Geom::
calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point,
                  bool &found_any,
                  const GeomVertexData *vertex_data,
                  bool got_mat, const LMatrix4 &mat,
                  const InternalName *column_name) const {

  PN_stdfloat sq_radius;
  do_calc_tight_bounds(min_point, max_point, sq_radius, found_any,
                       vertex_data, got_mat, mat,
                       column_name);
}

/**
 * Sets an explicit type of bounding volume that should be computed for the
 * Geom.
 */
INLINE void Geom::
set_bounds_type(BoundingVolume::BoundsType type) {
  _bounds_type = type;
  mark_internal_bounds_stale();
}

/**
 * Returns the bounding volume type set with set_bounds_type().
 */
INLINE BoundingVolume::BoundsType Geom::
get_bounds_type() const {
  return _bounds_type;
}

/**
 * Sets an explicit bounding volume for the Geom to be used during view
 * culling.
 */
INLINE void Geom::
set_bounds(const BoundingVolume *volume) {
  if (volume == nullptr) {
    _user_bounds = nullptr;
  } else {
    _user_bounds = volume->make_copy();
  }
}

/**
 * Clears the user specified bounding volume for the Geom and forces the
 * Geom's bounding volume to be recomputed the next time it is asked for.
 */
INLINE void Geom::
clear_bounds() {
  _user_bounds = nullptr;
  mark_internal_bounds_stale();
}

/**
 * Notes that something in the Geom has changes that warrants the bounding
 * volume being recomputed.  It will be recomputed the next time the bounds
 * are asked for.
 */
INLINE void Geom::
mark_bounds_stale() const {
  ((Geom *)this)->mark_internal_bounds_stale();
}

/**
 * Notes that something in the Geom has changes that warrants the bounding
 * volume being recomputed.  It will be recomputed the next time the bounds
 * are asked for.
 */
INLINE void Geom::
mark_internal_bounds_stale() {
  _internal_bounds_stale = true;
}
