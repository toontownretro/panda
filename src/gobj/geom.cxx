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
#include "geomVertexWriter.h"
#include "geomVertexRewriter.h"
#include "bamWriter.h"
#include "bamReader.h"

UpdateSeq Geom::_next_modified;

/**
 *
 */
Geom::
Geom(GeomPrimitiveType type, const GeomVertexData *vertex_data, const GeomIndexData *index_data) :
  _vertex_data((GeomVertexData *)vertex_data),
  _index_data((GeomIndexData *)index_data),
  _primitive_type(type)
{
  compute_index_range();
}

/**
 * Constructor for an empty Geom.
 */
Geom::
Geom() :
  _primitive_type(GPT_triangles),
  _vertex_data(nullptr),
  _index_data(nullptr),
  _first_index(0),
  _num_indices(0),
  _num_vertices_per_patch(0)
{
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
      _num_indices = _vertex_data.get_read_pointer()->get_num_rows();
    }
  } else {
    // Use the number of rows in the index buffer.
    _num_indices = _index_data.get_read_pointer()->get_num_rows();
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

/**
 *
 */
int Geom::
get_primitive_start(int n) const {
  return n * get_num_vertices_per_primitive();
}

/**
 *
 */
int Geom::
get_primitive_end(int n) const {
  int verts_per_prim = get_num_vertices_per_primitive();
  return n * verts_per_prim + verts_per_prim;
}

/**
 *
 */
int Geom::
get_primitive_num_vertices(int n) const {
  return get_num_vertices_per_primitive();
}

/**
 *
 */
int Geom::
get_num_used_vertices() const {
  int num_primitives = get_num_primitives();
  if (num_primitives > 0) {
    return get_num_vertices();

  } else {
    return 0;
  }
}

/**
 * Returns a copy of this Geom with the index ordering reversed.
 *
 * This only means something to triangle Geoms.  Other primitive types
 * just return a copy of the same exact Geom.
 */
Geom Geom::
reverse() const {
  Geom copy(*this);
  copy.reverse_in_place();
  return copy;
}

/**
 * Reverses the winding order of the Geom's primitives, as an in-place operation.
 *
 * This implicitly converts the Geom to an indexed Geom if it is not one already.
 */
void Geom::
reverse_in_place() {
  if (_primitive_type == GPT_triangles ||
      _primitive_type == GPT_triangles_adj) {

    make_indexed();
    set_index_data(get_index_data()->reverse());
  }
}

/**
 * Returns a copy of this Geom with the reversed set of indices appended
 * to the index buffer, to double-side the Geom.
 *
 * This only means something to triangle Geoms.  Other primitive types
 * just return a copy of the same exact Geom.
 */
Geom Geom::
doubleside() const {
  Geom copy(*this);
  copy.doubleside_in_place();
  return copy;
}

/**
 *
 */
void Geom::
doubleside_in_place() {
  if (_primitive_type == GPT_triangles ||
      _primitive_type == GPT_triangles_adj) {

    make_indexed();
    set_index_data(get_index_data()->doubleside());
  }
}

/**
 * If the Geom does not have an index buffer, creates one using the Geom's
 * non-indexed vertex range, and assigns it to the Geom.
 */
void Geom::
make_indexed() {
  if (is_indexed()) {
    return;
  }

  // We must have some vertex range.
  nassertv(_num_indices != -1);

  // Determine the index type from the maximum vertex index.
  int max_index = _first_index + _num_indices - 1;

  NumericType index_type;
  if (max_index <= 0xff) {
    index_type = NT_uint8;
  } else if (max_index <= 0xffff) {
    index_type = NT_uint16;
  } else {
    index_type = NT_uint32;
  }

  _index_data = new GeomIndexData(GeomEnums::UH_static, index_type);
  PT(GeomIndexData) indices = modify_index_data();
  indices->add_consecutive_vertices(_first_index, _num_indices);
}

/**
 * Replaces a Geom's vertex table with a new table, and simultaneously adds
 * the indicated offset to all vertex references within the Geom's primitives.
 * This is intended to be used to combine multiple GeomVertexDatas from
 * different Geoms into a single big buffer, with each Geom referencing a
 * subset of the vertices in the buffer.
 */
void Geom::
offset_vertices(const GeomVertexData *data, int offset) {
  _vertex_data = (GeomVertexData *)data;

  if (is_indexed()) {
    // Offset all indices in the index buffer.
    PT(GeomIndexData) index_data = modify_index_data();
    index_data->offset_vertices(offset);

#ifndef NDEBUG
    // Sanity check.
    index_data->check_minmax();
    nassertv((index_data->get_min_vertex() < data->get_num_rows()) &&
             (index_data->get_max_vertex() < data->get_num_rows()));
#endif

  } else {
    // Just simply push the non-indexed vertex range.
    _first_index += offset;
    _num_indices += offset;

    // Sanity check.
    nassertv((_first_index + _num_indices - 1) < data->get_num_rows());
  }
}

/**
 * Turns on all bits corresponding to vertex indices that are referenced by
 * the Geom (or its index buffer).
 */
void Geom::
get_referenced_vertices(BitArray &bits) const {
  if (is_indexed()) {
    // Go through the whole index buffer.
    get_index_data()->get_referenced_vertices(bits);

  } else {
    // Non-indexed case.
    bits.set_range(_first_index, _num_indices);
  }
}

/**
 * Clears the Geom.  This removes the vertex buffer and index buffer
 * references.
 */
void Geom::
clear() {
  _vertex_data = nullptr;
  _index_data = nullptr;
  _first_index = 0;
  _num_indices = 0;
}

/**
 * Returns a GeomVertexData that represents the results of computing the
 * vertex animation on the CPU for this Geom's vertex data.
 *
 * If there is no CPU-defined vertex animation on this object, this just
 * returns the original object.
 *
 * If there is vertex animation, but the VertexTransform values have not
 * changed since last time, this may return the same pointer it returned
 * previously.  Even if the VertexTransform values have changed, it may still
 * return the same pointer, but with its contents modified (this is preferred,
 * since it allows the graphics backend to update vertex buffers optimally).
 *
 * If force is false, this method may return immediately with stale data, if
 * the vertex data is not completely resident.  If force is true, this method
 * will never return stale data, but may block until the data is available.
 */
CPT(GeomVertexData) Geom::
get_animated_vertex_data(bool force, Thread *current_thread) const {
  const GeomVertexData *vdata = get_vertex_data();

  if (vdata == nullptr) {
    return nullptr;
  }

  return vdata->animate_vertices(force, current_thread);
}

/**
 *
 */
void Geom::
write_datagram(BamWriter *manager, Datagram &me) {
  manager->write_pointer(me, (GeomVertexData *)_vertex_data.get_read_pointer());
  manager->write_pointer(me, (GeomIndexData *)_index_data.get_read_pointer());
  me.add_uint8(_primitive_type);
  me.add_uint32(_first_index);
  me.add_uint32(_num_indices);
  me.add_int32(_num_vertices_per_patch);
}

/**
 *
 */
void Geom::
fillin(DatagramIterator &scan, BamReader *manager) {
  manager->read_pointer(scan); // vertex data
  manager->read_pointer(scan); // index data
  _primitive_type = (GeomPrimitiveType)scan.get_uint8();
  _first_index = scan.get_uint32();
  _num_indices = scan.get_uint32();
  _num_vertices_per_patch = scan.get_int32();
}

/**
 *
 */
int Geom::
complete_pointers(TypedWritable **p_list, BamReader *manager, int pi) {
  _vertex_data = DCAST(GeomVertexData, p_list[pi++]);
  _index_data = DCAST(GeomIndexData, p_list[pi++]);
  return pi;
}
