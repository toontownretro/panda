/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomIndexData.cxx
 * @author brian
 * @date 2021-09-06
 */

#include "geomIndexData.h"
#include "geomVertexWriter.h"
#include "geomVertexReader.h"
#include "thread.h"

TypeHandle GeomIndexData::_type_handle;

/**
 *
 */
PT(CopyOnWriteObject) GeomIndexData::
make_cow_copy() {
  return new GeomIndexData(*this);
}

/**
 *
 */
GeomIndexData::
~GeomIndexData() {
}

/**
 * Changes the index type of the index buffer.  If the index type is different,
 * the index data is reassigned to a copy of the index data with the new
 * index type assigned.
 */
void GeomIndexData::
set_index_type(NumericType type) {
  nassertv(type == NT_uint8 || type == NT_uint16 || type == NT_uint32);

  if (type == _index_type) {
    // Same index format.
    return;
  }

  GeomIndexData new_data(get_usage_hint(), type);
  new_data.local_object();
  new_data.set_num_rows(get_num_rows());

  GeomVertexReader from(this, 0);
  GeomVertexWriter to(&new_data, 0);

  // Copy the indices into the new buffer.
  while (!from.is_at_end()) {
    int index = from.get_data1i();
    to.set_data1i(index);
  }

  // Assign ourselves to the index data with the new format.
  GeomIndexData::operator = (new_data);
}

/**
 * Appends a vertex index to the index buffer.
 */
void GeomIndexData::
add_vertex(int vertex) {
  consider_elevate_index_type(vertex);

  GeomVertexArrayDataHandle handle(this, Thread::get_current_thread());
  int num_rows = handle.get_num_rows();
  handle.set_num_rows(num_rows + 1);

  unsigned char *ptr = handle.get_write_pointer();
  switch (_index_type) {
  case GeomEnums::NT_uint8:
    ((uint8_t *)ptr)[num_rows] = vertex;
    break;
  case GeomEnums::NT_uint16:
    ((uint16_t *)ptr)[num_rows] = vertex;
    break;
  case GeomEnums::NT_uint32:
    ((uint32_t *)ptr)[num_rows] = vertex;
    break;
  default:
    nassert_raise("unsupported index type");
    break;
  }

  _got_minmax = false;
}

/**
 * Adds a consecutive sequence of vertex indices, beginning at start, to the
 * index buffer.
 */
void GeomIndexData::
add_consecutive_vertices(int start, int num_vertices) {
  if (num_vertices == 0) {
    return;
  }
  int end = (start + num_vertices) - 1;

  consider_elevate_index_type(end);

  int old_num_rows = get_num_rows();
  set_num_rows(old_num_rows + num_vertices);

  GeomVertexWriter index(this, 0);
  index.set_row_unsafe(old_num_rows);

  for (int v = start; v <= end; ++v) {
    index.set_data1i(v);
  }

  _got_minmax = false;
}

/**
 * Adds the next n vertices in sequence, beginning from the last vertex added
 * to the primitive + 1.
 *
 * This is most useful when you are building up a GeomIndexData and a
 * GeomVertexData at the same time, and you just want the index data to
 * reference the first n vertices from the vertex data, then the next n, and so
 * on.
 */
void GeomIndexData::
add_next_vertices(int num_vertices) {
  if (get_num_vertices() == 0) {
    add_consecutive_vertices(0, num_vertices);
  } else {
    add_consecutive_vertices(get_vertex(get_num_vertices() - 1) + 1, num_vertices);
  }
}

/**
 * Ensures enough memory space in the index buffer for the indicated number
 * of entries.
 */
void GeomIndexData::
reserve_num_vertices(int count) {
  consider_elevate_index_type(count);
  reserve_num_rows(count);
}

/**
 * Returns the ith vertex index in the index buffer.
 */
int GeomIndexData::
get_vertex(int i) const {
  nassertr(i >= 0 && i < get_num_vertices(), -1);

  GeomVertexArrayDataHandle handle(this, Thread::get_current_thread());
  const unsigned char *ptr = handle.get_read_pointer(true);
  switch (_index_type) {
  case NT_uint8:
    return ((uint8_t *)ptr)[i];
  case NT_uint16:
    return ((uint16_t *)ptr)[i];
  case NT_uint32:
    return ((uint32_t *)ptr)[1];
  default:
    nassert_raise("unsupported index type");
    return -1;
  }
}

/**
 * Ensures that the index buffer's minmax cache has been computed.
 */
void GeomIndexData::
check_minmax() const {
  if (!_got_minmax) {
    ((GeomIndexData *)this)->recompute_minmax();
    nassertv(_got_minmax);
  }
}

/**
 * Computes the minimum and maximum vertex indices referenced by the index
 * buffer.  This is needed for glDrawRangeElements().
 */
void GeomIndexData::
recompute_minmax() {
  int num_vertices = get_num_rows();
  if (num_vertices == 0) {
    // If we don't have any vertices, the minmax is trivial.
    _min_vertex = 0;
    _max_vertex = 0;

  } else {
    GeomVertexReader index(this, 0);

    // Read over each index and find the minimum and maximum
    // vertex indices.

    unsigned int vertex = index.get_data1i();
    _min_vertex = vertex;
    _max_vertex = vertex;
    for (int vi = 1; vi < num_vertices; ++vi) {
      nassertv(!index.is_at_end());
      unsigned int vertex = index.get_data1i();
      _min_vertex = std::min(_min_vertex, vertex);
      _max_vertex = std::max(_max_vertex, vertex);
    }
  }

  _got_minmax = true;
}

/**
 * Increases the index type of the index buffer if the indicated vertex is
 * greater than the maximum value allowed by the current index type.
 */
void GeomIndexData::
consider_elevate_index_type(int vertex) {
  switch (_index_type) {
  case NT_uint8:
    if (vertex >= 0xff) {
      set_index_type(NT_uint16);
    }
    break;
  case NT_uint16:
    if (vertex >= 0xffff) {
      set_index_type(NT_uint32);
    }
    break;
  case NT_uint32:
    // Not much we can do here.
    nassertv(vertex < 0x7fffffff);
    break;

  default:
    break;
  }
}
