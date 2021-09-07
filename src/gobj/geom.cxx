/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geom.cxx
 * @author brian
 * @date 2021-09-05
 */

#include "geom.h"

UpdateSeq Geom::_next_modified;

/**
 *
 */
Geom::
Geom(GeomPrimitiveType type, const GeomVertexData *data) :
  _vertex_data(data),
  _index_data(nullptr),
  _primitive_type(type)
{
  compute_index_range();
}

/**
 * Recomputes the index range of the Geom.  If the Geom does not have an index
 * buffer, the range is the number of rows in the vertex buffer.  Otherwise,
 * the range is the number of rows in the index buffer.  This can be later
 * overridden by the user to specify a subset of vertices or indices that the
 * Geom should render.
 */
void Geom::
compute_index_range() {
  _first_index = 0;
  _num_indices = 0;

  if (_index_data == nullptr) {
    // No index buffer, use number of rows in vertex buffer.
    if (_vertex_data != nullptr) {
      _num_indices = _vertex_data->get_num_rows();
    }
  } else {
    // Use the number of rows in the index buffer.
    _num_indices = _index_data->get_num_rows();
  }
}

/**
 *
 */
UpdateSeq Geom::
get_next_modified() {
  ++_next_modified;
  return _next_modified;
}
