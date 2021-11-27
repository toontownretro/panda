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
 * @date 2021-11-25
 */

#include "geom.h"
#include "bamWriter.h"
#include "bamReader.h"
#include "boundingBox.h"
#include "boundingSphere.h"

/**
 * Creates and returns a BoundingVolume that encloses all of the vertex
 * positions referenced by this Geom.
 */
PT(BoundingVolume) Geom::
make_bounds(BoundingVolume::BoundsType bounds_type) const {

}

/**
 * Recomputes the vertex range of the Geom--the range of vertices in the
 * vertex or index buffer that the Geom should render.
 */
void Geom::
calc_vertex_range() {
  _first_vertex = 0;
  if (is_indexed()) {
    _num_vertices = _index_data->get_num_indices();
  } else {
    _num_vertices = _vertex_data->get_num_rows();
  }
}

/**
 *
 */
void Geom::
write_datagram(BamWriter *manager, Datagram &me) {
  manager->write_pointer(me, _vertex_data);
  manager->write_pointer(me, _index_data);
  me.add_int8(_primitive_type);
  me.add_int32(_first_vertex);
  me.add_int32(_num_vertices);
}

/**
 *
 */
void Geom::
fillin(DatagramIterator &scan, BamReader *manager) {
  manager->read_pointer(scan); // _vertex_data
  manager->read_pointer(scan); // _index_data
  _primitive_type = (GeomPrimitiveType)scan.get_int8();
  _first_vertex = scan.get_int32();
  _num_vertices = scan.get_int32();
}

/**
 *
 */
int Geom::
complete_pointers(int pi, TypedWritable **p_list, BamReader *manager) {
  _vertex_data = DCAST(GeomVertexData, p_list[pi++]);
  _index_data = DCAST(GeomIndexData, p_list[pi++]);
  return pi;
}
