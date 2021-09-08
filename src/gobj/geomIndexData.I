/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomIndexData.I
 * @author brian
 * @date 2021-09-06
 */

/**
 *
 */
INLINE GeomIndexData::
GeomIndexData(UsageHint usage, NumericType index_type) :
  GeomVertexArrayData(make_index_format(index_type), usage),
  _index_type(index_type),
  _min_vertex(0),
  _max_vertex(0),
  _got_minmax(false)
{
}

/**
 *
 */
INLINE GeomIndexData::
GeomIndexData(const GeomIndexData &copy) :
  GeomVertexArrayData(copy),
  _index_type(copy._index_type),
  _min_vertex(copy._min_vertex),
  _max_vertex(copy._max_vertex),
  _got_minmax(copy._got_minmax)
{
}

/**
 *
 */
INLINE void GeomIndexData::
operator = (const GeomIndexData &copy) {
  GeomVertexArrayData::operator = (copy);
  _index_type = copy._index_type;
  _min_vertex = copy._min_vertex;
  _max_vertex = copy._max_vertex;
  _got_minmax = copy._got_minmax;
}

/**
 * Returns the numeric type of the index buffer.
 */
INLINE GeomEnums::NumericType GeomIndexData::
get_index_type() const {
  return _index_type;
}

/**
 * Returns the byte length of a single entry within the index buffer.
 * This is determined by the index type.
 */
INLINE int GeomIndexData::
get_index_stride() const {
  switch(_index_type) {
  case GeomEnums::NT_uint8:
    return 1;
  case GeomEnums::NT_uint16:
    return 2;
  case GeomEnums::NT_uint32:
    return 4;
  default:
    // None of the index types other than the ones handled above are supported.
    assert(false);
    return 0;
  }
}

/**
 * Appends several vertex indices to the index buffer.
 */
INLINE void GeomIndexData::
add_vertices(int v1, int v2) {
  add_vertex(v1);
  add_vertex(v2);
}

/**
 * Appends several vertex indices to the index buffer.
 */
INLINE void GeomIndexData::
add_vertices(int v1, int v2, int v3) {
  add_vertex(v1);
  add_vertex(v2);
  add_vertex(v3);
}

/**
 * Appends several vertex indices to the index buffer.
 */
INLINE void GeomIndexData::
add_vertices(int v1, int v2, int v3, int v4) {
  add_vertex(v1);
  add_vertex(v2);
  add_vertex(v3);
  add_vertex(v4);
}

/**
 * Returns the total number of vertex indices in the index buffer.
 */
INLINE int GeomIndexData::
get_num_vertices() const {
  return get_num_rows();
}

/**
 * Returns the minimum vertex index referenced by the index buffer.
 */
INLINE int GeomIndexData::
get_min_vertex() const {
  nassertr(_got_minmax, 0);
  return _min_vertex;
}

/**
 * Returns the maximum vertex index referenced by the index buffer.
 */
INLINE int GeomIndexData::
get_max_vertex() const {
  nassertr(_got_minmax, 0);
  return _max_vertex;
}

/**
 * Returns a registered format appropriate for using to store the index table.
 */
INLINE CPT(GeomVertexArrayFormat) GeomIndexData::
make_index_format(NumericType index_type) {
  PT(GeomVertexArrayFormat) format = new GeomVertexArrayFormat;
  // It's important that the index format *not* respect the global setting of
  // vertex-column-alignment.  It needs to be tightly packed, so we specify an
  // explict column_alignment of 1.
  format->add_column(InternalName::get_index(), 1, index_type, C_index, 0, 1);
  return GeomVertexArrayFormat::register_format(format);
}
